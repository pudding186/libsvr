#include "StdAfx.h"
#include "tablemaker.h"

void SplitString(const std::string& strContent, char cSpliter, std::vector<std::string>& vecParams, size_t nMaxSplit/* = 0*/)
{
    std::string::size_type iStart = 0, iPos;
    iPos = strContent.find(cSpliter, iStart);
    while(std::string::npos != iPos)
    {
        if(iPos > iStart)
        {
            std::string strParam = strContent.substr(iStart, iPos - iStart);
            vecParams.push_back(strParam);

            if (nMaxSplit && vecParams.size() == nMaxSplit - 1)
            {
                iStart = iPos + 1;
                break;
            }
        }
        iStart = iPos + 1;
        iPos = strContent.find(cSpliter, iStart);
    }

    if(iStart < strContent.size())
    {
        std::string strParam = strContent.substr(iStart, strContent.size() - iStart);
        vecParams.push_back(strParam);
    }
}

CTableMaker::CTableMaker(const std::string& table_name)
{
    m_table_name = table_name;
    std::transform(m_table_name.begin(), m_table_name.end(), m_table_name.begin(), ::tolower);
    

    m_table_column.clear();
    m_table_key.clear();

    std::vector<std::string> name_list;

    SplitString(m_table_name, '_', name_list, 0);

    //m_struct_name = m_table_name;
    //std::transform(m_struct_name.begin(), m_struct_name.begin()+1, m_struct_name.begin(), ::toupper);

    for (size_t i = 0; i < name_list.size(); i++)
    {
        std::transform(name_list[i].begin(), name_list[i].begin()+1, name_list[i].begin(), ::toupper);
    }

    for (size_t i = 0; i < name_list.size(); i++)
    {
        m_struct_name += name_list[i];
    }

    m_class_name = m_struct_name + "Config";
}

CTableMaker::~CTableMaker(void)
{
}

std::string CTableMaker::add_column( column_info& info )
{
    if (m_table_column.find(info.m_col_name) != m_table_column.end())
    {
        std::string err = "表中有重复的字段名 " + info.m_col_name;

        return err;
    }

    m_table_column[info.m_col_name] = info;

    m_table_column_arry.push_back(info.m_col_name);

    return "";
}

std::string CTableMaker::add_key( key_info& info )
{
    if (m_table_key.find(info.m_key_list) != m_table_key.end())
    {
        std::string err = "表中有重复的索引, 索引组成: ";

        for (std::set<std::string>::iterator it = info.m_key_list.begin();
            it != info.m_key_list.end(); ++it)
        {
            err += (*it) + " ";
        }

        return err;
    }

    m_table_key[info.m_key_list] = info;

    return "";
}

bool CTableMaker::gen_code( const std::string& path, std::string& err )
{
    std::string hpp_file_path = path + m_table_name + "_table.hpp";
    std::string cpp_file_path = path + m_table_name + "_table.cpp";

    FILE* hpp_file = fopen(hpp_file_path.c_str(), "wb");

    if (!hpp_file)
    {
        err = strerror(errno);
        goto ERROR_DEAL;
    }

    fprintf(hpp_file, "/// Generate by table_gen tools, PLEASE DO NOT EDIT IT!\r\n");
    fprintf(hpp_file, "#pragma once\r\n");
    fprintf(hpp_file, "#include \"db_include.h\"\r\n");
    fprintf(hpp_file, "#include \"table_help.h\"\r\n");
    fprintf(hpp_file, "\r\n");
    fprintf(hpp_file, "namespace DATA\r\n");
    fprintf(hpp_file, "{\r\n");
    _print_data_struct(hpp_file);
    _print_class(hpp_file);
    fprintf(hpp_file, "}\r\n");
    fprintf(hpp_file, "SET_POD_TYPE(DATA::%s);\r\n\r\n", m_struct_name.c_str());

    fclose(hpp_file);

    FILE* cpp_file = fopen(cpp_file_path.c_str(), "wb");
    if (!cpp_file)
    {
        goto ERROR_DEAL;
    }

    fprintf(cpp_file, "#include \"stdafx.h\"\r\n");
    fprintf(cpp_file, "#include \"%s_table.hpp\"\r\n", m_table_name.c_str());
    fprintf(cpp_file, "namespace DATA\r\n");
    fprintf(cpp_file, "{\r\n");
    fprintf(cpp_file, "    INSTANCE_SINGLETON(%s)\r\n", m_class_name.c_str());
    fprintf(cpp_file, "}\r\n");

    fclose(cpp_file);
    return true;

ERROR_DEAL:
    return false;
}

void FirstCaps(std::string& src)
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    std::transform(src.begin(), src.begin()+1, src.begin(), ::toupper);
}

