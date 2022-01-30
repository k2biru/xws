#include "tool.h"


char *__toolStrnstr(const char *s,const char *find , size_t slen){
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

int __toolStrcicmp(char const *a, char const *b){
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

void __toolUrlDecode(char* dest, const size_t dest_size, const char* src, const size_t src_size){
	char temp[] = "0x00";
	size_t len = src_size;
    memset(dest,0,dest_size);
	unsigned int i = 0;
    uint8_t j = 0;
	while (i < len)	{
		char decodedChar;
		char encodedChar =  src[i++] ;  //get char
		if ((encodedChar == '%') && (i + 1 < len)){
			temp[2] =  src[i++];
			temp[3] =  src[i++];
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
        j++;
	}
    return;
}
