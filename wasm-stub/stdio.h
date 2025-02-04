#ifndef INCLUDE_STDIO_STUB
#define INCLUDE_STDIO_STUB

typedef int FILE;

enum
{
    SEEK_END,
    SEEK_SET
};

static inline
FILE *
fopen(const char *, const char *)
{
    return(NULL);
}

static inline
int
fclose(FILE *)
{
    return(NULL);
}

static inline void perror(const char *) {}

static inline
int
fseek(FILE *, long, int)
{
    return(0);
}

static inline
long ftell(FILE *)
{
    return(0);
}

static inline void fread(char *, int, int, FILE *) {}

#endif
