#include "../include/type_def.h"
#include "../include/file_help.h"
#include "../include/memory_pool.h"

#include <Windows.h>

typedef struct st_dir_file_node 
{
    struct st_dir_file_node* next;
    char dir_file_full_path[MAX_PATH];
}dir_file_node;

size_t alloc_count = 0;

void pop_front(dir_file_node** head, dir_file_node** tail, HMEMORYUNIT dir_file_node_unit)
{
    if (*head)
    {
        if (*head == *tail)
        {
            memory_unit_free(dir_file_node_unit, *head);
            --alloc_count;
            *head = 0;
            *tail = 0;
        }
        else
        {
            dir_file_node* tmp = *head;
            *head = (*head)->next;
            memory_unit_free(dir_file_node_unit, tmp);
            --alloc_count;
        }
    }
}

void push_back(dir_file_node** head, dir_file_node** tail, dir_file_node* node)
{
    if (*tail)
    {
        (*tail)->next = node;
        *tail = node;
        node->next = 0;
    }
    else
    {
        *head = node;
        *tail = node;
        node->next = 0;
    }
}

void clear(dir_file_node* head, HMEMORYUNIT dir_file_node_unit)
{
    while (head)
    {
        dir_file_node* node = head;
        memory_unit_free(dir_file_node_unit, node);
        --alloc_count;
        head = head->next;
    }

    destroy_memory_unit(dir_file_node_unit);
}

bool for_each_file(const char* dir, pfn_file do_file, pfn_dir do_dir, void* user_data)
{
    HMEMORYUNIT dir_file_node_unit = create_memory_unit(sizeof(dir_file_node));
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    dir_file_node* node = (dir_file_node*)memory_unit_alloc(dir_file_node_unit, 512);
    
    dir_file_node* dir_list_head = 0;
    dir_file_node* dir_list_tail = 0;

    ++alloc_count;

    strncpy(node->dir_file_full_path, dir, sizeof(node->dir_file_full_path));

    switch (node->dir_file_full_path[strlen(node->dir_file_full_path)-1])
    {
    case '/':
    case '\\':
        {
            strcat(node->dir_file_full_path, "*");
        }
        break;
    case '*':
        {

        }
        break;
    default:
        {
            strcat(node->dir_file_full_path, "/*");
        }
    }

    dir_list_head = node;
    dir_list_tail = node;
    node->next = 0;

    do 
    {
        hFind = FindFirstFile(dir_list_head->dir_file_full_path, &FindFileData);

        dir_list_head->dir_file_full_path[strlen(dir_list_head->dir_file_full_path)-1] = 0;

        if (INVALID_HANDLE_VALUE == hFind)
        {
            clear(dir_list_head, dir_file_node_unit);
            return false;
        }
        else
        {
            if ((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) && (strcmp(FindFileData.cFileName, ".")!=0) &&(strcmp(FindFileData.cFileName, "..")!=0))
            {
                node = (dir_file_node*)memory_unit_alloc(dir_file_node_unit, 512);
                ++alloc_count;
                strncpy(node->dir_file_full_path, dir_list_head->dir_file_full_path, sizeof(node->dir_file_full_path));
                strcat(node->dir_file_full_path, FindFileData.cFileName);
                strcat(node->dir_file_full_path, "/");

                if (do_dir)
                {
                    if (!do_dir(node->dir_file_full_path, user_data))
                    {
                        clear(dir_list_head, dir_file_node_unit);
                        return false;
                    }
                }

                strcat(node->dir_file_full_path, "*");
                push_back(&dir_list_head, &dir_list_tail, node);
            }
            else
            {
                if ((strcmp(FindFileData.cFileName, ".")!=0) && (strcmp(FindFileData.cFileName, "..")!=0))
                {
                    if (do_file)
                    {
                        if (!do_file(dir_list_head->dir_file_full_path, FindFileData.cFileName, user_data))
                        {
                            clear(dir_list_head, dir_file_node_unit);
                            return false;
                        }
                    }
                }
            }

            while (FindNextFile(hFind, &FindFileData) != 0)
            {
                if ((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) && (strcmp(FindFileData.cFileName, ".")!=0) &&(strcmp(FindFileData.cFileName, "..")!=0))
                {
                    node = (dir_file_node*)memory_unit_alloc(dir_file_node_unit, 512);
                    ++alloc_count;
                    strncpy(node->dir_file_full_path, dir_list_head->dir_file_full_path, sizeof(node->dir_file_full_path));
                    strcat(node->dir_file_full_path, FindFileData.cFileName);
                    strcat(node->dir_file_full_path, "/");

                    if (do_dir)
                    {
                        if (!do_dir(node->dir_file_full_path, user_data))
                        {
                            clear(dir_list_head, dir_file_node_unit);
                            return false;
                        }
                    }

                    strcat(node->dir_file_full_path, "*");
                    push_back(&dir_list_head, &dir_list_tail, node);
                }
                else
                {
                    if ((strcmp(FindFileData.cFileName, ".")!=0) && (strcmp(FindFileData.cFileName, "..")!=0))
                    {
                        if (do_file)
                        {
                            if (!do_file(dir_list_head->dir_file_full_path, FindFileData.cFileName, user_data))
                            {
                                clear(dir_list_head, dir_file_node_unit);
                                return false;
                            }
                        }
                    }
                }
            }
        }

        pop_front(&dir_list_head, &dir_list_tail, dir_file_node_unit);
        FindClose(hFind);

    } while (dir_list_head);

    destroy_memory_unit(dir_file_node_unit);
    return true;
}