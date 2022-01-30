#include "x_webs.h"

OPRT_RET _xwsParseQuery( struct xwsParse_ctx_t *parse, char* query_str){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    struct xwsParse_ctx_t* p = (struct xwsParse_ctx_t*)parse;
    if(&p->query) _xwsParseKeyListDelete(&p->query);
    p->query_size = 0;
    if(strlen(query_str) == 0){
        return OPRT_OK;
    }

    /// counting query
    p->query_size = 1;
    const char *tmp = query_str;
    while(tmp = strstr(tmp, "&")){
        p->query_size++;
        tmp++;
    }
    DEBUG_X_WEBS("query_size  |%i|", p->query_size);

    uint16_t pos = 0;
    uint8_t ia;
    
    char *key = NULL;
    char *value = NULL;
    size_t keySize = 0;
    size_t valueSize = 0;

    for (ia = 0; ia < p->query_size;) {
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

        _xwsToolUrlDecode(key  ,keySize+1,keyPos  ,keySize);
        _xwsToolUrlDecode(value,valueSize+1,valuePos,valueSize);

        // DEBUG_X_WEBS("key, val %i |%s| |%s|",ia, key,value);
        _xwsParseKeyListAdd(&p->query,key,keySize,value,valueSize);
        if(key) free(key);
        if(value) free(value);
        key = NULL;
        value = NULL;

        ++ia;
        if (next_q_p == NULL) break;
        pos = next_q_p - query_str + 1;
    }
    p->query_size = ia;
    
    DEBUG_X_WEBS("total Query  |%i|", p->query_size);
    for (size_t i = 0; i < p->query_size; i++)    {
        /* code */
        char *k=NULL;
        char *v=NULL;
        _xwsParseKeyListGet(&p->query,i,&k,&v);
        DEBUG_X_WEBS("Query %i  |%s| |%s|", i, k,v);
    }
    
}

OPRT_RET _xwsParseHeader( struct xwsParse_ctx_t *parse, char* header_str){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;

    if(&parse->header) _xwsParseKeyListDelete(&parse->header);
    memset(&parse->header,0,sizeof(struct _xwsParseKeyList_t));
    parse->headerSize = 0;
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
        parse->headerSize++;
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
        _xwsParseKeyListAdd(&parse->header,key,keySize,value,valueSize);
        if(__toolStrcicmp(key,"Content-Type")==0){
            if(strstr(value,"text/plain")!=NULL){
                parse->isForm = 0;
            } else if(strstr(value,"application/x-www-form-urlencoded")!=NULL){
                parse->isForm = 0;
                parse->isEncode = 1;
            }  else if(strstr(value,"multipart/")!=NULL){
                parse->isForm = 1;
                char *boundary_start = strstr(value,"=")+2; // boundary="xxx"
                char *boundary_stop = strstr(boundary_start,"\""); // boundary="xxx"
                size_t boundary_size = boundary_stop-boundary_start;
                if(parse->boundary) free(parse->boundary);
                parse->boundary = (char*)malloc(boundary_size+1);
                memset(parse->boundary,0,boundary_size+1);
                memcpy(parse->boundary,boundary_start,boundary_size);
            }
        } else if(__toolStrcicmp(key,"Content-Length")==0){
            parse->contentSize = atoi(value);
        }
        free(key);
        free(value);
        key = NULL;
        value = NULL;
        h_start = h_stop+2;        
    }
    
    DEBUG_X_WEBS("total Header  |%i|", parse->headerSize);
    for (size_t i = 0; i < parse->headerSize; i++)    {
        /* code */
        char *k=NULL;
        char *v=NULL;
        _xwsParseKeyListGet(&parse->header,i,&k,&v);
        DEBUG_X_WEBS("Header %i  |%s| |%s|", i, k,v);
    }
    
    return OPRT_OK;
}


OPRT_RET xwsParseHeaderGetValue( struct xwsParse_ctx_t *parse, const char* header, const uint8_t caseSensitive, char **value){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    return _xwsParseKeyListGetValue(&parse->header,header,caseSensitive, &(*value));
}

