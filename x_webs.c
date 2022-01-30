#include "x_webs.h"

OPRT_RET _xwsParseQuery( struct xwsParse_ctx_t *parse, char* query_str){
    if(!parse) return OPRT_ERR_INVALID_ACCESS;
    struct xwsParse_ctx_t* p = (struct xwsParse_ctx_t*)parse;
    if(&p->query) _xwsParseQueryDelete(&p->query);
    char key[k_f_string_size_medium];
    char value[k_f_string_size_medium];
    memset(key,0,k_f_string_size_medium);
    memset(value,0,k_f_string_size_medium);

    // DEBUG_X_WEBS("HERE");
    p->query_size = 0;
    if(strlen(query_str) == 0){
        return OPRT_OK;
    }

    // DEBUG_X_WEBS("HERE2");
    /// counting query
    p->query_size = 1;
    const char *tmp = query_str;
    while(tmp = strstr(tmp, "&"))
    {
    p->query_size++;
    tmp++;
    }
    DEBUG_X_WEBS("p->query_size  |%i|", p->query_size);

    int pos = 0;
    int iarg;
    for (iarg = 0; iarg < p->query_size;) {
        char* eq_sign_p = strstr(query_str+pos,"=");
        char* next_q_p = strstr(query_str+pos,"&");        
        if((eq_sign_p == NULL) ||((eq_sign_p >next_q_p) && (next_q_p != NULL))){
            // tak ada value, sorry
            if(next_q_p == NULL) {
                // DEBUG_X_WEBS("HERE 4");
                break;
            }
            pos = next_q_p-query_str +1;
            DEBUG_X_WEBS("HERE 5");
            continue;
        }

        // memset(key,0,k_f_string_size_medium);
        // memset(value,0,k_f_string_size_medium);
        // memcpy(key,query_str+pos,eq_sign_p-(query_str+pos));
        // memcpy(value,eq_sign_p+1,next_q_p-eq_sign_p-1);

        // DEBUG_X_WEBS("key  |%s|", key);
        // DEBUG_X_WEBS("val  |%s|", value);

        memset(key,0,k_f_string_size_medium);
        memset(value,0,k_f_string_size_medium);
        _xwsToolUrlDecode(key,k_f_string_size_medium,query_str+pos,eq_sign_p-(query_str+pos));
        _xwsToolUrlDecode(value,k_f_string_size_medium,eq_sign_p+1,next_q_p-eq_sign_p-1);

        DEBUG_X_WEBS("key, val %i |%s| |%s|",iarg, key,value);
        _xwsParseQueryAdd(&p->query,key,strlen(key),value,strlen(value));

        ++iarg;
        if (next_q_p == NULL) break;
        pos = next_q_p - query_str + 1;
    }
    p->query_size = iarg;
    
    DEBUG_X_WEBS("p->query_size  |%i|", p->query_size);
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
    struct _xwsParseQuery_t* q = (struct _xwsParseQuery_t*)query;
    if(q->key != NULL){
        q->_next = (struct _xwsParseQuery_t*) malloc(sizeof(struct _xwsParseQuery_t));
        if(q->_next == NULL) return OPRT_ERR_MALLOC_FAILED;
        memset(q->_next,0,sizeof(struct _xwsParseQuery_t));
        return _xwsParseQueryAdd(q->_next, key,key_size,value,value_size);
    } else {
        q->key = (char*) malloc(key_size+1);
        if(q->key == NULL) return OPRT_ERR_MALLOC_FAILED;
        memset(q->key,0,key_size+1);
        q->value = (char*) malloc(value_size+1);
        if(q->value == NULL) {
            free(q->key);
            q->key = NULL;
            return OPRT_ERR_MALLOC_FAILED;
        }
        memset(q->value,0,value_size+1);
        // copy
        memcpy(q->key,key,key_size);
        memcpy(q->value,value,value_size);
    }
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
		char encodedChar =  src[i++] ; // text.charAt(i++);
		if ((encodedChar == '%') && (i + 1 < len)){
			temp[2] =  dest[i++];
			temp[3] =  dest[i++];

			decodedChar = strtol(temp, NULL, 16);
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
        // memcpy(dest+strlen(dest),&decodedChar,sizeof(decodedChar));
        // strcat(dest,decodedChar);
		// decoded += decodedChar;
	}
	// return decoded;
    return;
}