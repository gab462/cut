// Copyright (c) 2025 gab462
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef INCLUDE_CUT_C
#define INCLUDE_CUT_C

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

static inline
int
da_buf_cap(int len)
{
	int cap = 1;

	while(cap < len)
		cap *= 2;

	return(cap);
}

#define da_reserve(da, capacity)                                                  \
	do{								          \
		(da)->cap = capacity;					          \
		(da)->ptr = realloc((da)->ptr, (da)->cap * sizeof((da)->ptr[0])); \
									          \
		assert((da)->ptr != NULL);				          \
	}while(0)

#define da_push(da, ...)                                                \
	do{                                                             \
		typeof((da)->ptr[0]) items[] = { __VA_ARGS__ };         \
		int item_count = sizeof(items) / sizeof(items[0]);      \
		                                                        \
		int previous_pos = (da)->len;                           \
		(da)->len += item_count;                                \
		                                                        \
		if((da)->len > (da)->cap)                               \
			da_reserve(da, da_buf_cap((da)->len));          \
		                                                        \
		memcpy((da)->ptr + previous_pos, items, sizeof(items)); \
	}while(0)

#define da_pop(da)              \
	do{                     \
		(da)->len -= 1; \
	}while(0)

#define da_swap_delete(da, idx)                            \
	do{                                                \
		(da)->ptr[idx] = (da)->ptr[(da)->len - 1]; \
		da_pop(da);                                \
	}while(0)

#define da_reset(da)                          \
	do{                                   \
		if((da)->ptr)                 \
			free((da)->ptr);      \
		memset(da, 0, sizeof(*(da))); \
	}while(0)

#define da_for(it, da) \
	for (typeof((da).ptr) it = (da).ptr; it != (da).ptr + (da).len; ++it)

struct string_view
{
	char *ptr;
	int len;
};

static inline
struct string_view
sv(char *string)
{
	return((struct string_view){ .ptr = string, .len = strlen(string) });
}

static inline
bool
sv_equal(struct string_view a, struct string_view b)
{
	if(a.len != b.len)
		return(false);

	return(memcmp(a.ptr, b.ptr, a.len) == 0);
}

static inline
struct string_view
sv_right(struct string_view string, int n)
{
	return((struct string_view){ string.ptr + string.len - n, n });
}

static inline
struct string_view
sv_left(struct string_view string, int n)
{
	return((struct string_view){ string.ptr, n });
}

static inline
struct string_view
sv_chop_right(struct string_view string, int n)
{
	return((struct string_view){ string.ptr, string.len - n });
}

static inline
struct string_view
sv_chop_left(struct string_view string, int n)
{
	return((struct string_view){ string.ptr + n, string.len - n });
}

