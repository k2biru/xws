#include "x_webs.h"

OPRT_RET _xwsParseQuery( struct xwsParse_ctx_t *parse, char* query_str){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    struct xwsParse_ctx_t* p = (struct xwsParse_ctx_t*)parse;
    if(&p->query) _xwsParseQueryDelete(&p->query);
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
        _xwsParseQueryAdd(&p->query,key,keySize,value,valueSize);
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
        _xwsParseQueryGet(&p->query,i,&k,&v);
        DEBUG_X_WEBS("Query %i  |%s| |%s|", i, k,v);
    }
    
}

OPRT_RET _xwsParseQueryGetCount( struct _xwsParseQuery_t *query, size_t* size){
    if(!query) return OPRT_ERR_INVALID_ACCESS;
    struct _xwsParseQuery_t* q = ( struct _xwsParseQuery_t*)query;
    size_t count = 0;
    if(q->_next != NULL){
        return _xwsParseQueryGetCount(q->_next, &count);
    } else {
        count = 0;
    }
    count++;
    *size = count;
    return OPRT_OK;
}

OPRT_RET _xwsParseQueryDelete( struct _xwsParseQuery_t *query){
    if(!query) return OPRT_ERR_INVALID_ACCESS;
    struct _xwsParseQuery_t* q = (struct _xwsParseQuery_t*)query;
    if(q->_next != NULL){
        return _xwsParseQueryDelete(q->_next);
    } else {
        if(q->key) free(q->key);
        q->key = NULL;
        if(q->value) free(q->value);
        q->value = NULL;
    }
    return OPRT_OK;
}

OPRT_RET _xwsParseQueryAdd( struct _xwsParseQuery_t *query, const char* key, const size_t key_size, const char* value, const size_t value_size ){
    if(!query) return OPRT_ERR_INVALID_ACCESS;
    while(query->key != NULL){
        if(query->_next != NULL){
            query = query->_next;
        } else {
            query->_next = (struct _xwsParseQuery_t*) malloc(sizeof(struct _xwsParseQuery_t));
            if(query->_next == NULL) return OPRT_ERR_MALLOC_FAILED;
            memset(query->_next,0,sizeof(struct _xwsParseQuery_t));
        }
    }
    query->key = (char*) malloc(key_size+1);
    if(query->key == NULL) return OPRT_ERR_MALLOC_FAILED;
    memset(query->key,0,key_size+1);
    query->value = (char*) malloc(value_size+1);
    if(query->value == NULL) {
        free(query->key);
        query->key = NULL;
        return OPRT_ERR_MALLOC_FAILED;
    }
    memset(query->value,0,value_size+1);
    // copy
    memcpy(query->key,key,key_size);
    memcpy(query->value,value,value_size);
    DEBUG_X_WEBS("add q at %i |%s| |%s|", query,query->key,query->value);
    return OPRT_OK;
}

OPRT_RET _xwsParseQueryGet(struct _xwsParseQuery_t *query, uint8_t index, char **key, char **value){
    if(!query) return OPRT_ERR_INVALID_ACCESS;
    while (index != 0){
        query = query->_next;
        index--;
    }
    // printf ("<><>>>Q %i |%s| |%s|\n", index, query->key, query->value);
    *key = query->key;
    *value = query->value;
    return OPRT_OK;
}

OPRT_RET xwsParseDelete( struct xwsParse_ctx_t * parse_ctx){
    if(!parse_ctx) return OPRT_ERR_INVALID_ACCESS;
    if(parse_ctx->url) free(parse_ctx->url);
    parse_ctx->url = NULL  ;
    parse_ctx->method = _XWS_HTTP_METHOD_NONE;
    parse_ctx->query_size = NULL;
    return  _xwsParseQueryDelete(&parse_ctx->query);
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

    // First line of HTTP request looks like "GET /path HTTP/1.1"
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

    // DEBUG_X_WEBS("all   |%i|", req);
    // DEBUG_X_WEBS("start |%i|", path_start);
    // DEBUG_X_WEBS("stop  |%i|", path_stop);
    DEBUG_X_WEBS("url_b  |%s|", url_b_str);
    DEBUG_X_WEBS("url  |%s|", parse->url);
    if(q) _xwsParseQuery(parse,q_start);  
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