#include "../include/share_memory.h"
#include "./data_def.h"
#include "../include/rb_tree.h"
#ifdef WIN32
#include <windows.h>
#include <stdio.h>

__declspec(thread) HMEMORYUNIT def_shm_info_unit = 0;
__declspec(thread) HMEMORYUNIT def_shm_mgr_unit = 0;

typedef struct st_share_memory_info
{
    int		key;
    HANDLE	handle;
    void*	mem;
    size_t	size;
}share_memory_info;

typedef struct st_share_memory_info_mgr
{
    struct st_rb_tree*  share_memory_info_map;
    struct st_mem_unit* info_unit;
}share_memory_info_mgr;

inline HMEMORYUNIT _default_shm_info_unit(void)
{
    return def_shm_info_unit;
}

inline HMEMORYUNIT _default_shm_info_mgr_unit(void)
{
    return def_shm_mgr_unit;
}

size_t sizeof_share_memory_info(void)
{
    return sizeof(share_memory_info);
}

size_t sizeof_share_mamory_mgr(void)
{
    return sizeof(share_memory_info_mgr);
}

HSHMMGR create_shm_mgr(void)
{
    HSHMMGR shm_info_mgr = (HSHMMGR)memory_unit_alloc(_default_shm_info_mgr_unit(), 256);
    shm_info_mgr->info_unit = _default_shm_info_unit();
    shm_info_mgr->share_memory_info_map = create_rb_tree(0);

	return shm_info_mgr;
}

void destroy_shm_mgr(HSHMMGR mgr)
{
	HRBNODE shm_node = rb_first(mgr->share_memory_info_map);

	while (shm_node)
	{
		HSHMINFO shm_info = (HSHMINFO)rb_node_value(shm_node);
		UnmapViewOfFile(shm_info->mem);
		CloseHandle(shm_info->handle);
		memory_unit_free(mgr->info_unit, shm_info);

		shm_node = rb_next(shm_node);
	}

	destroy_rb_tree(mgr->share_memory_info_map);

	memory_unit_free(_default_shm_info_mgr_unit(), mgr);
}

HSHMINFO shm_get_info(HSHMMGR mgr, int key)
{
	HRBNODE shm_node = rb_tree_find_int(mgr->share_memory_info_map, key);
	if (shm_node)
	{
		return (HSHMINFO)rb_node_value(shm_node);
	}

	return 0;
}

void* shm_alloc(HSHMMGR mgr, int key, size_t size)
{
    void* pTr = NULL;
    HANDLE hHandle = NULL;
    char key_name[64];
    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };
	HSHMINFO shm_info = shm_get_info(mgr, key);
	if (shm_info)
	{
		return 0;
	}

	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;


	sprintf(key_name, "Global\\LIBSVR_%d", key);

	hHandle = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, size, key_name);

	if (NULL == hHandle || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (hHandle)
		{
			CloseHandle(hHandle);
		}
		return 0;
	}

	pTr = MapViewOfFile(hHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);

	if (NULL == pTr)
	{
		CloseHandle(hHandle);
		return 0;
	}

	shm_info = memory_unit_alloc(mgr->info_unit, 4096);
	shm_info->handle	= hHandle;
	shm_info->key		= key;
	shm_info->mem		= pTr;
	shm_info->size		= size;

	rb_tree_insert_int(mgr->share_memory_info_map, key, shm_info);

	return pTr;
}

void shm_free(HSHMMGR mgr, int key)
{
    HSHMINFO shm_info;
	HRBNODE shm_node = rb_tree_find_int(mgr->share_memory_info_map, key);

	if (!shm_node)
	{
		return;
	}

	shm_info = (HSHMINFO)rb_node_value(shm_node);

	UnmapViewOfFile(shm_info->mem);
	CloseHandle(shm_info->handle);

	rb_tree_erase(mgr->share_memory_info_map, shm_node);

	memory_unit_free(mgr->info_unit, shm_info);
}

#else

#endif