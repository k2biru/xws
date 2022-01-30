#ifndef ___X_WEBS_H
#define ___X_WEBS_H
#include "error_code.h"
#include "f_header.h"
#include "tool.h"


#define DEBUG_X_WEBS(...)           printf(__VA_ARGS__); printf("\n");

#define _XWS_HTTP_METHOD_NONE      0
#define _XWS_HTTP_METHOD_GET       1
#define _XWS_HTTP_METHOD_POST      2

struct _xwsParseKeyList_t {
    char *key;
    char *value;
    void *_next;
};

struct xwsParse_ctx_t {
    char *url;
    char *boundary;
    uint8_t isForm;
    uint8_t isEncode;
    uint8_t method;
    uint8_t httpVerMiror;
    struct _xwsParseKeyList_t query;
    struct _xwsParseKeyList_t header;
    size_t query_size;
    size_t headerSize;
    size_t contentSize;
};

OPRT_RET _xwsParseKeyListGetCount( struct _xwsParseKeyList_t *kl, size_t* size);
OPRT_RET _xwsParseKeyListDelete( struct _xwsParseKeyList_t *kl);
OPRT_RET _xwsParseKeyListAdd( struct _xwsParseKeyList_t *kl, const char* key, const size_t key_size, const char* value, const size_t value_size );
OPRT_RET _xwsParseKeyListGet(struct _xwsParseKeyList_t *kl, uint8_t index, char **key, char **value);
OPRT_RET _xwsParseKeyListGetValue(struct _xwsParseKeyList_t *kl, const char *key, const uint8_t caseSensitive, char **value);
 


OPRT_RET _xwsParseQuery( struct xwsParse_ctx_t *parse, char* query_str);
OPRT_RET _xwsParseHeader( struct xwsParse_ctx_t *parse, char* header_str);

OPRT_RET xwsParseInit( struct xwsParse_ctx_t* parse);
OPRT_RET xwsParse( struct xwsParse_ctx_t* parse, char* req);
OPRT_RET xwsParseDelete( struct xwsParse_ctx_t * req_parse);

OPRT_RET xwsParseHeaderGetValue( struct xwsParse_ctx_t *parse, const char* header, const uint8_t caseSensitive, char **value);
OPRT_RET xwsParseHeaderGet( struct xwsParse_ctx_t *parse, const uint8_t index, char **header, char **value);

OPRT_RET xwsParseQueryGetValue( struct xwsParse_ctx_t *parse, const char* key, const uint8_t caseSensitive, char **value);
OPRT_RET xwsParseQueryrGet( struct xwsParse_ctx_t *parse, const uint8_t index, char **key, char **value);



void _xwsToolUrlDecode(char* dest, const size_t dest_size, const char* src, const size_t src_size);

#endif