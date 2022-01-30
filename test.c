#include <stdio.h>
#include "error_code.h"
#include "f_header.h"
#include "x_webs.h"
int main(){ 

    char test[] = "GET /hello.html?a=10&bb=2&ccc=a&text=Hello+Dunia%24%28%29s&ssh=sdgkjsdyfudihsfzkabdifhgawebfkdslhfajdsgafj HTTP/1.1\r\n\
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n\
Host: localhost\r\n\
Accept-Language: en-us\r\n\
Accept-Encoding: gzip, deflate\r\n\
Connection: Keep-Alive\r\n";

    struct xwsParse_ctx_t xwsP;
    xwsParseInit(&xwsP);
    xwsParse(&xwsP, test);
    char tk[] = "Connection";
    char * v;
    char *k;
    uint8_t i =2;
    xwsParseHeaderGetValue(&xwsP,tk,0,&v);
    printf("%s : |%s|\n",tk,v);
    xwsParseHeaderGet(&xwsP,i,&k,&v);
    printf("%i|%s| : |%s|\n",i,k,v);
    xwsParseDelete(&xwsP);

    return 0;
    
} 