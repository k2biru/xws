#include "xws.h"

OPRT_RET _xwsQuery( struct xwsCtx_t *xwsCtx, char* query_str){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    // struct xwsCtx_t* xwsCtx = (struct xwsCtx_t*)xwsCtx;
    if(&xwsCtx->query) _xwsKeyListDelete(&xwsCtx->query);
    xwsCtx->querySize = 0;
    if(strlen(query_str) == 0){
        return OPRT_OK;
    }

    /// counting query
    xwsCtx->querySize = 1;
    const char *tmp = query_str;
    while(tmp = strstr(tmp, "&")){
        xwsCtx->querySize++;
        tmp++;
    }
    DEBUG_X_WEBS("querySize  |%i|", xwsCtx->querySize);

    uint16_t pos = 0;
    uint8_t ia;
    
    char *key = NULL;
    char *value = NULL;
    size_t keySize = 0;
    size_t valueSize = 0;

    for (ia = 0; ia < xwsCtx->querySize;) {
        char* eq_sign_p = strstr(query_str+pos,"=");
        char* next_q_p = strstr(query_str+pos,"&");  
        // DEBUG_X_WEBS("ia  |%i|%d %d >>>>>>>>>>>>> %i %i", ia, eq_sign_p,next_q_p,(eq_sign_p > next_q_p),(next_q_p != NULL));      
        if((eq_sign_p == NULL) || ((eq_sign_p > next_q_p) && (next_q_p != NULL))) {
            // tak ada value, sorry
            if(next_q_p == NULL) {
                break;
            }
            pos = next_q_p-query_str +1;
            continue;
        }
        keySize = eq_sign_p-(query_str+pos);
        valueSize = (next_q_p == NULL)? strlen(eq_sign_p+1):next_q_p-eq_sign_p-1;
        key = (char*) malloc(keySize +1);
        if(key == NULL) return OPRT_ERR_MALLOC_FAILED;
        memset(key,0,keySize+1);
        value = (char*) malloc(valueSize+1);
        if(value == NULL) {
            free(key);
            return OPRT_ERR_MALLOC_FAILED;
        }
        memset(value,0,valueSize+1);

        char* keyPos = query_str+pos;
        char* valuePos = eq_sign_p+1;

        __toolUrlDecode(key  ,keySize+1,keyPos  ,keySize);
        __toolUrlDecode(value,valueSize+1,valuePos,valueSize);

        // DEBUG_X_WEBS("key, val %i |%s| |%s|",ia, key,value);
        _xwsKeyListAdd(&xwsCtx->query,key,keySize,value,valueSize);
        if(key) free(key);
        if(value) free(value);
        key = NULL;
        value = NULL;

        ++ia;
        if (next_q_p == NULL) break;
        pos = next_q_p - query_str + 1;
    }
    xwsCtx->querySize = ia;
    
    DEBUG_X_WEBS("total Query  |%i|", xwsCtx->querySize);
    for (size_t i = 0; i < xwsCtx->querySize; i++)    {
        /* code */
        char *k=NULL;
        char *v=NULL;
        _xwsKeyListGet(&xwsCtx->query,i,&k,&v);
        DEBUG_X_WEBS("Query %i  |%s| |%s|", i, k,v);
    }
    
}

