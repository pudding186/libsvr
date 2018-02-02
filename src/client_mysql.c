#include "../include/type_def.h"
#include "../include/client_mysql.h"
#include "../include/memory_pool.h"
#include <stdio.h>


////////////////////////////////////////////////////////////////////////

typedef struct st_client_mysql_result
{
    MYSQL*          cur_mysql;
    MYSQL_RES*      record_set;
    my_ulonglong    affect_row;
}client_mysql_result;

typedef struct st_client_mysql
{
    MYSQL*              real_mysql;
    client_mysql_result result;
}client_mysql;

HCLIENTMYSQL create_client_mysql(const char *host, unsigned int port,
    const char *user, const char *passwd, const char* db, const char* charact_set, 
    char* err_info, size_t err_info_size)
{
    unsigned long long character_set_num = 0;

    unsigned long long i = 0;

    my_bool reconnect = true;

    my_bool character_support = false;

    struct st_client_mysql_value value_data;

    char* character_client;
    char* character_connection;
    char* character_result;

    HCLIENTMYSQLRES mysql_res_ptr = 0;

    client_mysql* client_mysql_ptr = (client_mysql*)malloc(sizeof(client_mysql));
    
    client_mysql_ptr->result.cur_mysql = 0;
    client_mysql_ptr->result.record_set = 0;
    client_mysql_ptr->result.affect_row = 0;

    //MYSQL* mysql_ptr = mysql_init(0);
    client_mysql_ptr->real_mysql = mysql_init(0);
    

    //if (!mysql_ptr)
    if (!client_mysql_ptr->real_mysql)
    {
        //destroy_client_mysql(mysql_ptr);
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    //if (mysql_options(mysql_ptr, MYSQL_OPT_RECONNECT, &reconnect))
    if (mysql_options(client_mysql_ptr->real_mysql, MYSQL_OPT_RECONNECT, &reconnect))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size-1);
            memcpy(err_info, err, err_len+1);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (!mysql_real_connect(client_mysql_ptr->real_mysql, host, user,
        passwd, db, port, 0, CLIENT_MULTI_STATEMENTS))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size-1);
            memcpy(err_info, err, err_len+1);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (!charact_set)
    {
        charact_set = "latin1";
    }

    if (!client_mysql_query(client_mysql_ptr, "SHOW CHARACTER SET", (unsigned long)strlen("SHOW CHARACTER SET")))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size-1);
            memcpy(err_info, err, err_len+1);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    mysql_res_ptr = client_mysql_get_result(client_mysql_ptr);

    character_set_num = mysql_num_rows(mysql_res_ptr->record_set);

    while (i < character_set_num)
    {
        value_data = client_mysql_row_field_value(mysql_res_ptr, i, 0);

        if (!strcmp(charact_set, value_data.value))
        {
            character_support = true;
            break;
        }
        i++;
    }

    client_mysql_free_result(mysql_res_ptr);

    if (!character_support)
    {
        if (err_info)
        {
            sprintf_s(err_info, err_info_size, "character %s not support", charact_set);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (mysql_set_character_set(client_mysql_ptr->real_mysql, charact_set))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size-1);
            memcpy(err_info, err, err_len+1);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (!client_mysql_query(client_mysql_ptr,
        "select @@character_set_client, @@character_set_connection, @@character_set_results;",
        (unsigned long)strlen("select @@character_set_client, @@character_set_connection, @@character_set_results;")))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size-1);
            memcpy(err_info, err, err_len+1);
            err_info[err_info_size-1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    mysql_res_ptr = client_mysql_get_result(client_mysql_ptr);

    value_data = client_mysql_row_field_value(mysql_res_ptr, 0, 0);
    character_client = value_data.value;

    value_data = client_mysql_row_field_value(mysql_res_ptr, 0, 1);
    character_connection = value_data.value;

    value_data = client_mysql_row_field_value(mysql_res_ptr, 0, 2);
    character_result = value_data.value;

    if (!strcmp(character_client, character_connection))
    {
        if (!strcmp(character_connection, character_result))
        {
            client_mysql_free_result(mysql_res_ptr);
            return client_mysql_ptr;
        }
    }

    client_mysql_free_result(mysql_res_ptr);

    if (err_info)
    {
        sprintf_s(err_info, err_info_size, "character_client: %s character_connection: %s character_result: %s",
            character_client, character_connection, character_result);
    }

    destroy_client_mysql(client_mysql_ptr);

    return 0;
}

