#include <stdio.h>
#include "error_code.h"
#include "f_header.h"
#include "x_webs.h"
int main(){
    char name[50];
    // printf("input key:");
    // scanf("%s", &name);
    // printf("Pause %s \n" , name );

    char test[] = "GET /hello.html?a=10&bb=2&ccc=a&text=Hello+Dunia%24%28%29s&ssh=sdgkjsdyfudihsfzkabdifhgawebfkdslhfajdsgafj HTTP/1.1\
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\
Host: localhost\
Accept-Language: en-us\
Accept-Encoding: gzip, deflate\
Connection: Keep-Alive";

    struct xwsParse_ctx_t xwsP;
    xwsParseInit(&xwsP);
    xwsParse(&xwsP, test);
    return 0;
    
} 