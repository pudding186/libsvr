#pragma once
#include <WinSock2.h>


#ifdef WIN32
#ifdef _WIN64
#if _MSC_VER > 1400
#include "../lib3rd/mysql/x64/include/vs14/mysql.h"
#include "../lib3rd/mysql/x64/include/vs14/errmsg.h"
#else
#include "../lib3rd/mysql/x64/include/vs8/mysql.h"
#include "../lib3rd/mysql/x64/include/vs8/errmsg.h"
#endif
#else
#if _MSC_VER > 1400
#include "../lib3rd/mysql/win32/include/vs14/mysql.h"
#include "../lib3rd/mysql/win32/include/vs14/errmsg.h"
#else
#include "../lib3rd/mysql/win32/include/vs8/mysql.h"
#include "../lib3rd/mysql/win32/include/vs8/errmsg.h"
#endif
#endif
#endif




#ifdef  __cplusplus
extern "C" {
#endif

    typedef struct st_client_mysql*         HCLIENTMYSQL;

    typedef struct st_client_mysql_result*  HCLIENTMYSQLRES;

    typedef struct st_client_mysql_row
    {
        MYSQL_ROW       row_values;
        unsigned long*  row_lengths;
    }CLIENTMYSQLROW;

    typedef struct st_client_mysql_value
    {
        char*           value;
        unsigned long   size;
    }CLIENTMYSQLVALUE;

    extern HCLIENTMYSQL(create_client_mysql)(const char *host, unsigned int port, const char *user,
        const char *passwd, const char* db, const char* charact_set,
        char* err_info, size_t err_info_size);

    extern void (destroy_client_mysql)(HCLIENTMYSQL connection);

    extern bool (client_mysql_query)(HCLIENTMYSQL connection, const char* sql, unsigned long length);

    extern HCLIENTMYSQLRES(client_mysql_get_result)(HCLIENTMYSQL connection);

    extern HCLIENTMYSQLRES(client_mysql_next_result)(HCLIENTMYSQLRES last_result, unsigned int* client_mysql_errno);

    extern void (client_mysql_free_result)(HCLIENTMYSQLRES result);

    extern bool (client_mysql_result_record)(HCLIENTMYSQLRES result);

    extern unsigned long long (client_mysql_result_affected)(HCLIENTMYSQLRES result);

    extern CLIENTMYSQLROW(client_mysql_fetch_row)(HCLIENTMYSQLRES result);

    extern unsigned long long (client_mysql_rows_num)(HCLIENTMYSQLRES result);

    extern unsigned int (client_mysql_fields_num)(HCLIENTMYSQLRES result);

    extern CLIENTMYSQLROW(client_mysql_row)(HCLIENTMYSQLRES result, unsigned long long row_index);

    extern CLIENTMYSQLVALUE(client_mysql_value)(CLIENTMYSQLROW row, unsigned long field_index);

    extern CLIENTMYSQLVALUE(client_mysql_row_field_value)(HCLIENTMYSQLRES result, unsigned long long row_index, unsigned long field_index);

    extern unsigned char (client_mysql_value_uint8)(CLIENTMYSQLVALUE data);

    extern char (client_mysql_value_int8)(CLIENTMYSQLVALUE data);

    extern unsigned short (client_mysql_value_uint16)(CLIENTMYSQLVALUE data);

    extern short (client_mysql_value_int16)(CLIENTMYSQLVALUE data);

    extern unsigned int (client_mysql_value_uint32)(CLIENTMYSQLVALUE data);

    extern int (client_mysql_value_int32)(CLIENTMYSQLVALUE data);

    extern unsigned long long (client_mysql_value_uint64)(CLIENTMYSQLVALUE data);

    extern long long (client_mysql_value_int64)(CLIENTMYSQLVALUE data);

    extern unsigned long (client_mysql_escape_string)(HCLIENTMYSQL connection, char* src, unsigned long src_size, char* dst, unsigned long dst_size);

    extern const char* (client_mysql_err)(HCLIENTMYSQL connection);

#ifdef  __cplusplus
}
#endif