OPRT_RET _xwsParseHeader( struct xwsCtx_t *xwsCtx, char* header_str){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;

    if(&xwsCtx->header) _xwsKeyListDelete(&xwsCtx->header);
    memset(&xwsCtx->header,0,sizeof(struct _xwsKeyList_t));
    xwsCtx->headerSize = 0;
    if(strlen(header_str) == 0){
        return OPRT_OK;
    }
    char *h_start = header_str;
    char *h_stop = NULL;
    char *h_div = NULL;
    char *key = NULL;
    char *value =NULL;
    size_t keySize = 0;
    size_t valueSize = 0;
    while (1) {
        h_stop = strstr(h_start,"\r\n");
        if((h_stop-h_start) <=0) {
            //no header, kek
            break;
        }
        h_div = __toolStrnstr(h_start,":",h_stop-h_start);
        if(h_div == NULL){
            // no div ":", sory
            break;
        }
        xwsCtx->headerSize++;
        keySize = (h_div-h_start);
        valueSize = (h_stop-(h_div+2));
        key = (char*)malloc(keySize+1);
        if(!key) return OPRT_ERR_MALLOC_FAILED;
        memset(key,0,keySize+1);
        value = (char*)malloc(valueSize+1);
        if(!value) {
            free(key);
            return OPRT_ERR_MALLOC_FAILED;
        }
        memset(value,0,valueSize+1);
        memcpy(key,h_start,keySize);
        memcpy(value,h_div+2,valueSize);
        
        // DEBUG_X_WEBS("Header |%s||%s|",key,value);
        _xwsKeyListAdd(&xwsCtx->header,key,keySize,value,valueSize);
        if(__toolStrcicmp(key,"Content-Type")==0){
            if(strstr(value,"text/plain")!=NULL){
                xwsCtx->isForm = 0;
            } else if(strstr(value,"application/x-www-form-urlencoded")!=NULL){
                xwsCtx->isForm = 0;
                xwsCtx->isEncode = 1;
            }  else if(strstr(value,"multipart/")!=NULL){
                xwsCtx->isForm = 1;
                char *boundary_start = strstr(value,"=")+1; // boundary="xxx"\r\n
                char *boundary_stop = strstr(boundary_start,"\r\n")-1;
                size_t boundary_size = boundary_stop-boundary_start;
                if(xwsCtx->boundary) free(xwsCtx->boundary);
                xwsCtx->boundary = (char*)malloc(boundary_size+1);
                memset(xwsCtx->boundary,0,boundary_size+1);
                memcpy(xwsCtx->boundary,boundary_start,boundary_size);
                __toolStrremove(xwsCtx->boundary, "\""); // remove " 
            }
        } else if(__toolStrcicmp(key,"Content-Length")==0){
            xwsCtx->contentSize = atoi(value);
        }
        free(key);
        free(value);
        key = NULL;
        value = NULL;
        h_start = h_stop+2;        
    }
    
    DEBUG_X_WEBS("total Header  |%i|", xwsCtx->headerSize);
    for (size_t i = 0; i < xwsCtx->headerSize; i++)    {
        /* code */
        char *k=NULL;
        char *v=NULL;
        _xwsKeyListGet(&xwsCtx->header,i,&k,&v);
        DEBUG_X_WEBS("Header %i  |%s| |%s|", i, k,v);
    }
    
    return OPRT_OK;
}


OPRT_RET xwsHeaderGetValue( struct xwsCtx_t *xwsCtx, const char* header, const uint8_t caseSensitive, char **value){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsKeyListGetValue(&xwsCtx->header,header,caseSensitive, &(*value));
}

OPRT_RET xwsHeaderGet( struct xwsCtx_t *xwsCtx, const uint8_t index, char **header, char **value){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsKeyListGet(&xwsCtx->header,index, &(*header), &(*value));
}

OPRT_RET xwsQueryGetValue( struct xwsCtx_t *xwsCtx, const char* key, const uint8_t caseSensitive, char **value){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsKeyListGetValue(&xwsCtx->query,key,caseSensitive, &(*value));
}

OPRT_RET xwsQueryGet( struct xwsCtx_t *xwsCtx, const uint8_t index, char **key, char **value){
   if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsKeyListGet(&xwsCtx->query,index, &(*key), &(*value));

}

OPRT_RET _xwsKeyListGetCount( struct _xwsKeyList_t *kl, size_t* size){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    size_t count = 0;
    if(kl->_next != NULL){
        return _xwsKeyListGetCount(kl->_next, &count);
    } else {
        count = 0;
    }
    count++;
    *size = count;
    return OPRT_OK;
}

OPRT_RET _xwsKeyListDelete( struct _xwsKeyList_t *kl){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    if(kl->_next != NULL){
        return _xwsKeyListDelete(kl->_next);
    } else {
        if(kl->key) free(kl->key);
        if(kl->value) free(kl->value);
        if(kl->_next) free(kl->_next);
        memset(kl,0,sizeof(struct _xwsKeyList_t));
    }
    return OPRT_OK;
}