OPRT_RET xwsParseHeaderGet( struct xwsParse_ctx_t *parse, const uint8_t index, char **header, char **value){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    return _xwsParseKeyListGet(&parse->header,index, &(*header), &(*value));
}

OPRT_RET xwsParseQueryGetValue( struct xwsParse_ctx_t *parse, const char* key, const uint8_t caseSensitive, char **value){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    return _xwsParseKeyListGetValue(&parse->query,key,caseSensitive, &(*value));
}

OPRT_RET xwsParseQueryrGet( struct xwsParse_ctx_t *parse, const uint8_t index, char **key, char **value){
   if(!parse) return OPRT_ERR_INVALID_ACCESS;
    return _xwsParseKeyListGet(&parse->query,index, &(*key), &(*value));

}

OPRT_RET _xwsParseKeyListGetCount( struct _xwsParseKeyList_t *kl, size_t* size){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    size_t count = 0;
    if(kl->_next != NULL){
        return _xwsParseKeyListGetCount(kl->_next, &count);
    } else {
        count = 0;
    }
    count++;
    *size = count;
    return OPRT_OK;
}

OPRT_RET _xwsParseKeyListDelete( struct _xwsParseKeyList_t *kl){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    if(kl->_next != NULL){
        return _xwsParseKeyListDelete(kl->_next);
    } else {
        if(kl->key) free(kl->key);
        kl->key = NULL;
        if(kl->value) free(kl->value);
        kl->value = NULL;
    }
    return OPRT_OK;
}

OPRT_RET _xwsParseKeyListAdd( struct _xwsParseKeyList_t *kl, const char* key, const size_t key_size, const char* value, const size_t value_size ){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    while(kl->key != NULL){
        if(kl->_next != NULL){
            kl = kl->_next;
        } else {
            kl->_next = (struct _xwsParseKeyList_t*) malloc(sizeof(struct _xwsParseKeyList_t));
            if(kl->_next == NULL) return OPRT_ERR_MALLOC_FAILED;
            memset(kl->_next,0,sizeof(struct _xwsParseKeyList_t));
        }
    }
    kl->key = (char*) malloc(key_size+1);
    if(kl->key == NULL) return OPRT_ERR_MALLOC_FAILED;
    memset(kl->key,0,key_size+1);
    kl->value = (char*) malloc(value_size+1);
    if(kl->value == NULL) {
        free(kl->key);
        kl->key = NULL;
        return OPRT_ERR_MALLOC_FAILED;
    }
    memset(kl->value,0,value_size+1);
    // copy
    memcpy(kl->key,key,key_size);
    memcpy(kl->value,value,value_size);
    // DEBUG_X_WEBS("add q at %i |%s| |%s|", kl,kl->key,kl->value);
    return OPRT_OK;
}

