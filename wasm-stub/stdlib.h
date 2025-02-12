#ifndef INCLUDE_STDLIB_STUB
#define INCLUDE_STDLIB_STUB

static inline
void *
malloc(int)
{
	return(nullptr);
}

static inline
void *
realloc(void *, int)
{
	return(nullptr);
}

static inline void free(void *) {}

#endif
