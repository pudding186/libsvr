#include "../include/lua_script.hpp"
#include "../lib3rd/lua/include/lstate.h"
#include "../include/memory_pool.h"

#ifdef _WIN32
#include "../lib3rd/openssl/win32/include/openssl/des.h"
#else
#include "../lib3rd/openssl/x64/include/openssl/des.h"
#endif

#include <ctype.h>


#define CRUSH_CODE char* p = 0;*p = 'a';

#define ENCRYPT_FLAG "--###CRYPT_SECTION###"
#define LUA_C_FUNC_FLAG "lualib"
#define LUA_C_VAR_FLAG  "lua_"
#define LUA_REQUIRE_FLAG "require"
#define MAX_CRYPT_SIZE (1024)

typedef struct lua_segment
{
    const char*     seg_begin;
    const char*     seg_end;
    char*           base64_cache;
    size_t          base64_cache_size;
    size_t          base64_size;
    char*           ncbc_cache;
    size_t          ncbc_cache_size;
    size_t          ncbc_size;

    lua_segment*    next;
    bool            is_use;
    bool            is_normal;
}lua_segment;

typedef struct lua_c_func_name
{
    char*               c_func_name;
    lua_c_func_name*    next;
}lua_c_interface;

typedef struct st_lua_file
{
    char*               file_content;
    size_t              file_content_size;
    char*               buffer;
    size_t              buffer_size;
    lua_segment*        seg_head;
    lua_segment*        seg_tail;
    lua_c_func_name*    func_head;
}lua_file;

lua_segment* create_lua_segment(void)
{
    lua_segment* seg = new lua_segment;
    seg->seg_begin = 0;
    seg->seg_end = 0;
    seg->base64_cache = new char[8*1024];
    seg->base64_cache_size = 8*1024;
    seg->base64_size = 0;
    seg->ncbc_cache = new char[8*1024];
    seg->ncbc_cache_size = 8*1024;
    seg->ncbc_size = 0;
    seg->is_use = false;

    return seg;
}

extern void trace_lua_stack(lua_State* L);

void destroy_lua_segment(lua_segment* seg)
{
    if (seg->base64_cache)
    {
        delete[] seg->base64_cache;
    }

    if (seg->ncbc_cache)
    {
        delete[] seg->ncbc_cache;
    }

    delete seg;
}

lua_file* create_lua_life(void)
{
    lua_file* LF = new lua_file;
    LF->file_content = new char[16*1024];
    LF->file_content_size = 16*1024;
    LF->buffer = new char[16*1024];
    LF->buffer_size = 16*1024;
    LF->seg_head = 0;
    LF->seg_tail = 0;

    for (int i = 0; i < 1; i++)
    {
        lua_segment* seg = create_lua_segment();

        if (LF->seg_head)
        {
            LF->seg_tail->next = seg;
            seg->next = 0;
            LF->seg_tail = seg;
        }
        else
        {
            LF->seg_head = seg;
            LF->seg_tail = seg;
            seg->next = 0;
        }
    }

    return LF;
}

void reset_lua_life(lua_file* LF)
{
    lua_segment* seg = LF->seg_head;

    while (seg)
    {
        seg->is_use = false;
        seg->seg_begin = 0;
        seg->seg_end = 0;
        seg->base64_size = 0;
        seg->ncbc_size = 0;
        seg = seg->next;
    }
}

void destroy_lua_life(lua_file* LF)
{
    if (LF->file_content)
    {
        delete[] LF->file_content;
    }

    if (LF->buffer)
    {
        delete[] LF->buffer;
    }

    lua_segment* seg = LF->seg_head;

    while (seg)
    {
        lua_segment* tmp = seg->next;
        destroy_lua_segment(seg);
        seg = tmp;
    }
}

static void lua_base64_decode(lua_segment* seg)
{
    size_t iInSize = seg->seg_end - seg->seg_begin + 1;
    const char* pInput = seg->seg_begin;

    if (seg->base64_cache_size < iInSize)
    {
        if (seg->base64_cache)
        {
            delete[] seg->base64_cache;
        }

        seg->base64_cache_size = iInSize + 1;
        seg->base64_cache = new char[seg->base64_cache_size];
    }

    char* pOutput = seg->base64_cache;

    const unsigned char b64decode_table[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,62, 0, 0, 0,63,
        52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0,
        0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51
    };

    int nValue;
    size_t i = 0;
    int iIndex = 0;

    while (i < iInSize)
    {
        nValue = (int)(b64decode_table[*pInput++] << 18);
        nValue += (int)(b64decode_table[*pInput++] << 12);
        pOutput[iIndex++] = (nValue & 0x00FF0000) >> 16;

        if (*pInput != '=')
        {
            nValue += (int)(b64decode_table[*pInput++] << 6);
            pOutput[iIndex++] = (nValue & 0x0000FF00) >> 8;

            if (*pInput != '=')
            {
                nValue += (int)(b64decode_table[*pInput++]);
                pOutput[iIndex++] = nValue & 0x000000FF;
            }
        }
        i += 4;
    }
    seg->base64_size = (seg->seg_end - seg->seg_begin + 1)*6/8;
}

