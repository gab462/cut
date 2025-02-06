#ifndef INCLUDE_STRING_STUB
#define INCLUDE_STRING_STUB

static inline
int
memcmp(const char *a, const char *b, int len){
    // 0 for equal and non-zero for non-equal

    for(int i = 0; i < len; ++i){
        if(a[i] != b[i])
            return(-1);
    }

    return(0);
}

static inline
int
strlen(const char *s)
{
    int len = 0;

    while(*s){
        ++s;
        ++len;
    }

    return(len);
}

static inline
void
memcpy(void *dst, const void *src, int len)
{
    for(int i = 0; i < len; ++i)
        ((char *) dst)[i] = ((char *) src)[i];
}

#endif