void destroy_client_mysql(HCLIENTMYSQL client_mysql_ptr)
{
    if (client_mysql_ptr->result.record_set)
    {
        mysql_free_result(client_mysql_ptr->result.record_set);
    }

    if (client_mysql_ptr->real_mysql)
    {
        mysql_close(client_mysql_ptr->real_mysql);
    }

    free(client_mysql_ptr);
}

bool client_mysql_query(HCLIENTMYSQL client_mysql_ptr, const char* sql, unsigned long length)
{
    int query_ret;
    int try_query_count = 0;

QUERY:

    if (length)
    {
        query_ret = mysql_real_query(client_mysql_ptr->real_mysql, sql, length);
    }
    else
    {
        query_ret = mysql_query(client_mysql_ptr->real_mysql, sql);
    }

    if (query_ret)
    {
        switch (mysql_errno(client_mysql_ptr->real_mysql))
        {
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_LOST:
            {
                if (try_query_count < 3)
                {
                    try_query_count++;
                    mysql_ping(client_mysql_ptr->real_mysql);
                    goto QUERY;
                }
                else
                {
                    return false;
                }
            }
            break;
        default:
            return false;
        }
    }

    if (client_mysql_ptr->result.record_set)
    {
        mysql_free_result(client_mysql_ptr->result.record_set);
    }

    client_mysql_ptr->result.cur_mysql = client_mysql_ptr->real_mysql;

    client_mysql_ptr->result.record_set = mysql_store_result(client_mysql_ptr->real_mysql);

    if (client_mysql_ptr->result.record_set)
    {
        client_mysql_ptr->result.affect_row = 0;
    }
    else
    {
        if (mysql_field_count(client_mysql_ptr->real_mysql) == 0)
        {
            client_mysql_ptr->result.affect_row = mysql_affected_rows(client_mysql_ptr->real_mysql);
        }
        else
        {
            return false;
        }
    }

    return true;
}

HCLIENTMYSQLRES client_mysql_get_result(HCLIENTMYSQL connection)
{
    return &connection->result;
}

HCLIENTMYSQLRES client_mysql_next_result(HCLIENTMYSQLRES last_result, unsigned int* client_mysql_errno)
{
    if (!last_result)
    {
        *client_mysql_errno = 0;
        return 0;
    }

    MYSQL* real_mysql = last_result->cur_mysql;

    if (last_result->record_set)
    {
        mysql_free_result(last_result->record_set);
        last_result->record_set = 0;
    }

    last_result->affect_row = 0;

    switch (mysql_next_result(real_mysql))
    {
    case 0:
    break;
    case -1:
    {
        *client_mysql_errno = 0;
        return 0;
    }
    break;
    default:
    {
        *client_mysql_errno = mysql_errno(real_mysql);
        return last_result;
    }
    }

    last_result->record_set = mysql_store_result(real_mysql);

    if (last_result->record_set)
    {
        last_result->affect_row = 0;
    }
    else
    {
        if (mysql_field_count(real_mysql) == 0)
        {
            last_result->affect_row = mysql_affected_rows(real_mysql);
        }
        else
        {
            *client_mysql_errno = CR_NO_RESULT_SET;
            return last_result;
        }
    }

    *client_mysql_errno = 0;
    return last_result;
}

void client_mysql_free_result(HCLIENTMYSQLRES result)
{
    if (result)
    {
        if (result->record_set)
        {
            mysql_free_result(result->record_set);
            result->record_set = 0;
        }

        result->affect_row = 0;
    }
}

bool client_mysql_result_record(HCLIENTMYSQLRES result)
{
    if (result->record_set)
    {
        return true;
    }

    return false;
}

