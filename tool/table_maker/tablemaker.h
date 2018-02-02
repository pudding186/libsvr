#pragma once

#include <string>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <algorithm>

enum column_data_type
{
    col_string = 0,
    col_uint8,
    col_int8,
    col_uint16,
    col_int16,
    col_uint32,
    col_int32,
    col_uint64,
    col_int64,
};

typedef std::vector<std::string> StrVec;

typedef struct st_column_info
{
    std::string m_col_name;
    std::string m_col_note;
    std::string m_col_type;
    column_data_type m_data_type;
}column_info;

typedef struct st_key_info
{
    std::set<std::string>   m_key_list;
    std::list<std::string>  m_key_order;
}key_info;

typedef std::map<std::string, column_info> table_column_info;
typedef std::map<std::set<std::string>, key_info> table_key_info;
class CTableMaker
{
public:
    CTableMaker(const std::string& table_name);
    ~CTableMaker(void);

    std::string add_column(column_info& info);
    std::string add_key(key_info& info);
    bool gen_code(const std::string& path, std::string& err);

    table_column_info get_table_columns(void){return m_table_column;}
    std::string get_table_name(void){return m_table_name;}
protected:
    void _print_data_struct(FILE* hpp_file);
    void _print_class(FILE* hpp_file);
    void _print_func_GetSize(FILE* hpp_file);
    void _print_func_GetFieldCount(FILE* hpp_file);
    void _print_func_At(FILE* hpp_file);
    void _print_func_FillData(FILE* hpp_file);
    void _print_func_FillMapping(FILE* hpp_file);
    void _print_func_AllocRow(FILE* hpp_file);
    void _print_func_FreeRow(FILE* hpp_file);
    void _print_func_QuickMapping(FILE* hpp_file);
    void _print_func_ReleaseMapping(FILE* hpp_file);
    void _print_func_Release(FILE* hpp_file);
    void _print_func_Get(FILE* hpp_file);
    void _print_func_Load(FILE* hpp_file);
    void _print_func_ReLoad(FILE* hpp_file);
    void _print_func_Construct_Destruct(FILE* hpp_file);
    void _print_func_ReLoadEx(FILE* hpp_file);
    void _print_func_InitColInfo_UnInitColInfo_GetColVar(FILE* hpp_file);
protected:
    std::string         m_table_name;
    table_column_info   m_table_column;
    StrVec              m_table_column_arry;
    table_key_info      m_table_key;

    std::string         m_struct_name;
    std::string         m_class_name;
};
