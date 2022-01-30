#ifndef __TOOL_H
#define __TOOL_H
#include <stdint.h>

char *__toolStrnstr(const char *s,const char *find , size_t slen);
int __toolStrcicmp(char const *a, char const *b);
void __toolUrlDecode(char* dest, const size_t dest_size, const char* src, const size_t src_size);

#endif