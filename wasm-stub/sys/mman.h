#ifndef INCLUDE_MMAN_STUB
#define INCLUDE_MMAN_STUB

extern char __heap_base;

enum
{
	PROT_READ,
	PROT_WRITE,
	MAP_ANON,
	MAP_PRIVATE,
	MAP_NORESERVE
};

#define MAP_FAILED ((void *) -1)

static inline
void *
mmap(void *, unsigned long long, int, int, int, int)
{
	return(&__heap_base);
}

static inline void munmap(void *, unsigned long long) {}

#endif