void lua_base64_trim(lua_segment* seg)
{
    const char* pose = seg->seg_begin;
    while (pose <= seg->seg_end)
    {
        if (isspace(*(unsigned char*)pose))
        {
            ++pose;
            continue;
        }
        else
        {
            seg->seg_begin = pose;
            break;
        }
    }

    pose = seg->seg_end;
    while (pose >= seg->seg_begin)
    {
        if (isspace(*(unsigned char*)pose))
        {
            --pose;
            continue;
        }
        else
        {
            seg->seg_end = pose;
            break;
        }
    }
}

const char* g_key_tables[] = 
{
    "?d z?!2€?+<Iû}½ð?EP'??„ªAñ[ü{~NöB??€æ]8?_",
    "EP'?d z?!2€?+<I?„ªAñ[û}½ð??ü{~NöB€??æ]8?_"
};

void lua_ncbc_decode(lua_segment* seg)
{
    size_t i = 0, j = 0;
    int t = 0;
    char* pIn = seg->base64_cache;
    seg->ncbc_size = 0;
    unsigned char szInput[MAX_CRYPT_SIZE + 1];
    unsigned char szOutput[MAX_CRYPT_SIZE + 1];

    DES_key_schedule key_schedule;
    DES_cblock ivec;
    const_DES_cblock key[1];
    const char* keystring = g_key_tables[*(int*)(pIn)];
    DES_string_to_key(keystring, key);
    DES_set_key_checked(key, &key_schedule);

    size_t iInSize = seg->base64_size;

    if (seg->ncbc_cache_size < (iInSize*2+1))
    {
        if (seg->ncbc_cache)
        {
            delete[] seg->ncbc_cache;
        }

        seg->ncbc_cache_size = iInSize*2+1;
        seg->ncbc_cache = new char[seg->ncbc_cache_size];
    }

    char* pOut = seg->ncbc_cache;

    i = iInSize / MAX_CRYPT_SIZE;
    j = iInSize % MAX_CRYPT_SIZE;

    pIn += sizeof(int);

    t = *(int*)(pIn);
    pIn += sizeof(int);

    memset(szInput, 0, sizeof(szInput));
    memset(szOutput, 0, sizeof(szOutput));

    if (((t + 7) / 8 * 8) >= MAX_CRYPT_SIZE - 8)
    {
        --i;
        j = (t + 7) / 8 * 8;
    }

    for (; i > 0; --i)
    {        
        memcpy(szInput, pIn, MAX_CRYPT_SIZE);

        memset((char*)&ivec, 0, sizeof(ivec));
        DES_ncbc_encrypt(szInput, szOutput, MAX_CRYPT_SIZE, &key_schedule, &ivec, DES_DECRYPT);

        memcpy(pOut, szOutput, MAX_CRYPT_SIZE);

        pIn += MAX_CRYPT_SIZE;
        pOut += MAX_CRYPT_SIZE;
        seg->ncbc_size += MAX_CRYPT_SIZE;        
    }

    memset(szInput, 0, sizeof(szInput));
    memset(szOutput, 0, sizeof(szOutput));

    memcpy(szInput, pIn, j);

    memset((char*)&ivec, 0, sizeof(ivec));
    DES_ncbc_encrypt(szInput, szOutput, (long)j, &key_schedule, &ivec, DES_DECRYPT);

    memcpy(pOut, szOutput, t);
    seg->ncbc_size += t;
}


//////////////////////////////////////////////////////////////////////////

static void* mem_handler( void* ud, void* org_ptr, size_t org_size, size_t new_size )
{
    org_ptr;
    org_size;
    ud;
    if (!new_size)
    {
        //memory_pool_free((HMEMORYPOOL)ud, org_ptr);
        default_memory_pool_free(org_ptr);
        //free(org_ptr);
        return 0;
    }
    else
    {
        //return memory_pool_realloc((HMEMORYPOOL)ud, org_ptr, new_size);
        return default_memory_pool_realloc(org_ptr, new_size);
        //return realloc(org_ptr, new_size);
    }
}

static int nil_call_handler( lua_State* L )
{
    lua_getglobal(L, LUA_C_FUNC_FLAG);

    if (!lua_rawequal(L, -1, -2))
    {
        lua_pop(L, 1);
        return 0;
    }

    CLuaScript* script = (CLuaScript*)(L->user_data);
    if (!script->get_event())
    {
        lua_pop(L, 1);
        return 0;
    }

    const char* func_name = lua_tostring(L, -3);

    lua_c_function_base* func_ptr = script->get_event()->on_register_function(script, func_name);

    if (!func_ptr)
    {
        lua_pop(L, 1);
        return 0;
    }

    lua_pushlightuserdata(L, func_ptr);
    lua_pushcclosure(L, func_ptr->m_c_func, 1);
    lua_pushvalue(L, -4);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    lua_remove(L, -2);

    return 1;
}

const char* lua_err_type(int err_type)
{
    switch (err_type)
    {
    case LUA_ERRRUN:
        return "lua_errrun";
    case LUA_ERRSYNTAX:
        return "lua_errsyntax";
    case LUA_ERRMEM:
        return "lua_errmem";
    case LUA_ERRGCMM:
        return "lua_errgcmm";
    case LUA_ERRERR:
        return "lua_errerr";
    default:
        return "";
    }
}