void CTableMaker::_print_data_struct( FILE* hpp_file )
{
    fprintf(hpp_file, "typedef struct st_%s\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "{\r\n");

    std::vector<column_info*> type_64;
    std::vector<column_info*> type_32;
    std::vector<column_info*> type_16;
    std::vector<column_info*> type_8;

    type_64.reserve(64);
    type_32.reserve(64);
    type_16.reserve(64);
    type_8.reserve(64);

    size_t max_name_length = 0;

    for (table_column_info::iterator it = m_table_column.begin();
        it != m_table_column.end(); ++it)
    {
        column_info& info = it->second;

        switch (info.m_data_type)
        {
        case col_string:
            {
                type_64.push_back(&info);
            }
        	break;
        case col_uint8:
        case col_int8:
            {
                type_8.push_back(&info);
            }
            break;
        case col_uint16:
        case col_int16:
            {
                type_16.push_back(&info);
            }
            break;
        case col_uint32:
        case col_int32:
            {
                type_32.push_back(&info);
            }
            break;
        case col_uint64:
        case col_int64:
            {
                type_64.push_back(&info);
            }
            break;
        }

        if (info.m_col_name.length() > max_name_length)
        {
            max_name_length = info.m_col_name.length();

            max_name_length = 8 - max_name_length%8 + max_name_length;
        }
    }

    for (size_t i = 0; i < type_64.size(); i++)
    {
        column_info* info = type_64[i];

        fprintf(hpp_file, "    %-*s %s;%-*s/// %s\r\n", 19, 
            info->m_col_type.c_str(), info->m_col_name.c_str(), max_name_length - info->m_col_name.length(), " ", info->m_col_note.c_str());
    }

    for (size_t i = 0; i < type_32.size(); i++)
    {
        column_info* info = type_32[i];

        fprintf(hpp_file, "    %-*s %s;%-*s/// %s\r\n", 19, 
            info->m_col_type.c_str(), info->m_col_name.c_str(), max_name_length - info->m_col_name.length(), " ", info->m_col_note.c_str());
    }

    for (size_t i = 0; i < type_16.size(); i++)
    {
        column_info* info = type_16[i];

        fprintf(hpp_file, "    %-*s %s;%-*s/// %s\r\n", 19, 
            info->m_col_type.c_str(), info->m_col_name.c_str(), max_name_length - info->m_col_name.length(), " ", info->m_col_note.c_str());
    }

    for (size_t i = 0; i < type_8.size(); i++)
    {
        column_info* info = type_8[i];

        fprintf(hpp_file, "    %-*s %s;%-*s/// %s\r\n", 19, 
            info->m_col_type.c_str(), info->m_col_name.c_str(), max_name_length - info->m_col_name.length(), " ", info->m_col_note.c_str());
    }

    fprintf(hpp_file, "\r\n");

    fprintf(hpp_file, "}%s;\r\n\r\n", m_struct_name.c_str());
}

void CTableMaker::_print_class( FILE* hpp_file )
{
    fprintf(hpp_file, "class %s\r\n", m_class_name.c_str());
    fprintf(hpp_file, "{\r\n");
    fprintf(hpp_file, "    DECLARE_SINGLETON(%s)\r\n", m_class_name.c_str());
    fprintf(hpp_file, "private:\r\n");

    size_t max_member_length = m_struct_name.length()+2;
    max_member_length = 8 - max_member_length%8 + max_member_length;
    std::string tmp = m_struct_name + "**";

    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        key_info& info = it->second;
        fprintf(hpp_file, "    %-*s m_data_map%u;\r\n", max_member_length, "HRBTREE", key_count);
    }

    fprintf(hpp_file, "    %-*s m_col_info_map;\r\n", max_member_length, "HRBTREE");
    fprintf(hpp_file, "    %-*s m_data_arry;\r\n", max_member_length, tmp.c_str());
    fprintf(hpp_file, "    %-*s m_data_arry_size;\r\n", max_member_length, "size_t");
    fprintf(hpp_file, "public:\r\n");
    _print_func_At(hpp_file);
    _print_func_GetFieldCount(hpp_file);
    _print_func_GetSize(hpp_file);
    _print_func_FillData(hpp_file);
    _print_func_FillMapping(hpp_file);
    _print_func_AllocRow(hpp_file);
    _print_func_FreeRow(hpp_file);
    _print_func_QuickMapping(hpp_file);
    _print_func_ReleaseMapping(hpp_file);
    _print_func_Release(hpp_file);
    _print_func_Get(hpp_file);
    _print_func_Load(hpp_file);
    _print_func_ReLoad(hpp_file);
    _print_func_ReLoadEx(hpp_file);
    _print_func_Construct_Destruct(hpp_file);
    _print_func_InitColInfo_UnInitColInfo_GetColVar(hpp_file);
    fprintf(hpp_file, "};\r\n");
    fprintf(hpp_file, "#define s%s (*DATA::%s::Instance())\r\n", m_class_name.c_str(), m_class_name.c_str());

}

void CTableMaker::_print_func_GetSize( FILE* hpp_file )
{
    fprintf(hpp_file, "    size_t GetSize(void)\r\n");
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        return m_data_arry_size;\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_GetFieldCount( FILE* hpp_file )
{
    fprintf(hpp_file, "    size_t GetFieldCount(void)\r\n");
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        return %u;\r\n", m_table_column.size());
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_At( FILE* hpp_file )
{
    fprintf(hpp_file, "    %s* At(size_t index)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        return m_data_arry[index];\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_FillData( FILE* hpp_file )
{
    fprintf(hpp_file, "    void FillData(%s* row, TiXmlElement* element)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        const char* value = 0;\r\n");
    fprintf(hpp_file, "        TiXmlAttribute* cur_attr = element->FirstAttribute();\r\n");
    //for (table_column_info::iterator it = m_table_column.begin();
    //    it != m_table_column.end(); ++it)
    for (size_t i = 0; i < m_table_column_arry.size(); i++)
    {
        column_info& info = m_table_column.find(m_table_column_arry[i])->second;
        //column_info& info = it->second;
        //fprintf(hpp_file, "        value = element->Attribute(\"%s\");\r\n", info.m_col_name.c_str());
        fprintf(hpp_file, "        value = order_attribute(\"%s\", element, &cur_attr);\r\n", info.m_col_name.c_str());

        switch (info.m_data_type)
        {
        case col_string:
            {
                fprintf(hpp_file, "        if (value)\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            size_t str_len = strlen(value);\r\n");
                fprintf(hpp_file, "            if (row->%s)\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "            {\r\n");
                fprintf(hpp_file, "                S_DELETE(row->%s);\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "            }\r\n");
                fprintf(hpp_file, "            row->%s = S_NEW(char, str_len + 1);\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "            memcpy(row->%s, value, str_len);\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "            ((row->%s))[str_len] = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
                fprintf(hpp_file, "        else\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
            }
            break;
        case col_uint8:
        case col_int8:
        case col_uint16:
        case col_int16:
        case col_int32:
            {
                fprintf(hpp_file, "        if (value)\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = (%s)strtol(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                fprintf(hpp_file, "        }\r\n");
                fprintf(hpp_file, "        else\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
            }
            break;
        case col_uint32:
            {
                fprintf(hpp_file, "        if (value)\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = (%s)strtoul(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                fprintf(hpp_file, "        }\r\n");
                fprintf(hpp_file, "        else\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
            }
            break;
        case col_uint64:
            {
                fprintf(hpp_file, "        if (value)\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = (%s)_strtoui64(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                fprintf(hpp_file, "        }\r\n");
                fprintf(hpp_file, "        else\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
            }
            break;
        case col_int64:
            {
                fprintf(hpp_file, "        if (value)\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = (%s)_strtoi64(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                fprintf(hpp_file, "        }\r\n");
                fprintf(hpp_file, "        else\r\n");
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            row->%s = 0;\r\n", info.m_col_name.c_str());
                fprintf(hpp_file, "        }\r\n");
            }
            break;
        }
    }
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_Get( FILE* hpp_file )
{
    if (m_table_key.empty())
    {
        return;
    }

    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        key_info& k_info = it->second;

        //if (key_count == 1)
        //{
        //    fprintf(hpp_file, "    %s* Get(", m_struct_name.c_str());
        //}
        //else
        //{
        //    fprintf(hpp_file, "    %s* Get%u(", m_struct_name.c_str(), key_count-1);
        //}
        fprintf(hpp_file, "    %s* GetBy", m_struct_name.c_str());
        size_t param_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++param_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;
            fprintf(hpp_file, "%s", info.m_col_name.c_str());

            if (param_idx == k_info.m_key_order.size())
            {
                fprintf(hpp_file, "(");
            }
        }
        param_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++param_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            if (param_idx == k_info.m_key_order.size())
            {
                if (info.m_col_type == "char*")
                    fprintf(hpp_file, "const char* %s)\r\n", info.m_col_name.c_str());
                else
                    fprintf(hpp_file, "%s %s)\r\n", info.m_col_type.c_str(), info.m_col_name.c_str());
            }
            else
            {
                if (info.m_col_type == "char*")
                    fprintf(hpp_file, "const char* %s, ", info.m_col_name.c_str());
                else
                    fprintf(hpp_file, "%s %s, ", info.m_col_type.c_str(), info.m_col_name.c_str());
            }
        }

        fprintf(hpp_file, "    {\r\n");

        std::string space_str = "        ";
        size_t key_idx = 1;
        std::list<column_info*> key_list;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++key_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            if (key_idx < k_info.m_key_order.size())
            {
                fprintf(hpp_file, space_str.c_str());
                switch (info.m_data_type)
                {
                case col_int8:
                case col_uint8:
                case col_int16:
                case col_uint16:
                case col_int32:
                case col_uint32:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)quick_tree_find(m_data_map%u, %s);\r\n",key_idx, key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)quick_tree_find(tree%u, %s);\r\n",key_idx, key_idx-1, info.m_col_name.c_str());
                        }
                    }
                    break;
                case col_uint64:
                case col_int64:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)tree_find_int64(m_data_map%u, %s);\r\n",key_idx, key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)tree_find_int64(tree%u, %s);\r\n",key_idx, key_idx-1, info.m_col_name.c_str());
                        }
                    }
                    break;
                default:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)tree_find_str(m_data_map%u, %s);\r\n",key_idx, key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "HRBTREE tree%u = (HRBTREE)tree_find_str(tree%u, %s);\r\n",key_idx, key_idx-1, info.m_col_name.c_str());
                        }
                    }
                }

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "if (tree%u)\r\n", key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"{\r\n");
                space_str.append("    ");

                key_list.push_front(&info);
            }
            else
            {
                fprintf(hpp_file, space_str.c_str());
                switch (info.m_data_type)
                {
                case col_int8:
                case col_uint8:
                case col_int16:
                case col_uint16:
                case col_int32:
                case col_uint32:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "return (%s*)quick_tree_find(m_data_map%u, %s);\r\n", 
                                m_struct_name.c_str(), key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "return (%s*)quick_tree_find(tree%u, %s);\r\n", 
                                m_struct_name.c_str(), key_idx-1, info.m_col_name.c_str());
                        }
                    }
                    break;
                case col_uint64:
                case col_int64:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "return (%s*)tree_find_int64(m_data_map%u, %s);\r\n", 
                                m_struct_name.c_str(), key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "return (%s*)tree_find_int64(tree%u, %s);\r\n", 
                                m_struct_name.c_str(), key_idx-1, info.m_col_name.c_str());
                        }
                    }
                    break;
                default:
                    {
                        if (key_idx == 1)
                        {
                            fprintf(hpp_file, "return (%s*)tree_find_str(m_data_map%u, %s);\r\n", 
                                m_struct_name.c_str(), key_count, info.m_col_name.c_str());
                        }
                        else
                        {
                            fprintf(hpp_file, "return (%s*)tree_find_str(tree%u, %s);\r\n", 
                                m_struct_name.c_str(), key_idx-1, info.m_col_name.c_str());
                        }
                    }
                }
            }
        }

        while (!key_list.empty())
        {
            space_str = space_str.substr(0, space_str.length()-4);
            fprintf(hpp_file, space_str.c_str());
            fprintf(hpp_file, "}\r\n");
            key_list.pop_front();
        }

        if (k_info.m_key_order.size() > 1)
        {
            fprintf(hpp_file, "        return 0;\r\n");
        }
        
        fprintf(hpp_file, "    }\r\n");
    }
}

