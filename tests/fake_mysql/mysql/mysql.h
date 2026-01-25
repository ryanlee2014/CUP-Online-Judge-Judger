#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MYSQL MYSQL;
typedef struct MYSQL_RES MYSQL_RES;
typedef char **MYSQL_ROW;

enum { MYSQL_OPT_CONNECT_TIMEOUT = 0 };

MYSQL *mysql_init(MYSQL *mysql);
int mysql_options(MYSQL *mysql, int option, const void *arg);
MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user,
                          const char *passwd, const char *db, unsigned int port,
                          const char *unix_socket, unsigned long clientflag);
int mysql_real_query(MYSQL *mysql, const char *stmt_str, unsigned long length);
MYSQL_RES *mysql_store_result(MYSQL *mysql);
MYSQL_ROW mysql_fetch_row(MYSQL_RES *res);
void mysql_free_result(MYSQL_RES *res);
const char *mysql_error(MYSQL *mysql);
void mysql_close(MYSQL *mysql);

#ifdef __cplusplus
}
#endif