OPRT_RET _xwsKeyListAdd( struct _xwsKeyList_t *kl, const char* key, const size_t keySize, const char* value, const size_t valueSIze ){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    while(kl->key != NULL){
        if(kl->_next != NULL){
            kl = kl->_next;
        } else {
            kl->_next = (struct _xwsKeyList_t*) malloc(sizeof(struct _xwsKeyList_t));
            if(kl->_next == NULL) return OPRT_ERR_MALLOC_FAILED;
            memset(kl->_next,0,sizeof(struct _xwsKeyList_t));
        }
    }
    kl->key = (char*) malloc(keySize+1);
    if(kl->key == NULL) return OPRT_ERR_MALLOC_FAILED;
    memset(kl->key,0,keySize+1);
    kl->value = (char*) malloc(valueSIze+1);
    if(kl->value == NULL) {
        free(kl->key);
        kl->key = NULL;
        return OPRT_ERR_MALLOC_FAILED;
    }
    memset(kl->value,0,valueSIze+1);
    // copy
    memcpy(kl->key,key,keySize);
    memcpy(kl->value,value,valueSIze);
    // DEBUG_X_WEBS("add q at %i |%s| |%s|", kl,kl->key,kl->value);
    return OPRT_OK;
}

OPRT_RET _xwsKeyListGet(struct _xwsKeyList_t *kl, uint8_t index, char **key, char **value){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    while (index != 0){
        kl = kl->_next;
        index--;
    }
    // printf ("<><>>>Q %i |%s| |%s|\n", index, kl->key, kl->value);
    *key = kl->key;
    *value = kl->value;
    return OPRT_OK;
}

OPRT_RET _xwsKeyListGetValue(struct _xwsKeyList_t *kl, const char *key, const uint8_t caseSensitive, char **value){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    if(!key) return OPRT_ERR_INVALID_ARG;
    while((caseSensitive==0? __toolStrcicmp(kl->key, key):strcmp(kl->key,key))!=0){
        kl = kl->_next;
        if(kl == NULL) return OPRT_ERR_NOT_FOUND;
    }
    *value = kl->value;
    return OPRT_OK;
}

OPRT_RET xwsDelete( struct xwsCtx_t * xwsCtx_ctx){
    if(!xwsCtx_ctx) return OPRT_ERR_INVALID_ACCESS;
    if(xwsCtx_ctx->url) free(xwsCtx_ctx->url);
    if(xwsCtx_ctx->boundary) free(xwsCtx_ctx->boundary);
    xwsCtx_ctx->boundary = NULL;
    xwsCtx_ctx->url = NULL  ;
    xwsCtx_ctx->method = _XWS_HTTP_METHOD_NONE;
    xwsCtx_ctx->querySize = NULL;
    _xwsKeyListDelete(&xwsCtx_ctx->header);
    _xwsFormDelete(&xwsCtx_ctx->form)
    return  _xwsKeyListDelete(&xwsCtx_ctx->query);
}

OPRT_RET xwsInit( struct xwsCtx_t* xwsCtx){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    memset(xwsCtx,0, sizeof(struct xwsCtx_t));
}

