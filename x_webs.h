#ifndef ___X_WEBS_H
#define ___X_WEBS_H
#include "error_code.h"
#include "f_header.h"


#define DEBUG_X_WEBS(...)           printf(__VA_ARGS__); printf("\n");

#define _XWS_HTTP_METHOD_NONE      0
#define _XWS_HTTP_METHOD_GET       1
#define _XWS_HTTP_METHOD_POST      2

struct _xwsParseQuery_t {
    char *key;
    char *value;
    // void *_back =  NULL;
    void *_next;
};

struct xwsParse_ctx_t {
    char *url;
    uint8_t method;
    struct _xwsParseQuery_t query;
    size_t query_size;
};

OPRT_RET _xwsParseQueryGetCount( struct _xwsParseQuery_t *query, size_t* size);
OPRT_RET _xwsParseQueryDelete( struct _xwsParseQuery_t *query);
OPRT_RET _xwsParseQueryAdd( struct _xwsParseQuery_t *query, const char* key, const size_t key_size, const char* value, const size_t value_size );

OPRT_RET xwsParseInit( struct xwsParse_ctx_t* parse);
OPRT_RET xwsParse( struct xwsParse_ctx_t* parse, char* req);
OPRT_RET _xwsParseQuery( struct xwsParse_ctx_t *parse, char* query_str);
OPRT_RET xwsParseDelete( struct xwsParse_ctx_t * req_parse);

void _xwsToolUrlDecode(char* dest, const size_t dest_size, const char* src, const size_t src_size);

#endif