void CTableMaker::_print_func_ReleaseMapping( FILE* hpp_file )
{
    if (m_table_key.empty())
    {
        return;
    }

    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        key_info& k_info = it->second;

        fprintf(hpp_file, "    void ReleaseMapping%u(void)\r\n", key_count);
        fprintf(hpp_file, "    {\r\n");
        fprintf(hpp_file, "        if (!m_data_map%u)\r\n", key_count);
        fprintf(hpp_file, "        {\r\n");
        fprintf(hpp_file, "            return;\r\n");
        fprintf(hpp_file, "        }\r\n");

        std::string space_str = "        ";
        size_t key_idx = 1;
        std::list<column_info*> key_list;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++key_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            if (key_idx < k_info.m_key_order.size())
            {
                fprintf(hpp_file, space_str.c_str());

                if (key_idx == 1)
                {
                    fprintf(hpp_file, "HRBNODE node_%u = rb_first(m_data_map%u);\r\n", key_idx, key_count);
                }
                else
                {
                    fprintf(hpp_file, "HRBNODE node_%u = rb_first((HRBTREE)value_arry%u[i%u]);\r\n", key_idx, key_idx-1, key_idx-1);
                }

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "while (node_%u)\r\n", key_idx);

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                space_str.append("    ");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "void** value_arry%u;\r\n", key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "size_t value_arry_size%u;\r\n", key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "void* tmp%u[1];\r\n", key_idx);

                fprintf(hpp_file, space_str.c_str());

                if (key_idx == 1)
                {
                    fprintf(hpp_file, "if (tree_is_quick(m_data_map%u))\r\n", key_count);
                }
                else
                {
                    fprintf(hpp_file, "if (tree_is_quick((HRBTREE)value_arry%u[i%u]))\r\n", key_idx-1, key_idx-1);
                }
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    value_arry_size%u = quick_tree_node_value(node_%u, &value_arry%u);\r\n", key_idx, key_idx, key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"}\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"else\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    value_arry_size%u = 1;\r\n", key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    tmp%u[0] = rb_node_value(node_%u);\r\n", key_idx, key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    value_arry%u = tmp%u;\r\n", key_idx, key_idx);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file,"}\r\n");

                //switch (info.m_data_type)
                //{
                //case col_int8:
                //case col_uint8:
                //case col_int16:
                //case col_uint16:
                //case col_int32:
                //case col_uint32:
                //    {
                //        fprintf(hpp_file, "value_arry_size%u = quick_tree_node_value(node_%u, &value_arry%u);\r\n", key_idx, key_idx, key_idx);
                //    }
                //	break;
                //default:
                //    {
                //        fprintf(hpp_file, "value_arry_size%u = 1;\r\n", key_idx);
                //        fprintf(hpp_file, space_str.c_str());
                //        fprintf(hpp_file, "void* tmp%u[1];\r\n", key_idx);
                //        fprintf(hpp_file, space_str.c_str());
                //        fprintf(hpp_file, "tmp%u[0] = rb_node_value(node_%u);\r\n", key_idx, key_idx);
                //        fprintf(hpp_file, space_str.c_str());
                //        fprintf(hpp_file, "value_arry%u = tmp%u;\r\n", key_idx, key_idx);

                //    }
                //}

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "for (size_t i%u = 0; i%u < value_arry_size%u; i%u++)\r\n", key_idx, key_idx, key_idx, key_idx);

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                space_str.append("    ");

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "if (value_arry%u[i%u])\r\n", key_idx, key_idx);

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                space_str.append("    ");
            }

            key_list.push_front(&info);
        }

        while (!key_list.empty())
        {
            column_info* col_info = key_list.front();

            fprintf(hpp_file, space_str.c_str());

            if (key_list.size() == 1)
            {
                fprintf(hpp_file, "if (tree_is_quick(m_data_map%u))\r\n", key_count);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    destroy_quick_tree(m_data_map%u);\r\n", key_count);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "else\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    destroy_rb_tree(m_data_map%u);\r\n", key_count);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");
                //switch (col_info->m_data_type)
                //{
                //case col_int8:
                //case col_uint8:
                //case col_int16:
                //case col_uint16:
                //case col_int32:
                //case col_uint32:
                //    {
                //        fprintf(hpp_file, "destroy_quick_tree(m_data_map%u);\r\n", key_count);
                //    }
                //	break;
                //default:
                //    {
                //        fprintf(hpp_file, "destroy_rb_tree(m_data_map%u);\r\n", key_count);
                //    }
                //}

                fprintf(hpp_file, "        m_data_map%u = 0;\r\n", key_count);
            }
            else
            {
                fprintf(hpp_file, "if (tree_is_quick((HRBTREE)value_arry%u[i%u]))\r\n", key_list.size()-1, key_list.size()-1);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    destroy_quick_tree((HRBTREE)value_arry%u[i%u]);\r\n", key_list.size()-1, key_list.size()-1);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "else\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "{\r\n");
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "    destroy_rb_tree((HRBTREE)value_arry%u[i%u]);\r\n", key_list.size()-1, key_list.size()-1);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");
                //switch (col_info->m_data_type)
                //{
                //case col_int8:
                //case col_uint8:
                //case col_int16:
                //case col_uint16:
                //case col_int32:
                //case col_uint32:
                //    {
                //        fprintf(hpp_file, "destroy_quick_tree((HRBTREE)value_arry%u[i%u]);\r\n", key_list.size()-1, key_list.size()-1);
                //    }
                //    break;
                //default:
                //    {
                //        fprintf(hpp_file, "destroy_rb_tree((HRBTREE)value_arry%u[i%u]);\r\n", key_list.size()-1, key_list.size()-1);
                //    }
                //}

                space_str = space_str.substr(0, space_str.length()-4);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");

                space_str = space_str.substr(0, space_str.length()-4);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");

                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "node_%u = rb_next(node_%u);\r\n", key_list.size()-1, key_list.size()-1);

                space_str = space_str.substr(0, space_str.length()-4);
                fprintf(hpp_file, space_str.c_str());
                fprintf(hpp_file, "}\r\n");
            }

            key_list.pop_front();
        }

        fprintf(hpp_file, "    }\r\n");
    }
}