unsigned long long client_mysql_result_affected(HCLIENTMYSQLRES result)
{
    return result->affect_row;
}



CLIENTMYSQLROW client_mysql_fetch_row(HCLIENTMYSQLRES result)
{
    CLIENTMYSQLROW row;

    row.row_values = mysql_fetch_row(result->record_set);
    if (row.row_values)
    {
        row.row_lengths = mysql_fetch_lengths(result->record_set);
    }
    else
    {
        row.row_lengths = 0;
    }

    return row;
}

CLIENTMYSQLROW client_mysql_row(HCLIENTMYSQLRES result, unsigned long long row_index)
{
    mysql_data_seek(result->record_set, row_index);

    return client_mysql_fetch_row(result);
}

CLIENTMYSQLVALUE client_mysql_value(CLIENTMYSQLROW row, unsigned long field_index)
{
    CLIENTMYSQLVALUE data;

    data.value = row.row_values[field_index];
    data.size = row.row_lengths[field_index];

    return data;
}

CLIENTMYSQLVALUE client_mysql_row_field_value(HCLIENTMYSQLRES result, unsigned long long row_index, unsigned long field_index)
{
    CLIENTMYSQLVALUE data;

    MYSQL_ROW row_values = 0;
    unsigned long* row_lengths = 0;

    mysql_data_seek(result->record_set, row_index);

    row_values = mysql_fetch_row(result->record_set);

    if (row_values)
    {
        row_lengths = mysql_fetch_lengths(result->record_set);

        data.value = row_values[field_index];
        data.size = row_lengths[field_index];
    }
    else
    {
        data.value = 0;
        data.size = 0;
    }

    return data;
}

unsigned long client_mysql_escape_string(HCLIENTMYSQL connection, char* src, unsigned long src_size, char* dst, unsigned long dst_size)
{
    if (dst_size < src_size*2+1)
    {
        return (unsigned long)-1;
    }
    return mysql_real_escape_string(connection->real_mysql, dst, src, src_size);
}

unsigned char client_mysql_value_uint8(CLIENTMYSQLVALUE data)
{
    unsigned char value_uint8;

    value_uint8 = (unsigned char)atoi(data.value);

    return value_uint8;
}

char client_mysql_value_int8(CLIENTMYSQLVALUE data)
{
    char value_int8;

    value_int8 = (char)atoi(data.value);

    return value_int8;
}

unsigned short client_mysql_value_uint16(CLIENTMYSQLVALUE data)
{
    unsigned short value_uint16;

    value_uint16 = (unsigned short)atoi(data.value);

    return value_uint16;
}

short client_mysql_value_int16(CLIENTMYSQLVALUE data)
{
    short value_int16;

    value_int16 = (short)atoi(data.value);

    return value_int16;
}

unsigned int client_mysql_value_uint32(CLIENTMYSQLVALUE data)
{
    unsigned int value_uint32;

    value_uint32 = (unsigned int)_strtoui64(data.value, 0, 10);

    return value_uint32;
}

int client_mysql_value_int32(CLIENTMYSQLVALUE data)
{
    int value_int32;

    value_int32 = atoi(data.value);

    return value_int32;
}

unsigned long long client_mysql_value_uint64(CLIENTMYSQLVALUE data)
{
    unsigned long long value_uint64;

    value_uint64 = _strtoui64(data.value, 0, 10);

    return value_uint64;
}

long long client_mysql_value_int64(CLIENTMYSQLVALUE data)
{
    long long value_int64;

    value_int64 = _strtoi64(data.value, 0, 10);

    return value_int64;
}

const char* client_mysql_err(HCLIENTMYSQL connection)
{
    return mysql_error(connection->real_mysql);
}

unsigned long long client_mysql_rows_num(HCLIENTMYSQLRES result)
{
    return mysql_num_rows(result->record_set);
}

unsigned int client_mysql_fields_num(HCLIENTMYSQLRES result)
{
    return mysql_num_fields(result->record_set);
}
