#ifndef ___X_WEBSS_H
#define ___X_WEBSS_H
#include "error_code.h"
#include "f_header.h"
#include "tool.h"
#include <stddef.h>
#include <stdint.h>


// #define DEBUG_X_WEBS(...)           printf(__VA_ARGS__); printf("\n");
#define DEBUG_X_WEBS(...)           

#define _XWS_HTTP_METHOD_NONE      0
#define _XWS_HTTP_METHOD_GET       1
#define _XWS_HTTP_METHOD_POST      2
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _xwsKeyList_t {
    char *key;
    char *value;
    void *_next;
} _xwsKeyList_t;

typedef struct _xwsFormList_t {
    char *name;
    char *value;
    char *type;
    char *fileName;
    uint8_t *content;
    size_t contentSize;
    void *_next;
} _xwsFormList_t;

typedef struct xwsCtx_t {
    char *url;
    char *boundary;
    uint8_t isForm;
    uint8_t isEncode;
    uint8_t method;
    uint8_t httpVerMiror;
    size_t querySize;
    size_t headerSize;
    size_t contentSize;
    struct _xwsKeyList_t query;
    struct _xwsKeyList_t header;
    struct _xwsFormList_t form;
    
} xwsCtx_t;

OPRT_RET _xwsKeyListGetCount( struct _xwsKeyList_t *kl, size_t* size);
OPRT_RET _xwsKeyListDelete( struct _xwsKeyList_t *kl);
OPRT_RET _xwsKeyListAdd( struct _xwsKeyList_t *kl, const char* key, const size_t keySize, const char* value, const size_t valueSIze );
OPRT_RET _xwsKeyListGet(struct _xwsKeyList_t *kl, uint8_t index, char **key, char **value);
OPRT_RET _xwsKeyListGetValue(struct _xwsKeyList_t *kl, const char *key, const uint8_t caseSensitive, char **value);


OPRT_RET _xwsFormAdd( struct _xwsFormList_t *f, const char* name, const size_t nameSize, const char* value, const size_t valueSize, const char* type, const size_t typeSize, const char* fileName, const size_t fileNameSize,const uint8_t *content, const size_t contentSize);
OPRT_RET _xwsFormDelete( struct _xwsFormList_t *f);
OPRT_RET _xwsFormGet(struct _xwsFormList_t *f,uint8_t index, char **name, char **value, char **type, char **fileName,uint8_t **content,size_t *contentSize);
OPRT_RET _xwsFormGetFromName(struct _xwsFormList_t *f, const char *name, char **value, char **type, char **fileName, uint8_t **content, size_t *contentSize);

OPRT_RET _xwsQuery( struct xwsCtx_t *xwsCtx, char* query_str);
OPRT_RET _xwsParseHeader( struct xwsCtx_t *xwsCtx, char* header_str);

OPRT_RET xwsInit( struct xwsCtx_t* xwsCtx);
OPRT_RET xwsParse( struct xwsCtx_t* xwsCtx, char* req);
OPRT_RET xwsDelete( struct xwsCtx_t * xwsCtx);

OPRT_RET xwsHeaderGetValue( struct xwsCtx_t *xwsCtx, const char* header, const uint8_t caseSensitive, char **value);
OPRT_RET xwsHeaderGet( struct xwsCtx_t *xwsCtx, const uint8_t index, char **header, char **value);

OPRT_RET xwsQueryGetValue( struct xwsCtx_t *xwsCtx, const char* key, const uint8_t caseSensitive, char **value);
OPRT_RET xwsQueryGet( struct xwsCtx_t *xwsCtx, const uint8_t index, char **key, char **value);

OPRT_RET xwsFormGetFromName(struct xwsCtx_t *xwsCtx, const char *name, char **value, char **type, char **fileName, uint8_t **content, size_t *contentSize);
OPRT_RET xwsFormGet(struct xwsCtx_t *xwsCtx,uint8_t index, char **name, char **value, char **type, char **fileName,uint8_t **content,size_t *contentSize);


#ifdef __cplusplus
}
#endif
#endif