void CTableMaker::_print_func_QuickMapping( FILE* hpp_file )
{
    if (m_table_key.empty())
    {
        return;
    }

    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        key_info& k_info = it->second;

        fprintf(hpp_file, "    void QuickMapping%u(HRBTREE& tree)\r\n", key_count);
        fprintf(hpp_file, "    {\r\n");
        fprintf(hpp_file, "        tree;\r\n");

        size_t key_idx = 1;
        std::list<column_info*> key_list;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++key_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            if (key_idx < k_info.m_key_order.size())
            {
                for (size_t i = 0; i < key_idx; i++)
                {
                    fprintf(hpp_file, "    ");
                }
                if (key_idx == 1)
                {
                    fprintf(hpp_file, "    HRBNODE node_%u = rb_first(tree);\r\n", key_idx);
                }
                else
                {
                    fprintf(hpp_file, "    HRBNODE node_%u = rb_first((HRBTREE)rb_node_value(node_%u));\r\n", key_idx, key_idx - 1);
                }

                for (size_t i = 0; i < key_idx; i++)
                {
                    fprintf(hpp_file, "    ");
                }
                fprintf(hpp_file, "    while(node_%u)\r\n", key_idx);
                for (size_t i = 0; i < key_idx; i++)
                {
                    fprintf(hpp_file, "    ");
                }
                fprintf(hpp_file, "    {\r\n", key_idx);
            }

            key_list.push_front(&info);
        }

        while (!key_list.empty())
        {
            column_info* col_info = key_list.front();

            if (key_list.size() == 1)
            {
                switch (col_info->m_data_type)
                {
                case col_int8:
                case col_uint8:
                case col_int16:
                case col_uint16:
                case col_int32:
                case col_uint32:
                    {
                        fprintf(hpp_file, "        tree = quick_tree(tree, 2048);\r\n");
                    }
                	break;
                }
            }
            else
            {
                switch (col_info->m_data_type)
                {
                case col_int8:
                case col_uint8:
                case col_int16:
                case col_uint16:
                case col_int32:
                case col_uint32:
                    {
                        for (size_t i = 0; i < key_list.size(); i++)
                        {
                            fprintf(hpp_file, "    ");
                        }

                        fprintf(hpp_file, "    HRBTREE new_tree_%u = quick_tree((HRBTREE)rb_node_value(node_%u), 2048);\r\n", key_list.size(), key_list.size()-1);

                        for (size_t i = 0; i < key_list.size(); i++)
                        {
                            fprintf(hpp_file, "    ");
                        }

                        fprintf(hpp_file, "    rb_node_set_value(node_%u, new_tree_%u);\r\n", key_list.size()-1, key_list.size());
                    }
                    break;
                }

                for (size_t i = 0; i < key_list.size(); i++)
                {
                    fprintf(hpp_file, "    ");
                }

                fprintf(hpp_file, "    node_%u = rb_next(node_%u);\r\n", key_list.size()-1, key_list.size()-1);

                for (size_t i = 0; i < key_list.size()-1; i++)
                {
                    fprintf(hpp_file, "    ");
                }

                fprintf(hpp_file, "    }\r\n");
            }

            key_list.pop_front();
        }

        fprintf(hpp_file, "    }\r\n");
    }
}