static inline
bool
char_is_whitespace(char c)
{
	return(c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static inline
struct string_view
sv_trim_left(struct string_view string)
{
	for(int i = 0; i < string.len; ++i){
		if(!char_is_whitespace(string.ptr[i]))
			return(sv_chop_left(string, i));
	}

	return((struct string_view){});
}

static inline
struct string_view
sv_trim_right(struct string_view string)
{
	for(int i = 0; i < string.len; ++i){
		if(!char_is_whitespace(string.ptr[string.len - 1 - i]))
			return(sv_chop_right(string, i));
	}

	return((struct string_view){});
}

static inline
struct string_view
sv_trim(struct string_view string)
{
	return(sv_trim_left(sv_trim_right(string)));
}

static inline
int
sv_find(struct string_view string, struct string_view substring)
{
	if(substring.len > string.len)
		return(-1);

	int match = 0;

	for(int i = 0; i < string.len; ++i){
		if(string.ptr[i] == substring.ptr[match]){
			++match;

			if(match == substring.len)
				return(i + 1 - match);
		}else
			match = (string.ptr[i] == substring.ptr[0]) ? 1 : 0;
	}

	return(-1);
}

static inline
struct string_view *
sv_split_sv(struct string_view string, struct string_view sep, int *count)
{
	struct { struct string_view *ptr; int len; int cap; } strings = {0};

	while(string.len > 0){
		int found = sv_find(string, sep);

		if(found == -1)
			found = string.len;

		da_push(&strings, sv_left(string, found));
		string = sv_chop_left(string, found + sep.len);
	}

	/* Shrink to exactly length */
	da_reserve(&strings, strings.len);

	*count = strings.len;

	return(strings.ptr);
}

static inline
struct string_view *
sv_split(struct string_view string, char *sep, int *count)
{
	return(sv_split_sv(string, sv(sep), count));
}

static inline
struct string_view *
sv_split_once_sv(struct string_view string, struct string_view sep, int *count)
{
	struct { struct string_view *ptr; int len; int cap; } strings = {};

	int found = sv_find(string, sep);

	if(found == -1){
		da_push(&strings, string);
	}else{
		da_push(&strings, sv_left(string, found));
		da_push(&strings, sv_chop_left(string, found + sep.len));
	}

	*count = strings.len;

	return(strings.ptr);
}

static inline
struct string_view *
sv_split_once(struct string_view string, char *sep, int *count)
{
	return(sv_split_once_sv(string, sv(sep), count));
}

static inline
char *
sv_save(struct string_view string)
{
	char *cstr = malloc(string.len + 1);
	assert(cstr != NULL);

	memcpy(cstr, string.ptr, string.len);
	cstr[string.len] = '\0';

	return(cstr);
}

struct string_buffer
{
	char *ptr;
	int len;
	int cap;
};

static inline
struct string_view
sv_from_sb(struct string_buffer string)
{
	return((struct string_view){ .ptr = string.ptr, .len = string.len });
}

static inline
void
sb_reserve(struct string_buffer *string, int cap)
{
	da_reserve(string, cap);
}

static inline
void
sb_append_sv(struct string_buffer *string, struct string_view other)
{
	int previous_pos = string->len;
	string->len += other.len;

	if(string->len > string->cap)
		da_reserve(string, da_buf_cap(string->len));

	memcpy(string->ptr + previous_pos, other.ptr, other.len);
}

static inline
void
sb_append(struct string_buffer *string, char *other)
{
	sb_append_sv(string, sv(other));
}

static inline
void
sb_terminate(struct string_buffer *string)
{
	sb_append_sv(string, (struct string_view){ .ptr = "", .len = 1 });
}

static inline
struct string_buffer
sb_from_file(char *path)
{
	FILE *f = fopen(path, "rb");

	if(f == NULL){
		perror("string_from_file");
		return((struct string_buffer){0});
	}

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = malloc(fsize);
	fread(buf, fsize, 1, f);

	fclose(f);

	return((struct string_buffer){ .ptr = buf, .len = fsize, .cap = fsize });
}

static inline
char *
sb_save(struct string_buffer string)
{
	return(sv_save(sv_from_sb(string)));
}

static inline
void
sb_reset(struct string_buffer *string)
{
	if(string->ptr)
		free(string->ptr);
	memset(string, 0, sizeof(struct string_buffer));
}

struct memory_arena
{
	char *start;
	char *end;
};

constexpr unsigned long long ma_size = 64ull * 1024 * 1024 * 1024; /* 64GB */

static inline
void
ma_reset(struct memory_arena *arena)
{
	if(arena->start == NULL){
		arena->start = mmap(NULL, ma_size, PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0);

		assert(arena->start != MAP_FAILED);
	}

	arena->end = arena->start;
}

static inline
void
ma_align(struct memory_arena *arena, int alignment)
{
	unsigned long long pos = arena->end - arena->start;

	if(pos % alignment != 0)
		arena->end += alignment - (pos % alignment);
}

static inline
void *ma_allocate_impl(struct memory_arena *arena, int size, int alignment)
{
	if(arena->start == NULL)
		ma_reset(arena);

	ma_align(arena, alignment);

	char *ptr = arena->end;

	arena->end += size;

	return(ptr);
}

#define ma_allocate(arena, T) ma_allocate_impl(arena, sizeof(T), alignof(T))

#define ma_allocate_n(arena, T, count) ma_allocate_impl(arena, sizeof(T) * count, alignof(T))

static inline
struct memory_arena
ma_scratch(struct memory_arena arena)
{
	return((struct memory_arena){ arena.end, arena.end });
}

static inline
void ma_free(struct memory_arena arena)
{
	munmap(arena.start, ma_size);
}

#define arena_da_reserve(arena, da, cap)                                        \
	do{                                                                     \
		da->items = arena_allocate_n(arena, typeof(da->items[0]), cap); \
		da->cap = cap;                                                  \
	}while(0)

#define arena_sb_reserve(arena, sb, cap) arena_da_reserve(arena, sb, cap)

#endif
