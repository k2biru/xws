#ifndef ___X_WEBS_H
#define ___X_WEBS_H
#include "error_code.h"
#include "f_header.h"
#include "tool.h"


#define DEBUG_X_WEBS(...)           printf(__VA_ARGS__); printf("\n");

#define _XWS_HTTP_METHOD_NONE      0
#define _XWS_HTTP_METHOD_GET       1
#define _XWS_HTTP_METHOD_POST      2

struct _xwsKeyList_t {
    char *key;
    char *value;
    void *_next;
};

struct xws_ctx_t {
    char *url;
    char *boundary;
    uint8_t isForm;
    uint8_t isEncode;
    uint8_t method;
    uint8_t httpVerMiror;
    struct _xwsKeyList_t query;
    struct _xwsKeyList_t header;
    size_t query_size;
    size_t headerSize;
    size_t contentSize;
};

OPRT_RET _xwsKeyListGetCount( struct _xwsKeyList_t *kl, size_t* size);
OPRT_RET _xwsKeyListDelete( struct _xwsKeyList_t *kl);
OPRT_RET _xwsKeyListAdd( struct _xwsKeyList_t *kl, const char* key, const size_t key_size, const char* value, const size_t value_size );
OPRT_RET _xwsKeyListGet(struct _xwsKeyList_t *kl, uint8_t index, char **key, char **value);
OPRT_RET _xwsKeyListGetValue(struct _xwsKeyList_t *kl, const char *key, const uint8_t caseSensitive, char **value);
 


OPRT_RET _xwsQuery( struct xws_ctx_t *xwsCtx, char* query_str);
OPRT_RET _xwsParseHeader( struct xws_ctx_t *xwsCtx, char* header_str);

OPRT_RET xwsInit( struct xws_ctx_t* xwsCtx);
OPRT_RET xwsParse( struct xws_ctx_t* xwsCtx, char* req);
OPRT_RET xwsDelete( struct xws_ctx_t * xwsCtx);

OPRT_RET xwsHeaderGetValue( struct xws_ctx_t *xwsCtx, const char* header, const uint8_t caseSensitive, char **value);
OPRT_RET xwsHeaderGet( struct xws_ctx_t *xwsCtx, const uint8_t index, char **header, char **value);

OPRT_RET xwsQueryGetValue( struct xws_ctx_t *xwsCtx, const char* key, const uint8_t caseSensitive, char **value);
OPRT_RET xwsQueryGet( struct xws_ctx_t *xwsCtx, const uint8_t index, char **key, char **value);




#endif