static int err_handler( lua_State* L )
{
    CLuaScript* script = (CLuaScript*)(L->user_data);

    const char* err_msg = 0;
    size_t err_msg_len = 0;
    if (lua_isstring(L, -1))
    {
        err_msg = lua_tolstring(L, -1, &err_msg_len);
        lua_pop(L, 1);
    }

    std::string err_str;
    err_str.reserve(256);

    err_str.append(lua_err_type(L->status));
    err_str.append(": ");

    if (err_msg_len)
    {
        err_str.append(err_msg, err_msg_len);
    }

    CLuaException e;

    if (script->get_event())
    {
        script->get_event()->on_error(script, err_str.c_str(), err_str.length(), e);
    }

    throw e;
}

static void hook_handler( lua_State* L, lua_Debug* ar)
{
    CLuaScript* script = (CLuaScript*)(L->user_data);

    if (script->get_event())
    {
        script->get_event()->on_count_hook(script, ar);
    }
}

lua_call_c_check::lua_call_c_check(lua_State* L, lua_c_function_base* func_ptr)
{
    m_L = L;
    m_func_ptr = func_ptr;

    CLuaScript* script = (CLuaScript*)(m_L->user_data);

    if (script->get_event())
    {
        script->get_event()->on_pre_call_c(script, m_func_ptr);
    }
}

lua_call_c_check::~lua_call_c_check()
{
    CLuaScript* script = (CLuaScript*)(m_L->user_data);

    if (script->get_event())
    {
        script->get_event()->on_post_call_c(script, m_func_ptr);
    }
}

lua_call_lua_check::lua_call_lua_check(lua_State* L, const char* func_name)
{
    m_L = L;
    m_func_name = func_name;

    CLuaScript* script = (CLuaScript*)(m_L->user_data);

    if (script->get_event())
    {
        script->get_event()->on_pre_call_lua(script, m_func_name);
    }
}

