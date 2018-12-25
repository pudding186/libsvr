#include "../include/type_def.h"
#include "../include/client_mysql.h"
#include "../include/memory_pool.h"
#include <stdio.h>


////////////////////////////////////////////////////////////////////////

typedef struct st_client_mysql
{
    MYSQL*              real_mysql;
    
    char*               host;
    char*               user;
    char*               passwd;
    char*               db;
    char*               charact_set;
    unsigned int        port;
}client_mysql;

bool client_mysql_result_success(HCLIENTMYSQLRES result)
{
    if (result)
    {
        return result->error_code == 0;
    }

    return false;
}

HCLIENTMYSQL create_client_mysql(const char *host, unsigned int port,
    const char *user, const char *passwd, const char* db, const char* charact_set,
    char* err_info, size_t err_info_size)
{
    unsigned long long character_set_num = 0;

    unsigned long long i = 0;

    struct st_client_mysql_value value_data;

    my_bool character_support = false;

    char* character_client;
    char* character_connection;
    char* character_result;

    if (!charact_set)
    {
        charact_set = "latin1";
    }

    size_t host_length = strlen(host);
    size_t user_length = strlen(user);
    size_t passwd_length = strlen(passwd);
    size_t db_length = strlen(db);
    size_t charact_set_length = strlen(charact_set);
    size_t port_length = sizeof(port);

    CLIENTMYSQLRES mysql_result;

    char* ptr = (char*)malloc(
        sizeof(client_mysql)
        + host_length + 1
        + user_length + 1
        + passwd_length + 1
        + db_length + 1
        + charact_set_length + 1
        + port_length);

    client_mysql* client_mysql_ptr = (client_mysql*)ptr;
    ptr += sizeof(client_mysql);
    
    client_mysql_ptr->host = ptr;
    strcpy(ptr, host);
    ptr += host_length + 1;

    client_mysql_ptr->user = ptr;
    strcpy(ptr, user);
    ptr += user_length + 1;

    client_mysql_ptr->passwd = ptr;
    strcpy(ptr, passwd);
    ptr += passwd_length + 1;

    client_mysql_ptr->db = ptr;
    strcpy(ptr, db);
    ptr += db_length + 1;

    client_mysql_ptr->charact_set = ptr;
    strcpy(ptr, charact_set);
    ptr += charact_set_length + 1;

    client_mysql_ptr->port = port;

    mysql_result.cur_mysql = 0;
    mysql_result.record_set = 0;
    mysql_result.affect_row = 0;
    mysql_result.error_code = 0;

    client_mysql_ptr->real_mysql = mysql_init(0);


    if (!client_mysql_ptr->real_mysql)
    {
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (!mysql_real_connect(client_mysql_ptr->real_mysql, host, user,
        passwd, db, port, 0, CLIENT_MULTI_STATEMENTS))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size - 1);
            memcpy(err_info, err, err_len + 1);
            err_info[err_info_size - 1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    mysql_result = client_mysql_query(client_mysql_ptr, "SHOW CHARACTER SET", (unsigned long)strlen("SHOW CHARACTER SET"));

    if (!client_mysql_result_success(&mysql_result))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size - 1);
            memcpy(err_info, err, err_len + 1);
            err_info[err_info_size - 1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    character_set_num = mysql_num_rows(mysql_result.record_set);

    while (i < character_set_num)
    {
        value_data = client_mysql_row_field_value(&mysql_result, i, 0);

        if (!strcmp(charact_set, value_data.value))
        {
            character_support = true;
            break;
        }
        i++;
    }

    client_mysql_free_result(&mysql_result);

    if (!character_support)
    {
        if (err_info)
        {
            sprintf_s(err_info, err_info_size, "character %s not support", charact_set);
            err_info[err_info_size - 1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    if (mysql_set_character_set(client_mysql_ptr->real_mysql, charact_set))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size - 1);
            memcpy(err_info, err, err_len + 1);
            err_info[err_info_size - 1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    mysql_result = client_mysql_query(client_mysql_ptr,
        "select @@character_set_client, @@character_set_connection, @@character_set_results;",
        (unsigned long)strlen("select @@character_set_client, @@character_set_connection, @@character_set_results;"));

    if (!client_mysql_result_success(&mysql_result))
    {
        if (err_info)
        {
            const char* err = mysql_error(client_mysql_ptr->real_mysql);

            size_t err_len = strnlen(err, err_info_size - 1);
            memcpy(err_info, err, err_len + 1);
            err_info[err_info_size - 1] = '\0';
        }
        destroy_client_mysql(client_mysql_ptr);
        return 0;
    }

    value_data = client_mysql_row_field_value(&mysql_result, 0, 0);
    character_client = value_data.value;

    value_data = client_mysql_row_field_value(&mysql_result, 0, 1);
    character_connection = value_data.value;

    value_data = client_mysql_row_field_value(&mysql_result, 0, 2);
    character_result = value_data.value;

    if (!strcmp(character_client, character_connection))
    {
        if (!strcmp(character_connection, character_result))
        {
            client_mysql_free_result(&mysql_result);
            return client_mysql_ptr;
        }
    }

    client_mysql_free_result(&mysql_result);

    if (err_info)
    {
        sprintf_s(err_info, err_info_size, "character_client: %s character_connection: %s character_result: %s",
            character_client, character_connection, character_result);
    }

    destroy_client_mysql(client_mysql_ptr);

    return 0;
}

bool _re_connect(HCLIENTMYSQL client_mysql_ptr)
{
    unsigned long long character_set_num = 0;

    unsigned long long i = 0;

    my_bool character_support = false;

    struct st_client_mysql_value value_data;

    char* character_client;
    char* character_connection;
    char* character_result;

    CLIENTMYSQLRES mysql_result;

    if (client_mysql_ptr->real_mysql)
    {
        mysql_close(client_mysql_ptr->real_mysql);
    }

    client_mysql_ptr->real_mysql = mysql_init(0);


    if (!client_mysql_ptr->real_mysql)
    {
        return false;
    }

    if (!mysql_real_connect(client_mysql_ptr->real_mysql, 
        client_mysql_ptr->host, 
        client_mysql_ptr->user,
        client_mysql_ptr->passwd, 
        client_mysql_ptr->db, 
        client_mysql_ptr->port, 0, CLIENT_MULTI_STATEMENTS))
    {
        //if (err_info)
        //{
        //    const char* err = mysql_error(client_mysql_ptr->real_mysql);

        //    size_t err_len = strnlen(err, err_info_size - 1);
        //    memcpy(err_info, err, err_len + 1);
        //    err_info[err_info_size - 1] = '\0';
        //}
        //destroy_client_mysql(client_mysql_ptr);
        //return 0;
        mysql_close(client_mysql_ptr->real_mysql);
        client_mysql_ptr->real_mysql = 0;
        return false;
    }

    mysql_result = client_mysql_query(client_mysql_ptr, "SHOW CHARACTER SET", (unsigned long)strlen("SHOW CHARACTER SET"));

    if (!client_mysql_result_success(&mysql_result))
    {
        //if (err_info)
        //{
        //    const char* err = mysql_error(client_mysql_ptr->real_mysql);

        //    size_t err_len = strnlen(err, err_info_size - 1);
        //    memcpy(err_info, err, err_len + 1);
        //    err_info[err_info_size - 1] = '\0';
        //}
        //destroy_client_mysql(client_mysql_ptr);
        //return 0;
        mysql_close(client_mysql_ptr->real_mysql);
        client_mysql_ptr->real_mysql = 0;
        return false;
    }

    character_set_num = mysql_num_rows(mysql_result.record_set);

    while (i < character_set_num)
    {
        value_data = client_mysql_row_field_value(&mysql_result, i, 0);

        if (!strcmp(client_mysql_ptr->charact_set, value_data.value))
        {
            character_support = true;
            break;
        }
        i++;
    }

    client_mysql_free_result(&mysql_result);

    if (!character_support)
    {
        //if (err_info)
        //{
        //    sprintf_s(err_info, err_info_size, "character %s not support", charact_set);
        //    err_info[err_info_size - 1] = '\0';
        //}
        //destroy_client_mysql(client_mysql_ptr);
        //return 0;

        mysql_close(client_mysql_ptr->real_mysql);
        client_mysql_ptr->real_mysql = 0;
        return false;
    }

    if (mysql_set_character_set(client_mysql_ptr->real_mysql, client_mysql_ptr->charact_set))
    {
        //if (err_info)
        //{
        //    const char* err = mysql_error(client_mysql_ptr->real_mysql);

        //    size_t err_len = strnlen(err, err_info_size - 1);
        //    memcpy(err_info, err, err_len + 1);
        //    err_info[err_info_size - 1] = '\0';
        //}
        //destroy_client_mysql(client_mysql_ptr);
        //return 0;
        mysql_close(client_mysql_ptr->real_mysql);
        client_mysql_ptr->real_mysql = 0;
        return false;
    }

    mysql_result = client_mysql_query(client_mysql_ptr,
        "select @@character_set_client, @@character_set_connection, @@character_set_results;",
        (unsigned long)strlen("select @@character_set_client, @@character_set_connection, @@character_set_results;"));

    if (!client_mysql_result_success(&mysql_result))
    {
        //if (err_info)
        //{
        //    const char* err = mysql_error(client_mysql_ptr->real_mysql);

        //    size_t err_len = strnlen(err, err_info_size - 1);
        //    memcpy(err_info, err, err_len + 1);
        //    err_info[err_info_size - 1] = '\0';
        //}
        //destroy_client_mysql(client_mysql_ptr);
        //return 0;
        mysql_close(client_mysql_ptr->real_mysql);
        client_mysql_ptr->real_mysql = 0;
        return false;
    }

    value_data = client_mysql_row_field_value(&mysql_result, 0, 0);
    character_client = value_data.value;

    value_data = client_mysql_row_field_value(&mysql_result, 0, 1);
    character_connection = value_data.value;

    value_data = client_mysql_row_field_value(&mysql_result, 0, 2);
    character_result = value_data.value;

    if (!strcmp(character_client, character_connection))
    {
        if (!strcmp(character_connection, character_result))
        {
            client_mysql_free_result(&mysql_result);
            return true;
        }
    }

    client_mysql_free_result(&mysql_result);

    //if (err_info)
    //{
    //    sprintf_s(err_info, err_info_size, "character_client: %s character_connection: %s character_result: %s",
    //        character_client, character_connection, character_result);
    //}

    //destroy_client_mysql(client_mysql_ptr);

    //return 0;

    mysql_close(client_mysql_ptr->real_mysql);
    client_mysql_ptr->real_mysql = 0;
    return false;
}

void destroy_client_mysql(HCLIENTMYSQL client_mysql_ptr)
{
    if (client_mysql_ptr->real_mysql)
    {
        mysql_close(client_mysql_ptr->real_mysql);
    }

    free(client_mysql_ptr);
}

CLIENTMYSQLRES client_mysql_query(HCLIENTMYSQL client_mysql_ptr, const char* sql, unsigned long length)
{
    CLIENTMYSQLRES result;

    int query_ret;
    int try_query_count = 0;

    result.cur_mysql = 0;
    result.record_set = 0;
    result.affect_row = 0;
    result.error_code = 0;

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
        result.error_code = mysql_errno(client_mysql_ptr->real_mysql);
        switch (result.error_code)
        {
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_LOST:
        case CR_INVALID_CONN_HANDLE:
        case CR_SERVER_LOST_EXTENDED:
        case CR_CONN_HOST_ERROR:
        {
            if (try_query_count < 3)
            {
                try_query_count++;
                result.error_code = 0;
                //mysql_ping(client_mysql_ptr->real_mysql);
                _re_connect(client_mysql_ptr);
                goto QUERY;
            }
            else
            {
                return result;
            }
        }
        break;
        default:
            return result;
        }
    }

    result.cur_mysql = client_mysql_ptr->real_mysql;

    result.record_set = mysql_store_result(result.cur_mysql);

    if (result.record_set)
    {
        result.affect_row = 0;
    }
    else
    {
        if (mysql_field_count(result.cur_mysql) == 0)
        {
            result.affect_row = mysql_affected_rows(result.cur_mysql);
        }
        else
        {
            result.cur_mysql = 0;
        }
    }

    return result;
}

HCLIENTMYSQLRES client_mysql_next_result(HCLIENTMYSQLRES last_result)
{
    MYSQL* real_mysql = 0;

    if (!last_result)
    {
        return 0;
    }

    real_mysql = last_result->cur_mysql;

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
        last_result->error_code = 0;
        return 0;
    }
    break;
    default:
    {
        last_result->error_code = mysql_errno(real_mysql);
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
            last_result->error_code = CR_NO_RESULT_SET;
            return last_result;
        }
    }

    last_result->error_code = 0;
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
    if (dst_size < src_size * 2 + 1)
    {
        return (unsigned long)-1;
    }
    return mysql_real_escape_string(connection->real_mysql, dst, src, src_size);
}

unsigned char client_mysql_value_uint8(CLIENTMYSQLVALUE data)
{
    return (unsigned char)atoi(data.value);
}

char client_mysql_value_int8(CLIENTMYSQLVALUE data)
{
    return (char)atoi(data.value);
}

unsigned short client_mysql_value_uint16(CLIENTMYSQLVALUE data)
{
    return (unsigned short)atoi(data.value);
}

short client_mysql_value_int16(CLIENTMYSQLVALUE data)
{
    return (short)atoi(data.value);
}

unsigned int client_mysql_value_uint32(CLIENTMYSQLVALUE data)
{
    return (unsigned int)_strtoui64(data.value, 0, 10);
}

int client_mysql_value_int32(CLIENTMYSQLVALUE data)
{
    return atoi(data.value);
}

unsigned long long client_mysql_value_uint64(CLIENTMYSQLVALUE data)
{
    return _strtoui64(data.value, 0, 10);
}

long long client_mysql_value_int64(CLIENTMYSQLVALUE data)
{
    return _strtoi64(data.value, 0, 10);
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