OPRT_RET xwsParse( struct xwsCtx_t* xwsCtx, char* req){
    char *path_start =  NULL;
    char *path_stop =  NULL;
    char *q_start =  NULL;
    char *q_stop =  NULL;
    char method_str[k_f_string_size_ultra_light];
    memset(method_str,0,k_f_string_size_ultra_light);
    char url_b_str[k_f_string_size_long_long];
    memset(url_b_str,0,k_f_string_size_long_long);
    DEBUG_X_WEBS("REQ =========\n%s\n===========",req)

    // First line of HTTP looks like "GET /path HTTP/1.1"
    path_start = strstr(req," ")+1;
    path_stop = strstr(path_start," ");
    if(path_start== NULL ||path_stop == NULL){
        //Invalid request
        return OPRT_ERR_INVALID_OPERTATION;
    }
    if(path_stop-req >=k_f_string_size_long_long){
        // URL too long
        return OPRT_ERR_INVALID_OPERTATION;
    }
    /// get method
    memcpy(method_str,req,(path_start-req)-1);
    // get URL
    memcpy(url_b_str,path_start,(path_stop-path_start));

    // xwsCtx query
    q_start = strstr(url_b_str,"?");
    uint8_t q = (q_start!=NULL);


    DEBUG_X_WEBS("method |%s|", method_str);
    if(strcmp(method_str,"GET")==0){
        xwsCtx->method = _XWS_HTTP_METHOD_GET;
        
    DEBUG_X_WEBS("method a%i", xwsCtx->method);
    } else if(strcmp(method_str,"POST")==0){
        xwsCtx->method = _XWS_HTTP_METHOD_POST;
    } else {
        xwsCtx->method = _XWS_HTTP_METHOD_NONE;
    }
    if(xwsCtx->url) free(xwsCtx->url);
    
    DEBUG_X_WEBS("method %i", xwsCtx->method);

    const size_t url_size = (q? q_start-url_b_str:strlen(url_b_str));

    q_start++;
    DEBUG_X_WEBS("url_size %i|%i|", q,url_size);
    xwsCtx->url = (char*) malloc(url_size+1);
    memset(xwsCtx->url,0,url_size+1);
    memcpy(xwsCtx->url,url_b_str,url_size);

    DEBUG_X_WEBS("url_b  |%s|", url_b_str);
    DEBUG_X_WEBS("url  |%s|", xwsCtx->url);
    if(q) _xwsQuery(xwsCtx,q_start);  
    
    /// http version
    // First line of HTTP looks like "GET /path HTTP/1.1"
    char* http_start = strstr(req,"HTTP/1.");
    xwsCtx->httpVerMiror = http_start[7]-'0';

    DEBUG_X_WEBS("HTTP V |%i|", xwsCtx->httpVerMiror);
    char *header_start = strstr(http_start,"\r\n")+2;
    
    // DEBUG_X_WEBS("HTTP Header |%s|", header_start);
    _xwsParseHeader(xwsCtx,header_start);

    // char* vs;
    // _xwsKeyListGetValue(&xwsCtx->header,"Host",0,&vs);
    // DEBUG_X_WEBS("HOST |%s|", vs);
    if(xwsCtx->method == _XWS_HTTP_METHOD_POST){
        char *contentStart =  strstr(req,"\r\n\r\n")+4;
        DEBUG_X_WEBS("CONTENT %i|%s|",xwsCtx->contentSize,contentStart);
        if(!xwsCtx->isForm){
            if(xwsCtx->isEncode){
                _xwsQuery(xwsCtx,contentStart);
            } else {
                _xwsKeyListDelete(&xwsCtx->query);
                char pl[] ="plain";
                _xwsKeyListAdd(&xwsCtx->query,pl,sizeof(pl),contentStart,xwsCtx->contentSize);

            }
        } else { // is form

        }
    }
    
}

OPRT_RET _xwsFormAdd( struct _xwsFormList_t *f, 
                      const char* name,
                      const size_t nameSize,
                      const char* value,
                      const size_t valueSize,
                      const char* type,
                      const size_t typeSize,
                      const char* fileName,
                      const size_t fileNameSize,
                      const uint8_t *content,
                      const size_t contentSize){
    if(!f) return OPRT_ERR_INVALID_ACCESS;
    if(!name) return OPRT_ERR_INVALID_ARG;

    while(f->name != NULL){
        if(f->_next != NULL){
            f = f->_next;
        } else {
            f->_next = (struct _xwsFormList_t*) malloc(sizeof(struct _xwsFormList_t));
            if(f->_next == NULL) return OPRT_ERR_MALLOC_FAILED;
            memset(f->_next,0,sizeof(struct _xwsFormList_t));
        }
    }

    f->name = (char*) malloc(nameSize+1);
    if(f->name == NULL) goto _xwsFormAdd_failed;
    memset(f->name,0,nameSize+1);
    memcpy(f->name,name,nameSize);

    f->value = (char*) malloc(valueSize+1);
    if(f->value == NULL) goto _xwsFormAdd_failed;
    memset(f->value,0,valueSize+1);
    memcpy(f->value,value,valueSize);

    f->type = (char*) malloc(typeSize+1);
    if(f->type == NULL) goto _xwsFormAdd_failed;
    memset(f->type,0,typeSize+1);
    memcpy(f->type,type,typeSize);

    f->fileName = (char*) malloc(fileName+1);
    if(f->fileName == NULL) goto _xwsFormAdd_failed;
    memset(f->fileName,0,fileName+1);
    memcpy(f->fileName,fileName,fileNameSize);

    f->content = (char*) malloc(content);
    if(f->content == NULL) goto _xwsFormAdd_failed;
    memcpy(f->content,content,contentSize);

    return OPRT_OK;

    /// failed malloc
    _xwsFormAdd_failed:
        if(f->name) free(f->name);
        if(f->value) free(f->value);
        if(f->type) free(f->type);
        if(f->fileName) free(f->fileName);
        if(f->content) free(f->content);
        memset(f,0,sizeof(struct _xwsFormList_t));
        return OPRT_ERR_MALLOC_FAILED;
}