lua_call_lua_check::~lua_call_lua_check()
{
    CLuaScript* script = (CLuaScript*)(m_L->user_data);

    if (script->get_event())
    {
        script->get_event()->on_post_call_lua(script, m_func_name);
    }
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

CLuaScript::CLuaScript(ILuaEvent* evt):m_L(0), m_evt(evt)
{
}

CLuaScript::~CLuaScript()
{
    close_lua();
}

#pragma warning( push )
#pragma warning( disable : 4706 )

typedef struct st_decrypt_lua_data
{
    const char* decrypt_data;
    size_t      decrypt_data_size;
}decrypt_lua_data;

void buffer_decrypt(const char* buffer, size_t buffer_size, lua_file* LF, decrypt_lua_data* data)
{
    const char* p_current;
    const char* p_end;
    const char* p_find;
    int find_encrypt_count = 0;
    size_t lua_buff_size = 0;

    p_current = (char*)buffer;
    p_end = p_current + buffer_size;

    lua_segment* seg = LF->seg_head;

    if (seg->is_use)
    {
        CRUSH_CODE;
    }

    while (p_find = strstr(p_current, ENCRYPT_FLAG))
    {
        if (seg)
        {
            if (seg->is_use)
            {
                CRUSH_CODE;
            }
        }
        else
        {
            seg = create_lua_segment();
            LF->seg_tail->next = seg;
            seg->next = 0;
            LF->seg_tail = seg;
        }

        seg->is_use = true;
        seg->seg_begin = p_current;
        seg->seg_end = p_find-1;
        seg->is_normal = true;

        if (find_encrypt_count%2)
        {
            seg->is_normal = false;
            lua_base64_trim(seg);
            lua_base64_decode(seg);
            lua_ncbc_decode(seg);
            lua_buff_size += seg->ncbc_size;
        }
        else
        {
            lua_buff_size += (seg->seg_end - seg->seg_begin+1);
        }

        seg = seg->next;

        find_encrypt_count++;
        p_current = p_find + strlen(ENCRYPT_FLAG);
    }

    if (p_current < p_end)
    {
        if (seg)
        {
            if (seg->is_use)
            {
                CRUSH_CODE;
            }
        }
        else
        {
            seg = create_lua_segment();
            LF->seg_tail->next = seg;
            seg->next = 0;
            LF->seg_tail = seg;
        }

        seg->is_use = true;
        seg->seg_begin = p_current;
        seg->seg_end = p_end-1;
        seg->is_normal = true;

        if (find_encrypt_count%2)
        {
            seg->is_normal = false;
            lua_base64_trim(seg);
            lua_base64_decode(seg);
            lua_ncbc_decode(seg);
            lua_buff_size += seg->ncbc_size;
        }
        else
        {
            lua_buff_size += (seg->seg_end - seg->seg_begin+1);
        }
    }

    if (find_encrypt_count)
    {
        seg = LF->seg_head;
        char* ptr = LF->buffer;
        while (seg && seg->is_use)
        {
            if (seg->is_normal)
            {
                memcpy(ptr, seg->seg_begin, seg->seg_end - seg->seg_begin+1);
                ptr += seg->seg_end - seg->seg_begin+1;
            }
            else
            {
                memcpy(ptr, seg->ncbc_cache, seg->ncbc_size);
                ptr += seg->ncbc_size;
            }

            seg = seg->next;
        }

        data->decrypt_data = LF->buffer;
    }
    else
    {
        data->decrypt_data = buffer;
    }

    data->decrypt_data_size = lua_buff_size;
}

size_t file_to_buffer(const char* file_path, lua_file* LF, char* err_msg)
{
    FILE* p_file = fopen(file_path, "rb");
    if (!p_file)
    {
        if (err_msg)
        {
            sprintf(err_msg, "´ò¿ª %s ÎÄ¼þÊ§°Ü", file_path);
        }
        goto END_DEAL;
    }

    if (fseek(p_file, 0, SEEK_END))
    {
        if (err_msg)
        {
            sprintf(err_msg, "»ñÈ¡ %s ÎÄ¼þ´óÐ¡Ê§°Ü", file_path);
        }
        goto END_DEAL;
    }

    size_t file_size = ftell(p_file);

    if (fseek(p_file, 0, SEEK_SET))
    {
        if (err_msg)
        {
            sprintf(err_msg, "»ñÈ¡ %s ÎÄ¼þ´óÐ¡Ê§°Ü", file_path);
        }
        goto END_DEAL;
    }

    if (LF->file_content_size < file_size)
    {
        if (LF->file_content)
        {
            delete[] LF->file_content;
        }

        LF->file_content_size = file_size+1;
        LF->file_content = new char[LF->file_content_size];
    }

    if (fread(LF->file_content, file_size, 1, p_file) != 1)
    {
        if (err_msg)
        {
            sprintf(err_msg, "¶ÁÈ¡ %s ÎÄ¼þÄÚÈÝÊ§°Ü", file_path);
        }
        goto END_DEAL;
    }

    fclose(p_file);
    p_file = 0;

    LF->file_content[file_size] = 0;

    return file_size;

END_DEAL:

    if (p_file)
    {
        fclose(p_file);
        p_file = 0;
    }

    return 0;
}

bool CLuaScript::load_file( const char* file_path, lua_file* LF )
{
    char err_msg[512];
    size_t buffer_size = file_to_buffer(file_path, LF, err_msg);
    if (!buffer_size)
    {
        if (m_evt)
        {
            CLuaException e;
            m_evt->on_error(this, err_msg, strlen(err_msg), e);
        }

        reset_lua_life(LF);
        return false;
    }

    decrypt_lua_data data;

    buffer_decrypt(LF->file_content, buffer_size, LF, &data);

    bool load_ret = _load_lua(data.decrypt_data, data.decrypt_data_size, file_path);

    reset_lua_life(LF);

    return load_ret;
}

bool CLuaScript::load_buffer( const char* buffer, size_t buffer_size, const char* module_name, lua_file* LF /*= 0*/ )
{
    decrypt_lua_data data;

    buffer_decrypt(buffer, buffer_size, LF, &data);

    bool load_ret = _load_lua(data.decrypt_data, data.decrypt_data_size, module_name);

    reset_lua_life(LF);

    return load_ret;
}

bool CLuaScript::has_function( const char* lua_function_name )
{
    if (m_L)
    {
        lua_getglobal(m_L, lua_function_name);
        bool ret = (lua_type(m_L, -1) == LUA_TFUNCTION);
        lua_pop(m_L, 1);

        return ret;
    }

    return false;
}

void CLuaScript::close_lua(void)
{
    if (m_L)
    {
        lua_close(m_L);
        m_L = 0;
    }
}

bool CLuaScript::_load_lua( const char* lua_data, size_t lua_data_size, const char* chunk_name)
{
    HRBTREE register_c_func_ptr_list = create_rb_tree(0,0,0);
    HRBTREE unregister_c_func_name_list = create_rb_tree(0,0,0);
    HRBTREE register_c_var_ptr_list = create_rb_tree(0,0,0);
    HRBTREE require_lua_file_list = create_rb_tree(0,0,0);

    lua_State* new_state = 0;
    bool load_ret = false;
    CLuaException e;

    try
    {
        //new_state = lua_newstate(mem_handler, SMemory::default_memory_pool());
        new_state = lua_newstate(mem_handler, 0);
        if (!new_state)
        {
            if (m_evt)
            {
                m_evt->on_error(this, "lua_newstate fail", strlen("lua_newstate fail"), e);
            }
            throw CLuaException();
        }

        new_state->user_data = this;

        lua_atpanic(new_state, err_handler);
        lua_nilcall(new_state, nil_call_handler);

        luaL_openlibs(new_state);

        if (luaL_loadbuffer(new_state, lua_data, lua_data_size, chunk_name) != LUA_OK)
        {
            size_t err_len;
            const char* err = lua_tolstring(new_state, -1, &err_len);

            if (m_evt)
            {
                m_evt->on_error(this, err, err_len, e);
            }

            lua_pop(new_state, -1);

            throw CLuaException();
        }

        if (!_scan_lua_data(lua_data, lua_data_size, 
            register_c_func_ptr_list, unregister_c_func_name_list, register_c_var_ptr_list, require_lua_file_list))
        {
            CRUSH_CODE;
        }

        if (rb_tree_size(require_lua_file_list))
        {
            lua_file* LF = create_lua_life();

            while (rb_tree_size(require_lua_file_list))
            {
                HRBNODE node = rb_first(require_lua_file_list);


                char* lua_file = (char*)rb_node_key_str(node);
                size_t buffer_size = file_to_buffer(lua_file, LF, 0);
                if (buffer_size)
                {
                    decrypt_lua_data data;
                    buffer_decrypt(LF->file_content, buffer_size, LF, &data);
                    _scan_lua_data(data.decrypt_data, data.decrypt_data_size, 
                        register_c_func_ptr_list, unregister_c_func_name_list, register_c_var_ptr_list, require_lua_file_list);
                }
                default_memory_pool_free(lua_file);

                reset_lua_life(LF);

                rb_tree_erase(require_lua_file_list, node);
            }

            destroy_lua_life(LF);
        }

        if (rb_tree_size(unregister_c_func_name_list))
        {
            char err_msg[256];
            HRBNODE unreg_func_name_node = rb_first(unregister_c_func_name_list);
            while (unreg_func_name_node)
            {
                if (m_evt)
                {
                    sprintf(err_msg, "½Å±¾½Ó¿Ú¿âÖÐÕÒ²»µ½ %s:%s", LUA_C_FUNC_FLAG, rb_node_key_str(unreg_func_name_node));
                    m_evt->on_error(this, err_msg, strlen(err_msg), e);
                }
                default_memory_pool_free((char*)rb_node_key_str(unreg_func_name_node));

                unreg_func_name_node = rb_next(unreg_func_name_node);
            }

            throw CLuaException();
        }

        int reg_func_size = (int)rb_tree_size(register_c_func_ptr_list);
        if (reg_func_size)
        {
            reg_func_size = 8 - reg_func_size%8 + reg_func_size;

            lua_createtable(new_state, 8, reg_func_size);
            lua_setglobal(new_state, LUA_C_FUNC_FLAG);
            lua_getglobal(new_state, LUA_C_FUNC_FLAG);

            HRBNODE reg_func_ptr_node = rb_first(register_c_func_ptr_list);
            while (reg_func_ptr_node)
            {
                lua_c_function_base* func_ptr = (lua_c_function_base*)rb_node_key_user(reg_func_ptr_node);

                lua_pushstring(new_state, func_ptr->m_c_function_name);
                lua_pushlightuserdata(new_state, func_ptr);
                lua_pushcclosure(new_state, func_ptr->m_c_func, 1);
                lua_settable(new_state, 2);

                reg_func_ptr_node = rb_next(reg_func_ptr_node);
            }

            lua_pop(new_state, 1);
        }

        if (rb_tree_size(register_c_var_ptr_list))
        {
            HRBNODE reg_var_ptr_node = rb_first(register_c_var_ptr_list);
            while (reg_var_ptr_node)
            {
                lua_c_variable_base* var_ptr = (lua_c_variable_base*)rb_node_key_user(reg_var_ptr_node);

                if (var_ptr->m_var_type == 0)
                {
                    lua_pushinteger(new_state, var_ptr->m_var_int);
                }
                else
                {
                    lua_pushstring(new_state, var_ptr->m_var_str);
                }

                lua_setglobal(new_state, var_ptr->m_c_variable_name);

                reg_var_ptr_node = rb_next(reg_var_ptr_node);
            }
        }

        lua_call(new_state, 0, LUA_MULTRET);

        lua_sethook(new_state, hook_handler, LUA_MASKCOUNT, 2000);

        if (m_L)
        {
            lua_close(m_L);
        }

        m_L = new_state;

        load_ret = true;
    }
    catch (CLuaException& e)
    {
        e;
        if (new_state)
        {
            lua_close(new_state);
        }
    }


    if (register_c_func_ptr_list)
    {
        destroy_rb_tree(register_c_func_ptr_list);
    }

    if (unregister_c_func_name_list)
    {
        destroy_rb_tree(unregister_c_func_name_list);
    }

    if (register_c_var_ptr_list)
    {
        destroy_rb_tree(register_c_var_ptr_list);
    }

    if (require_lua_file_list)
    {
        destroy_rb_tree(require_lua_file_list);
    }

    return load_ret;
}

bool CLuaScript::_scan_lua_data( const char* lua_data, size_t lua_data_size, 
                                HRBTREE register_c_func_list, HRBTREE unregister_c_func_list, 
                                HRBTREE register_c_var_list, HRBTREE require_lua_list )
{
#define SCAN_C_FUNCTION(end_ptr)    if (end_ptr - code_begin > (ptrdiff_t)strlen(LUA_C_FUNC_FLAG))\
    {\
    char tmp = *(char*)end_ptr;\
    *(char*)end_ptr = 0;\
    _scan_c_functions(code_begin, end_ptr - code_begin, register_c_func_list, unregister_c_func_list);\
    *(char*)end_ptr = tmp;\
}

#define SCAN_C_VARIABLE(end_ptr)    if (end_ptr - code_begin > (ptrdiff_t)strlen(LUA_C_VAR_FLAG))\
    {\
    char tmp = *(char*)end_ptr;\
    *(char*)end_ptr = 0;\
    _scan_c_variables(code_begin, end_ptr - code_begin, register_c_var_list);\
    *(char*)end_ptr = tmp;\
}

#define SCAN_LUA_REQUIRE(end_ptr)   if (end_ptr - code_begin > (ptrdiff_t)strlen(LUA_REQUIRE_FLAG))\
    {\
    _scan_lua_require(code_begin, end_ptr - code_begin, require_lua_list);\
}

    typedef struct st_seg_info 
    {
        const char* begin;
        const char* end;
        const char* proc;
        size_t      level;

        // note type

        // string type
        // '...     1
        // "...     2

        // bracket type
        // [        1
        // [=       2
        // [=...[   3
        // ]        4
        // ]=       5
        // ]=...]   6
        size_t      type;
    }seg_info;

    seg_info* note_seg = 0;
    seg_info* string_seg = 0;
    seg_info* bracket_seg_begin = 0;
    seg_info* bracket_seg_end = 0;

    const char* p_current = lua_data;
    const char* p_end = lua_data + lua_data_size;
    const char* code_begin = lua_data;

    seg_info note_seg_info;
    seg_info string_seg_info;
    seg_info bracket_seg_begin_info;
    seg_info bracket_seg_end_info;

    for (;;)
    {
        switch (*p_current)
        {
        case '-':
            {
                if ((!note_seg) && (!string_seg) && (!bracket_seg_begin))
                {
                    ++p_current;
                    if (p_current >= p_end)
                    {
                        const char* scan_end_ptr = p_current;
                        SCAN_C_FUNCTION(scan_end_ptr);
                        SCAN_C_VARIABLE(scan_end_ptr);
                        SCAN_LUA_REQUIRE(scan_end_ptr);
                        return true;
                    }

                    if (*p_current == '-')
                    {
                        const char* scan_end_ptr = p_current-1;
                        SCAN_C_FUNCTION(scan_end_ptr);
                        SCAN_C_VARIABLE(scan_end_ptr);
                        SCAN_LUA_REQUIRE(scan_end_ptr);

                        note_seg = &note_seg_info;
                        note_seg->begin = (p_current-1);
                        note_seg->end = p_current;
                    }
                    else
                        --p_current;
                }
            }
            break;
        case '[':
            {
                if (!string_seg)
                {
                    if (bracket_seg_begin)
                    {
                        if (bracket_seg_begin->type == 2)
                        {
                            if (p_current - bracket_seg_begin->proc == 1)
                            {
                                //[=...[
                                bracket_seg_begin->type = 3;
                                bracket_seg_begin->proc = p_current;
                                bracket_seg_begin->end = p_current;
                            }
                        }
                    }
                    else
                    {
                        //INC_CHECK;
                        ++p_current;
                        if (p_current >= p_end)
                        {
                            if (!note_seg)
                            {
                                const char* scan_end_ptr = p_current;
                                SCAN_C_FUNCTION(scan_end_ptr);
                                SCAN_C_VARIABLE(scan_end_ptr);
                                SCAN_LUA_REQUIRE(scan_end_ptr);
                            }

                            return true;
                        }

                        if (*p_current == '[')
                        {
                            //[[
                            bracket_seg_begin = &bracket_seg_begin_info;
                            bracket_seg_begin->begin = (p_current-1);
                            bracket_seg_begin->end = p_current;
                            bracket_seg_begin->level = 0;
                            bracket_seg_begin->proc = p_current;
                            bracket_seg_begin->type = 3;
                        }
                        else if (*p_current == '=')
                        {
                            //[=
                            bracket_seg_begin = &bracket_seg_begin_info;
                            bracket_seg_begin->begin = (p_current-1);
                            bracket_seg_begin->end = 0;
                            bracket_seg_begin->level = 1;
                            bracket_seg_begin->proc = p_current;
                            bracket_seg_begin->type = 2;
                        }
                        else
                            --p_current;

                        if (note_seg && bracket_seg_begin)
                        {
                            if (bracket_seg_begin->begin - note_seg->end > 1)
                            {
                                //--...[
                                bracket_seg_begin = 0;
                            }
                        }

                        if (bracket_seg_begin)
                        {
                            if (!note_seg)
                            {
                                const char* scan_end_ptr = p_current-1;
                                SCAN_C_FUNCTION(scan_end_ptr);
                                SCAN_C_VARIABLE(scan_end_ptr);
                                SCAN_LUA_REQUIRE(scan_end_ptr);
                            }
                        }
                    }
                }
            }
            break;
        case ']':
            {
                if (!string_seg)
                {
                    if (bracket_seg_begin)
                    {
                        if (bracket_seg_begin->type == 2)
                        {
                            return false;
                        }
                        else if (bracket_seg_begin->type == 3)
                        {
                            if (bracket_seg_end)
                            {
                                if (bracket_seg_end->type == 5)
                                {
                                    if (p_current - bracket_seg_end->proc == 1)
                                    {
                                        //]=...]
                                        bracket_seg_end->type = 6;
                                        bracket_seg_end->end = p_current;
                                        bracket_seg_end->proc = p_current;
                                    }
                                    else
                                    {
                                        bracket_seg_end = 0;

                                        //INC_CHECK;
                                        ++p_current;
                                        if (p_current >= p_end)
                                        {
                                            return false;
                                        }

                                        if (*p_current == ']')
                                        {
                                            //]]
                                            bracket_seg_end = &bracket_seg_end_info;
                                            bracket_seg_end->begin = (p_current - 1);
                                            bracket_seg_end->end = p_current;
                                            bracket_seg_end->level = 0;
                                            bracket_seg_end->proc = p_current;
                                            bracket_seg_end->type = 6;

                                        }
                                        else if (*p_current == '=')
                                        {
                                            //]=
                                            bracket_seg_end = &bracket_seg_end_info;
                                            bracket_seg_end->begin = (p_current - 1);
                                            bracket_seg_end->end = 0;
                                            bracket_seg_end->level = 1;
                                            bracket_seg_end->proc = p_current;
                                            bracket_seg_end->type = 5;
                                        }
                                        else
                                            --p_current;
                                    }
                                }
                            }
                            else
                            {
                                //INC_CHECK;
                                ++p_current;
                                if (p_current >= p_end)
                                {
                                    return false;
                                }

                                if (*p_current == ']')
                                {
                                    //]]
                                    bracket_seg_end = &bracket_seg_end_info;
                                    bracket_seg_end->begin = (p_current - 1);
                                    bracket_seg_end->end = p_current;
                                    bracket_seg_end->level = 0;
                                    bracket_seg_end->proc = p_current;
                                    bracket_seg_end->type = 6;

                                }
                                else if (*p_current == '=')
                                {
                                    //]=
                                    bracket_seg_end = &bracket_seg_end_info;
                                    bracket_seg_end->begin = (p_current - 1);
                                    bracket_seg_end->end = 0;
                                    bracket_seg_end->level = 1;
                                    bracket_seg_end->proc = p_current;
                                    bracket_seg_end->type = 5;
                                }
                                else
                                    --p_current;
                            }

                            if (bracket_seg_begin && bracket_seg_end)
                            {
                                if (bracket_seg_end->type == 6)
                                {
                                    if (bracket_seg_end->level == bracket_seg_begin->level)
                                    {
                                        //[=...[ ... ]=...]
                                        if (note_seg)
                                        {
                                            note_seg = 0;
                                        }
                                        bracket_seg_begin = 0;
                                        code_begin = p_current+1;
                                    }
                                    bracket_seg_end = 0;
                                }
                            }
                        }
                        else
                            return false;
                    }
                }
            }
            break;
        case '=':
            {
                if (bracket_seg_begin)
                {
                    if (bracket_seg_begin->type == 2)
                    {
                        if (p_current - bracket_seg_begin->proc == 1)
                        {
                            bracket_seg_begin->level++;
                            bracket_seg_begin->proc = p_current;
                        }
                    }
                }

                if (bracket_seg_end)
                {
                    if (bracket_seg_end->type == 5)
                    {
                        if (p_current - bracket_seg_end->proc == 1)
                        {
                            bracket_seg_end->level++;
                            bracket_seg_end->proc = p_current;
                        }
                    }
                }
            }
            break;
        case '\n':
            {
                if (!string_seg && !bracket_seg_begin)
                {
                    if (note_seg)
                    {
                        note_seg = 0;
                        //-- ...
                        code_begin = p_current+1;
                    }
                }
            }
            break;
        case '\'':
            {
                if (!bracket_seg_begin && !note_seg)
                {
                    if (!string_seg)
                    {
                        const char* scan_end_ptr = p_current;
                        SCAN_C_FUNCTION(scan_end_ptr);
                        SCAN_C_VARIABLE(scan_end_ptr);
                        SCAN_LUA_REQUIRE(scan_end_ptr);
                        //'...
                        string_seg = &string_seg_info;
                        string_seg->begin = p_current;
                        string_seg->end = 0;
                        string_seg->level = 0;
                        string_seg->proc = p_current;
                        string_seg->type = 1;
                    }
                    else
                    {
                        if (string_seg->type == 1)
                        {
                            //'...'
                            code_begin = p_current+1;
                            string_seg = 0;
                        }
                    }
                }
            }
            break;
        case '\"':
            {
                if (!bracket_seg_begin && !note_seg)
                {
                    if (!string_seg)
                    {
                        const char* scan_end_ptr = p_current;
                        SCAN_C_FUNCTION(scan_end_ptr);
                        SCAN_C_VARIABLE(scan_end_ptr);
                        SCAN_LUA_REQUIRE(scan_end_ptr);
                        //"...
                        string_seg = &string_seg_info;
                        string_seg->begin = p_current;
                        string_seg->end = 0;
                        string_seg->level = 0;
                        string_seg->proc = p_current;
                        string_seg->type = 2;
                    }
                    else
                    {
                        if (string_seg->type == 2)
                        {
                            //"..."
                            code_begin = p_current+1;
                            string_seg = 0;
                        }
                    }
                }
            }
            break;
        }

        //INC_CHECK;
        ++p_current;
        if (p_current >= p_end)
        {
            if (string_seg)
            {
                return false;
            }

            if (bracket_seg_begin)
            {
                return false;
            }

            if (!note_seg)
            {
                SCAN_C_FUNCTION(p_current);
                SCAN_C_VARIABLE(p_current);
                SCAN_LUA_REQUIRE(p_current);
            }

            return true;
        }
    }
}


void CLuaScript::_scan_c_functions( const char* lua_data, size_t lua_data_size, HRBTREE register_c_func_list, HRBTREE unregister_c_func_list )
{
    const char* p_current = lua_data;
    const char* p_end = lua_data + lua_data_size;
    const char* p_find;

    while (p_find = strstr(p_current, LUA_C_FUNC_FLAG))
    {
        const char* p_func_begin = p_find + strlen(LUA_C_FUNC_FLAG);

        if (p_func_begin >= p_end)
        {
            return;
        }

        if (*p_func_begin != ':')
        {
            p_func_begin = strstr(p_func_begin, ":");
        }

        if (!p_func_begin)
        {
            return;
        }

        p_func_begin++;

        const char* p_func_end = strstr(p_func_begin, "(");

        if (!p_func_end)
        {
            return;
        }

        *((char*)(p_func_end)) = 0;

        if (!rb_tree_find_str(unregister_c_func_list, p_func_begin))
        {
            lua_c_function_base* func_ptr = 0;
            if (m_evt)
            {
                func_ptr = m_evt->on_register_function(this, p_func_begin);
            }

            if (!func_ptr)
            {
                const char* new_func_begin = p_func_begin;
                const char* new_func_end = p_func_end;
                const char* pose = p_func_begin;
                while (pose < p_func_end)
                {
                    if (isspace(*(unsigned char*)pose))
                    {
                        ++pose;
                        continue;
                    }
                    else
                    {
                        new_func_begin = pose;
                        break;
                    }
                }

                pose = p_func_end-1;
                while (pose > p_func_begin)
                {
                    if (isspace(*(unsigned char*)pose))
                    {
                        --pose;
                        continue;
                    }
                    else
                    {
                        new_func_end = pose;
                        break;
                    }
                }

                ++new_func_end;

                char tmp = *new_func_end;
                *((char*)(new_func_end)) = 0;

                if (m_evt)
                {
                    func_ptr = m_evt->on_register_function(this, new_func_begin);
                }
                *((char*)(new_func_end)) = tmp;
            }

            if (func_ptr)
            {
                HRBNODE exist_node;
                rb_tree_try_insert_user(register_c_func_list, func_ptr, 0, &exist_node);
            }
            else
            {
                size_t func_len = strlen(p_func_begin);
                char* unregister_func = (char*)default_memory_pool_alloc(func_len+1);
                memcpy(unregister_func, p_func_begin, func_len);
                unregister_func[func_len] = 0;
                rb_tree_insert_str(unregister_c_func_list, unregister_func, 0);
            }
        }

        *((char*)(p_func_end)) = '(';

        p_current = p_func_end;
    }

}

const char* _get_var_end(const char* var_begin, const char* data_end)
{
    const char* ptr = var_begin;

    while (ptr < data_end)
    {
        switch (*ptr)
        {
        case '+': case '*': case '/': case ']':
        case '%': case '^': case '#': case '=':
        case '~': case '<': case '>': case '(':
        case ')': case '{': case '}': case ';':
        case ',': case'\r': case'\n': case ' ':
        case'\f': case'\t': case'\v': case '[':
        case '"': case '-': case '.':
            {
                return ptr;
            }
        	break;
        default:
            {
                ++ptr;
            }
        }
    }

    return 0;
}

void CLuaScript::_scan_c_variables( const char* lua_data, size_t lua_data_size, HRBTREE register_c_var_list )
{
    const char* p_current = lua_data;
    const char* p_end = lua_data + lua_data_size;
    const char* p_find;

    while (p_find = strstr(p_current, LUA_C_VAR_FLAG))
    {
        const char* p_var_begin = p_find;
        const char* p_var_end = _get_var_end(p_var_begin, p_end);

        if (!p_var_end)
        {
            return;
        }

        char tmp = *p_var_end;
        *(char*)p_var_end = 0;

        if (m_evt)
        {
            lua_c_variable_base* var_ptr = m_evt->on_register_variable(this, p_var_begin);

            if (var_ptr)
            {
                HRBNODE exist_node;
                rb_tree_try_insert_user(register_c_var_list, var_ptr, 0, &exist_node);
            }
        }

        *(char*)p_var_end = tmp;

        p_current = p_var_end;
    }
}

void CLuaScript::_scan_lua_require( const char* lua_data, size_t lua_data_size, HRBTREE require_lua_list )
{
    const char* p_current = lua_data;
    const char* p_end = lua_data + lua_data_size;
    const char* p_find;

    char tmp = lua_data[lua_data_size];
    ((char*)lua_data)[lua_data_size] = 0;
    while (p_find = strstr(p_current, LUA_REQUIRE_FLAG))
    {
        ((char*)lua_data)[lua_data_size] = tmp;
        const char* p_require_begin = p_find + strlen(LUA_REQUIRE_FLAG);

        if (p_require_begin >= p_end)
        {
            return;
        }

        if (*p_require_begin != '(')
        {
            while (isspace(*(unsigned char*)p_require_begin))
            {
                ++p_require_begin;
                if (p_require_begin >= p_end)
                {
                    return;
                }
            }

            if (*p_require_begin != '(')
            {
                return;
            }
        }

        if (!p_require_begin)
        {
            return;
        }

        ++p_require_begin;

        const char* p_require_end = strstr(p_require_begin, ")");

        if (!p_require_end)
        {
            return;
        }

        if (m_evt)
        {

            const char* new_require_begin = p_require_begin;
            const char* new_require_end = p_require_end;
            const char* pose = p_require_begin;
            while (pose < p_require_end)
            {
                if (isspace(*(unsigned char*)pose))
                {
                    ++pose;
                    continue;
                }
                else if (*pose == '\"')
                {
                    ++pose;
                    continue;
                }
                else
                {
                    new_require_begin = pose;
                    break;
                }
            }

            pose = p_require_end-1;
            while (pose > p_require_begin)
            {
                if (isspace(*(unsigned char*)pose))
                {
                    --pose;
                    continue;
                }
                else if (*pose == '\"')
                {
                    --pose;
                    continue;
                }
                else
                {
                    new_require_end = pose;
                    break;
                }
            }

            ++new_require_end;
            char new_tmp = *new_require_end;
            *(char*)new_require_end = 0;

            char* require_path = (char*)default_memory_pool_alloc(512);
            m_evt->on_require_lua_path(this, new_require_begin, require_path);
            rb_tree_insert_str(require_lua_list, require_path, 0);

            *(char*)new_require_end = new_tmp;
        }

        ((char*)lua_data)[lua_data_size] = 0;
        p_current = p_require_end;
    }

    ((char*)lua_data)[lua_data_size] = tmp;
}

#pragma warning( pop )

//////////////////////////////////////////////////////////////////////////