void CTableMaker::_print_func_FillMapping( FILE* hpp_file )
{
    if (m_table_key.empty())
    {
        return;
    }



    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        key_info& k_info = it->second;
        

        fprintf(hpp_file, "    bool FillMapping%u(%s* row, HRBTREE tree, char* err, size_t err_len)\r\n", key_count, m_struct_name.c_str());
        fprintf(hpp_file, "    {\r\n");
        fprintf(hpp_file, "        HRBNODE exist_node = 0;\r\n");
        fprintf(hpp_file, "        HRBTREE key_1_map = tree;\r\n");

        size_t key_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++key_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            switch (info.m_data_type)
            {
            case col_string:
                {
                    fprintf(hpp_file, "        exist_node = 0;\r\n");
                    if (it_key_next == k_info.m_key_order.end())
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_str(key_%u_map, row->%s, row, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                    else
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_str(key_%u_map, row->%s, 0, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                    
                }
                break;
            case col_uint8:
            case col_int8:
            case col_uint16:
            case col_int16:
            case col_uint32:
            case col_int32:
                {
                    fprintf(hpp_file, "        exist_node = 0;\r\n");
                    if (it_key_next == k_info.m_key_order.end())
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_int(key_%u_map, row->%s, row, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                    else
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_int(key_%u_map, row->%s, 0, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                }
                break;
            case col_uint64:
            case col_int64:
                {
                    fprintf(hpp_file, "        exist_node = 0;\r\n");
                    if (it_key_next == k_info.m_key_order.end())
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_int64(key_%u_map, row->%s, row, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                    else
                    {
                        fprintf(hpp_file, "        rb_tree_try_insert_int64(key_%u_map, row->%s, 0, &exist_node);\r\n", key_idx, info.m_col_name.c_str());
                    }
                }
                break;
            }

            if (it_key_next == k_info.m_key_order.end())
            {
                fprintf(hpp_file, "        %s* exist_row = (%s*)rb_node_value(exist_node);\r\n", m_struct_name.c_str(), m_struct_name.c_str());
                fprintf(hpp_file, "        if (exist_row != row)\r\n");
                fprintf(hpp_file, "        {\r\n");
                
                switch (info.m_data_type)
                {
                case col_string:
                    {
                        fprintf(hpp_file, "            sprintf_s(err, err_len, \"key %s=%%s repeat\", row->%s);\r\n", info.m_col_name.c_str(), info.m_col_name.c_str());
                    }
                    break;
                case col_uint8:
                case col_int8:
                case col_uint16:
                case col_int16:
                case col_int32:
                    {
                        fprintf(hpp_file, "            sprintf_s(err, err_len, \"key %s=%%d repeat\", row->%s);\r\n", info.m_col_name.c_str(), info.m_col_name.c_str());
                    }
                    break;
                case col_uint32:
                    {
                        fprintf(hpp_file, "            sprintf_s(err, err_len, \"key %s=%%u repeat\", row->%s);\r\n", info.m_col_name.c_str(), info.m_col_name.c_str());
                    }
                    break;
                case col_uint64:
                    {
                        fprintf(hpp_file, "            sprintf_s(err, err_len, \"key %s=%%llu repeat\", row->%s);\r\n", info.m_col_name.c_str(), info.m_col_name.c_str());
                    }
                    break;
                case col_int64:
                    {
                        fprintf(hpp_file, "            sprintf_s(err, err_len, \"key %s=%%lld repeat\", row->%s);\r\n", info.m_col_name.c_str(), info.m_col_name.c_str());
                    }
                    break;
                }

                //fprintf(hpp_file, "            FreeRow(exist_row);\r\n");
                //fprintf(hpp_file, "            rb_node_set_value(exist_node, row);\r\n");
                fprintf(hpp_file, "            return false;\r\n");
                fprintf(hpp_file, "        }\r\n");
            }
            else
            {
                fprintf(hpp_file, "        HRBTREE key_%u_map = (HRBTREE)rb_node_value(exist_node);\r\n", key_idx+1);
                fprintf(hpp_file, "        if (!key_%u_map)\r\n", key_idx+1);
                fprintf(hpp_file, "        {\r\n");
                fprintf(hpp_file, "            key_%u_map = create_rb_tree(0, 0, 0);\r\n", key_idx+1);
                fprintf(hpp_file, "            rb_node_set_value(exist_node, key_%u_map);\r\n", key_idx+1);
                fprintf(hpp_file, "        }\r\n");
            }
        }

        fprintf(hpp_file, "        return true;\r\n");
        fprintf(hpp_file, "    }\r\n");
    }
}

void CTableMaker::_print_func_AllocRow( FILE* hpp_file )
{
    fprintf(hpp_file, "    %s* AllocRow(void)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        %s* row = S_NEW(%s, 1);\r\n", m_struct_name.c_str(), m_struct_name.c_str());

    for (table_column_info::iterator it = m_table_column.begin();
        it != m_table_column.end(); ++it)
    {
        column_info& info = it->second;

        if (info.m_data_type == col_string)
        {
            fprintf(hpp_file, "        row->%s = 0;\r\n", info.m_col_name.c_str());
        }
    }
    fprintf(hpp_file, "        return row;\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_FreeRow( FILE* hpp_file )
{
    fprintf(hpp_file, "    void FreeRow(%s* row)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");

    for (table_column_info::iterator it = m_table_column.begin();
        it != m_table_column.end(); ++it)
    {
        column_info& info = it->second;

        if (info.m_data_type == col_string)
        {
            fprintf(hpp_file, "        if (row->%s)\r\n", info.m_col_name.c_str());
            fprintf(hpp_file, "        {\r\n");
            fprintf(hpp_file, "            S_DELETE(row->%s);\r\n", info.m_col_name.c_str());
            fprintf(hpp_file, "        }\r\n");
        }
    }
    fprintf(hpp_file, "        S_DELETE(row);\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_Release( FILE* hpp_file )
{
    fprintf(hpp_file, "    void Release(void)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");

    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        ReleaseMapping%u();\r\n", key_count);
    }

    fprintf(hpp_file,"        for (size_t i = 0; i < m_data_arry_size; i++)\r\n");
    fprintf(hpp_file,"        {\r\n");
    fprintf(hpp_file,"            FreeRow(m_data_arry[i]);\r\n");
    fprintf(hpp_file,"        }\r\n");
    fprintf(hpp_file,"        if (m_data_arry)\r\n");
    fprintf(hpp_file,"        {\r\n");
    fprintf(hpp_file,"            S_DELETE(m_data_arry);\r\n");  
    fprintf(hpp_file,"            m_data_arry = 0;\r\n");
    fprintf(hpp_file,"            m_data_arry_size = 0;\r\n");
    fprintf(hpp_file,"        }\r\n");

    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_Load( FILE* hpp_file )
{
    fprintf(hpp_file, "    bool Load(const char* path, char* err, size_t err_len)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        Release();\r\n");
    fprintf(hpp_file, "        TiXmlDocument doc;\r\n");
    fprintf(hpp_file, "        if (!doc.LoadFile(path))\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            sprintf_s(err, err_len, \"%%s : %%s\", path, doc.ErrorDesc());\r\n");
    fprintf(hpp_file, "            return false;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        TiXmlElement* root = doc.FirstChildElement(\"root\");\r\n");
    fprintf(hpp_file, "        if (root == 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            sprintf_s(err, err_len, \"root is null\");\r\n");
    fprintf(hpp_file, "            return false;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        size_t data_count = 0;\r\n");
    fprintf(hpp_file, "        TiXmlElement* element = root->FirstChildElement(\"content\");\r\n");
    fprintf(hpp_file, "        while (element != 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            data_count++;\r\n");
    fprintf(hpp_file, "            element = element->NextSiblingElement();\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        m_data_arry = S_NEW(%s*, data_count);\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "        m_data_arry_size = 0;\r\n");
    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        m_data_map%u = create_rb_tree(0, 0, 0);\r\n", key_count);
    }
    fprintf(hpp_file, "        element = root->FirstChildElement(\"content\");\r\n");
    fprintf(hpp_file, "        while (element != 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            %s* row = AllocRow();\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "            FillData(row, element);\r\n");
    fprintf(hpp_file, "            m_data_arry[m_data_arry_size] = row;\r\n");
    fprintf(hpp_file, "            ++m_data_arry_size;\r\n");

    key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "            if (!FillMapping%u(row, m_data_map%u, err, err_len))\r\n", key_count, key_count);
        fprintf(hpp_file, "            {\r\n");
        fprintf(hpp_file, "                return false;\r\n");
        fprintf(hpp_file, "            }\r\n");
    }

    fprintf(hpp_file, "            element = element->NextSiblingElement();\r\n");
    fprintf(hpp_file, "        }\r\n");
    key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        QuickMapping%u(m_data_map%u);\r\n", key_count, key_count);
    }
    fprintf(hpp_file, "        return true;\r\n");

    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_ReLoadEx( FILE* hpp_file )
{
    fprintf(hpp_file, "    bool ReLoadEx(TiXmlDocument& doc, char* err, size_t err_len)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        TiXmlElement* root = doc.FirstChildElement(\"root\");\r\n");
    fprintf(hpp_file, "        if (root == 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            sprintf_s(err, err_len, \"root is null\");\r\n");
    fprintf(hpp_file, "            return false;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        size_t data_count = 0;\r\n");
    fprintf(hpp_file, "        TiXmlElement* element = root->FirstChildElement(\"content\");\r\n");
    fprintf(hpp_file, "        while (element != 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            data_count++;\r\n");
    fprintf(hpp_file, "            element = element->NextSiblingElement();\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        if (m_data_arry)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            S_DELETE(m_data_arry);\r\n");
    fprintf(hpp_file, "            m_data_arry = 0;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        m_data_arry = S_NEW(%s*, data_count);\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "        m_data_arry_size = 0;\r\n");
    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        HRBTREE tmp_data_map%u = create_rb_tree(0, 0, 0);\r\n", key_count);
    }

    fprintf(hpp_file, "        element = root->FirstChildElement(\"content\");\r\n");
    fprintf(hpp_file, "        while (element != 0)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            const char* value = 0;\r\n");

    key_count = 1;
    //for (table_key_info::iterator it = m_table_key.begin();
    //    it != m_table_key.end(); ++it, ++key_count)
    {
        std::vector<std::string> del_key_string;

        key_info& k_info = m_table_key.begin()->second;
        size_t param_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++param_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            fprintf(hpp_file, "            %s key_%s;\r\n", info.m_col_type.c_str(), info.m_col_name.c_str());

            fprintf(hpp_file, "            value = element->Attribute(\"%s\");\r\n", info.m_col_name.c_str());

            switch (info.m_data_type)
            {
            case col_string:
                {
                    fprintf(hpp_file, "            if (value)\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                size_t str_len = strlen(value);\r\n");
                    fprintf(hpp_file, "                key_%s = S_NEW(char, str_len + 1);\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "                memcpy(key_%s, value, str_len);\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "                (key_%s)[str_len] = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");
                    fprintf(hpp_file, "            else\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");

                    std::string del_string = "S_DELETE(key_";
                    del_string += info.m_col_name;
                    del_string += ");\r\n";

                    del_key_string.push_back(del_string);
                }
                break;
            case col_uint8:
            case col_int8:
            case col_uint16:
            case col_int16:
            case col_int32:
                {
                    fprintf(hpp_file, "            if (value)\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = (%s)strtol(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                    fprintf(hpp_file, "            }\r\n");
                    fprintf(hpp_file, "            else\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");
                }
                break;
            case col_uint32:
                {
                    fprintf(hpp_file, "            if (value)\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = (%s)strtoul(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                    fprintf(hpp_file, "            }\r\n");
                    fprintf(hpp_file, "            else\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");
                }
                break;
            case col_uint64:
                {
                    fprintf(hpp_file, "            if (value)\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = (%s)_strtoui64(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                    fprintf(hpp_file, "            }\r\n");
                    fprintf(hpp_file, "            else\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");
                }
                break;
            case col_int64:
                {
                    fprintf(hpp_file, "            if (value)\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = (%s)_strtoi64(value, 0, 10);\r\n", info.m_col_name.c_str(), info.m_col_type.c_str());
                    fprintf(hpp_file, "            }\r\n");
                    fprintf(hpp_file, "            else\r\n");
                    fprintf(hpp_file, "            {\r\n");
                    fprintf(hpp_file, "                key_%s = 0;\r\n", info.m_col_name.c_str());
                    fprintf(hpp_file, "            }\r\n");
                }
                break;
            }
        }

        fprintf(hpp_file, "            %s* row = 0;\r\n", m_struct_name.c_str());
        fprintf(hpp_file, "            if (m_data_map1)\r\n");
        fprintf(hpp_file, "            {\r\n");
        fprintf(hpp_file, "                row = GetBy", m_struct_name.c_str());
        param_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++param_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            fprintf(hpp_file, "%s", info.m_col_name.c_str());

            if (param_idx == k_info.m_key_order.size())
            {
                fprintf(hpp_file, "(");
            }
        }
        param_idx = 1;
        for (std::list<std::string>::iterator it_key = k_info.m_key_order.begin();
            it_key != k_info.m_key_order.end(); ++it_key, ++param_idx)
        {
            table_column_info::iterator it_col = m_table_column.find(*it_key);
            std::list<std::string>::iterator it_key_next = it_key;
            ++it_key_next;
            column_info& info = it_col->second;

            if (param_idx == k_info.m_key_order.size())
            {
                fprintf(hpp_file, "key_%s);\r\n", info.m_col_name.c_str());
            }
            else
            {
                fprintf(hpp_file, "key_%s, ", info.m_col_name.c_str());
            }
        }
        fprintf(hpp_file, "            }\r\n");
        fprintf(hpp_file, "            if (!row)\r\n");
        fprintf(hpp_file, "            {\r\n");
        fprintf(hpp_file, "                row = AllocRow();\r\n");
        fprintf(hpp_file, "            }\r\n");
        fprintf(hpp_file, "            FillData(row, element);\r\n");
        fprintf(hpp_file, "            m_data_arry[m_data_arry_size] = row;\r\n");
        fprintf(hpp_file, "            ++m_data_arry_size;\r\n");
        key_count = 1;
        for (table_key_info::iterator it = m_table_key.begin();
            it != m_table_key.end(); ++it, ++key_count)
        {
            fprintf(hpp_file, "            if (!FillMapping%u(row, tmp_data_map%u, err, err_len))\r\n", key_count, key_count);
            fprintf(hpp_file, "            {\r\n");
            fprintf(hpp_file, "                return false;\r\n");
            fprintf(hpp_file, "            }\r\n");
        }


        fprintf(hpp_file, "            element = element->NextSiblingElement();\r\n");

        for (size_t i = 0; i < del_key_string.size(); i++)
        {
            fprintf(hpp_file, "            %s", del_key_string[i].c_str());
        }
    }

    fprintf(hpp_file, "        }\r\n");

    key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        QuickMapping%u(tmp_data_map%u);\r\n", key_count, key_count);
        fprintf(hpp_file, "        ReleaseMapping%u();\r\n", key_count);
        fprintf(hpp_file, "        m_data_map%u = tmp_data_map%u;\r\n", key_count, key_count);
    }

    fprintf(hpp_file, "        return true;\r\n");

    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_ReLoad( FILE* hpp_file )
{
    fprintf(hpp_file, "    bool ReLoad(const char* path, char* err, size_t err_len)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        TiXmlDocument doc;\r\n");
    fprintf(hpp_file, "        if (!doc.LoadFile(path))\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            sprintf_s(err, err_len, \"%%s : %%s\", path, doc.ErrorDesc());\r\n");
    fprintf(hpp_file, "            return false;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        return ReLoadEx(doc, err, err_len);\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_Construct_Destruct( FILE* hpp_file )
{
    fprintf(hpp_file, "    %s(void)\r\n", m_class_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    size_t key_count = 1;
    for (table_key_info::iterator it = m_table_key.begin();
        it != m_table_key.end(); ++it, ++key_count)
    {
        fprintf(hpp_file, "        m_data_map%u = 0;\r\n", key_count);
    }
    
    fprintf(hpp_file, "        m_data_arry = 0;\r\n");
    fprintf(hpp_file, "        m_data_arry_size = 0;\r\n");
    fprintf(hpp_file, "        InitColInfo();\r\n");
    fprintf(hpp_file, "    }\r\n");

    fprintf(hpp_file, "    ~%s(void)\r\n", m_class_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        UnInitColInfo();\r\n");
    fprintf(hpp_file, "        Release();\r\n");
    fprintf(hpp_file, "    }\r\n");
}

void CTableMaker::_print_func_InitColInfo_UnInitColInfo_GetColVar( FILE* hpp_file )
{
    fprintf(hpp_file, "    void InitColInfo(void)\r\n");
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        m_col_info_map = create_rb_tree(0, 0, 0);\r\n");

    for (table_column_info::iterator it = m_table_column.begin();
        it != m_table_column.end(); ++it)
    {
        column_info& info = it->second;

        std::string type;

        switch (info.m_data_type)
        {
        case col_string:
            type = "col_string";
            break;
        case col_uint8:
            type = "col_uint8";
            break;
        case col_int8:
            type = "col_int8";
            break;
        case col_uint16:
            type = "col_uint16";
            break;
        case col_int16:
            type = "col_int16";
            break;
        case col_uint32:
            type = "col_uint32";
            break;
        case col_int32:
            type = "col_int32";
            break;
        case col_uint64:
            type = "col_uint64";
            break;
        case col_int64:
            type = "col_int64";
            break;
        }

        fprintf(hpp_file, "        add_col_info(m_col_info_map, \"%s\", offsetof(%s, %s::%s), %s);\r\n",
            info.m_col_name.c_str(), m_struct_name.c_str(), m_struct_name.c_str(), info.m_col_name.c_str(), type.c_str());
    }



    fprintf(hpp_file, "    }\r\n");



    fprintf(hpp_file, "    void UnInitColInfo(void)\r\n");
    fprintf(hpp_file, "    {\r\n");

    for (table_column_info::iterator it = m_table_column.begin();
        it != m_table_column.end(); ++it)
    {
        column_info& info = it->second;

        fprintf(hpp_file, "        del_col_info(m_col_info_map, \"%s\");\r\n", info.m_col_name.c_str());
    }


    fprintf(hpp_file, "        destroy_rb_tree(m_col_info_map);\r\n");
    fprintf(hpp_file, "    }\r\n");


    fprintf(hpp_file, "    col_var GetColVar(%s* row, const char* col_name)\r\n", m_struct_name.c_str());
    fprintf(hpp_file, "    {\r\n");
    fprintf(hpp_file, "        col_var var;\r\n");
    fprintf(hpp_file, "        col_var_info* info = get_col_info(m_col_info_map, col_name);\r\n");
    fprintf(hpp_file, "        if (info)\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            var.col_var_ptr = (char*)(row)+info->col_var_offset;\r\n");
    fprintf(hpp_file, "            var.col_var_type = info->col_var_type;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        else\r\n");
    fprintf(hpp_file, "        {\r\n");
    fprintf(hpp_file, "            var.col_var_ptr = 0;\r\n");
    fprintf(hpp_file, "            var.col_var_type = col_null;\r\n");
    fprintf(hpp_file, "        }\r\n");
    fprintf(hpp_file, "        return var;\r\n");
    fprintf(hpp_file, "    }\r\n");
}