OPRT_RET _xwsFormDelete( struct _xwsFormList_t *f){
    if(!f) return OPRT_ERR_INVALID_ACCESS;
    if(f->_next != NULL){
        return _xwsFormDelete(f->_next);
    } else {
        if(f->name) free(f->name);
        if(f->value) free(f->value);
        if(f->type) free(f->type);
        if(f->fileName) free(f->fileName);
        if(f->content) free(f->content);
        if(f->_next) free(f->_next);
        memset(f,0,sizeof(struct _xwsFormList_t));
    }
    return OPRT_OK;
}

OPRT_RET _xwsFormGet(struct _xwsFormList_t *f, 
                    uint8_t index,
                    char **name,
                    char **value,
                    char **type,
                    char **fileName,
                    uint8_t **content,
                    size_t *contentSize
                    ){
    if(!f) return OPRT_ERR_INVALID_ACCESS;
    while (index != 0){
        f = f->_next;
        index--;
    }

    *name = f->name;
    if(*value != NULL) *value = f->value;
    if(*type != NULL) *type = f->type;
    if(*fileName != NULL) *fileName = f->fileName;
    if(*content != NULL) {
        *content = f->content;
        contentSize = f->contentSize;
    }
    return OPRT_OK;
}

OPRT_RET _xwsFormGetFromName(struct _xwsFormList_t *f, 
                    const char *name,
                    char **value,
                    char **type,
                    char **fileName,
                    uint8_t **content,
                    size_t *contentSize
                    ){
    
    if(!f) return OPRT_ERR_INVALID_ACCESS;
    if(!name) return OPRT_ERR_INVALID_ARG;
    while(strcmp(f->name,name))!=0){
        f = f->_next;
        if(f == NULL) return OPRT_ERR_NOT_FOUND;
    }
    
    if(*value != NULL) *value = f->value;
    if(*type != NULL) *type = f->type;
    if(*fileName != NULL) *fileName = f->fileName;
    if(*content != NULL) {
        *content = f->content;
        contentSize = f->contentSize;
    }
    return OPRT_OK;
}

OPRT_RET xwsFormGetFromName(struct xwsCtx_t *xwsCtx, const char *name, char **value, char **type, char **fileName, uint8_t **content, size_t *contentSize){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsFormGetFromName(&xwsCtx->form,name,&(*value),&(*type),&(*fileName),&(*content),contentSize);
}

OPRT_RET xwsFormGet(struct xwsCtx_t *xwsCtx,uint8_t index, char **name, char **value, char **type, char **fileName,uint8_t **content,size_t *contentSize){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    return _xwsFormGet(&xwsCtx->form,index,&(*name),&(*value),&(*type),&(*fileName),&(*content),contentSize);
}



// POST /test HTTP/1.1
// Host: foo.example
// Content-Type: multipart/form-data;boundary="boundary"
//
// --boundary
// Content-Disposition: form-data; name="field1"
//
// value1
// --boundary
// Content-Disposition: form-data; name="field2"; filename="example.txt"
//
// value2
// --boundary--

// WIP
OPRT_RET _xwsParseForm(struct xwsCtx_t* xwsCtx, char* form){
    if(!xwsCtx) return OPRT_ERR_INVALID_ACCESS;
    // read boundary

    if (xwsCtx->boundary == NULL) return OPRT_ERR_INVALID_OPERTATION;

    const size_t bMarkerSize = strlen(xwsCtx->boundary)+2;
    char *bMarker = (char*) malloc(bMarkerSize+1);
    memset(bMarker,0,bMarkerSize+1);
    memcpy(bMarker,"--",2);
    memcpy(bMarker+2,xwsCtx->boundary,strlen(xwsCtx->boundary));

    char *fStart = form;
    char *fStop = NULL;
    char *bStart = NULL;
    char *bStop = NULL;

    while (1) {
        fStart = strstr(fStrat,bMarker);
        fStop = strstr(fStrat,bMarker)? strlen(fStrat)<

    }
    
    if(bMarker) free(bMarker);

}