OPRT_RET _xwsParseKeyListGet(struct _xwsParseKeyList_t *kl, uint8_t index, char **key, char **value){
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

OPRT_RET _xwsParseKeyListGetValue(struct _xwsParseKeyList_t *kl, const char *key, const uint8_t caseSensitive, char **value){
    if(!kl) return OPRT_ERR_INVALID_ACCESS;
    if(!key) return OPRT_ERR_INVALID_ARG;
    while((caseSensitive==0? __toolStrcicmp(kl->key, key):strcmp(kl->key,key))!=0){
        kl = kl->_next;
        if(kl == NULL) return OPRT_ERR_NOT_FOUND;
    }
    *value = kl->value;
    return OPRT_OK;
}

OPRT_RET xwsParseDelete( struct xwsParse_ctx_t * parse_ctx){
    if(!parse_ctx) return OPRT_ERR_INVALID_ACCESS;
    if(parse_ctx->url) free(parse_ctx->url);
    if(parse_ctx->boundary) free(parse_ctx->boundary);
    parse_ctx->boundary = NULL;
    parse_ctx->url = NULL  ;
    parse_ctx->method = _XWS_HTTP_METHOD_NONE;
    parse_ctx->query_size = NULL;
    _xwsParseKeyListDelete(&parse_ctx->header);
    return  _xwsParseKeyListDelete(&parse_ctx->query);
}

OPRT_RET xwsParseInit( struct xwsParse_ctx_t* parse){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    memset(parse,0, sizeof(struct xwsParse_ctx_t));
}

OPRT_RET xwsParse( struct xwsParse_ctx_t* parse, char* req){
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

    // parse query
    q_start = strstr(url_b_str,"?");
    uint8_t q = (q_start!=NULL);


    DEBUG_X_WEBS("method |%s|", method_str);
    if(strcmp(method_str,"GET")==0){
        parse->method = _XWS_HTTP_METHOD_GET;
        
    DEBUG_X_WEBS("method a%i", parse->method);
    } else if(strcmp(method_str,"POST")==0){
        parse->method = _XWS_HTTP_METHOD_POST;
    } else {
        parse->method = _XWS_HTTP_METHOD_NONE;
    }
    if(parse->url) free(parse->url);
    
    DEBUG_X_WEBS("method %i", parse->method);

    const size_t url_size = (q? q_start-url_b_str:strlen(url_b_str));

    q_start++;
    DEBUG_X_WEBS("url_size %i|%i|", q,url_size);
    parse->url = (char*) malloc(url_size+1);
    memset(parse->url,0,url_size+1);
    memcpy(parse->url,url_b_str,url_size);

    DEBUG_X_WEBS("url_b  |%s|", url_b_str);
    DEBUG_X_WEBS("url  |%s|", parse->url);
    if(q) _xwsParseQuery(parse,q_start);  
    
    /// http version
    // First line of HTTP looks like "GET /path HTTP/1.1"
    char* http_start = strstr(req,"HTTP/1.");
    parse->httpVerMiror = http_start[7]-'0';

    DEBUG_X_WEBS("HTTP V |%i|", parse->httpVerMiror);
    char *header_start = strstr(http_start,"\r\n")+2;
    
    // DEBUG_X_WEBS("HTTP Header |%s|", header_start);
    _xwsParseHeader(parse,header_start);

    // char* vs;
    // _xwsParseKeyListGetValue(&parse->header,"Host",0,&vs);
    // DEBUG_X_WEBS("HOST |%s|", vs);
    if(parse->method == _XWS_HTTP_METHOD_POST){
        char *contentStart =  strstr(req,"\r\n\r\n")+4;
        DEBUG_X_WEBS("CONTENT %i|%s|",parse->contentSize,contentStart);
        if(!parse->isForm){
            if(parse->isEncode){
                _xwsParseQuery(parse,contentStart);
            } else {
                _xwsParseKeyListDelete(&parse->query);
                char pl[] ="plain";
                _xwsParseKeyListAdd(&parse->query,pl,sizeof(pl),contentStart,parse->contentSize);

            }
        } else { // is form

        }
    }
    
}


void _xwsToolUrlDecode(char* dest, const size_t dest_size, const char* src, const size_t src_size){
	char temp[] = "0x00";
	size_t len = src_size;
    memset(dest,0,dest_size);
	unsigned int i = 0;
    uint8_t j = 0;
	while (i < len)
	{
		char decodedChar;
		char encodedChar =  src[i++] ;  //get char
		if ((encodedChar == '%') && (i + 1 < len)){
			temp[2] =  src[i++];
			temp[3] =  src[i++];
			decodedChar = strtol(temp, NULL, 16);
            // DEBUG_X_WEBS("decodedChar  |%c|%s|", decodedChar,temp);
		}
		else {
			if (encodedChar == '+'){
				decodedChar = ' ';
			}
			else {
				decodedChar = encodedChar;  // normal ascii char
			}
		}
        dest[j]=decodedChar;
        // DEBUG_X_WEBS("j  |%i|%c|%s|", j,decodedChar,dest);
        j++;
	}
    return;
}

