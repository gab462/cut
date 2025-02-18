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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

struct da_header
{
	int len, cap;
	max_align_t start;
};

constexpr int da_header_offset = sizeof(struct da_header) - sizeof(max_align_t);

static inline
struct da_header *
da_header(void *da)
{
	return((struct da_header *) (((char *) da) - da_header_offset));
}

static inline
int
da_len(void *da)
{
	if (da != nullptr)
		return(da_header(da)->len);
	else
		return(0);
}

static inline
int
da_cap(void *da)
{
	if (da != nullptr)
		return(da_header(da)->cap);
	else
		return(0);
}

static inline
int
da_next_cap(int len)
{
	int cap = 1;

	while(cap < len)
		cap *= 2;

	return(cap);
}

#define da_reserve(da, capacity)					\
	do{								\
		struct da_header *header;				\
									\
		if (*(da) == nullptr){					\
			header = malloc(da_header_offset		\
					+ sizeof(**(da)) * capacity);	\
									\
			assert(header != nullptr);			\
									\
			header->len = 0;				\
		}else{							\
			header = realloc(da_header(*(da)),		\
					 da_header_offset		\
					 + sizeof(**(da)) * capacity);	\
									\
			assert(header != nullptr);			\
		}							\
									\
		header->cap = capacity;					\
									\
		*(da) = (void *) &header->start;			\
	}while(0)

#define da_push_items(da, items, item_count)				\
	do{								\
		int len = da_len(*(da));				\
		int cap = da_cap(*(da));				\
									\
		if(len + item_count > cap)				\
			da_reserve(da, da_next_cap(len + item_count));	\
									\
		memcpy(*(da) + len, items,				\
		       sizeof(**(da)) * item_count);			\
									\
		da_header(*(da))->len += item_count;			\
	}while(0)

#define da_push(da, ...)						\
	do{								\
		typeof(**(da)) items[] = { __VA_ARGS__ };		\
		int item_count = sizeof(items) / sizeof(items[0]);	\
		da_push_items(da, items, item_count);			\
	}while(0)

#define da_pop(da)				\
	do{					\
		da_header(*(da))->len -= 1;	\
	}while(0)

#define da_swap_delete(da, idx)					\
	do{							\
		(*(da))[idx] = (*(da))[da_len(*(da)) - 1];	\
		da_pop(da);					\
	}while(0)

#define da_reset(da)				\
	do{					\
		if(*(da) != nullptr)		\
			free(da_header(*(da)));	\
		*(da) = nullptr;		\
	}while(0)

#define da_for(it, da) for (typeof(da) it = da; it != da + da_len(da); ++it)

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
sv_split_sv(struct string_view string, struct string_view sep)
{
	struct string_view *strings = nullptr;

	while(string.len > 0){
		int found = sv_find(string, sep);

		if(found == -1)
			found = string.len;

		da_push(&strings, sv_left(string, found));
		string = sv_chop_left(string, found + sep.len);
	}

	/* Shrink to exactly length */
	da_reserve(&strings, da_len(strings));

	return(strings);
}

static inline
struct string_view *
sv_split(struct string_view string, char *sep)
{
	return(sv_split_sv(string, sv(sep)));
}

static inline
struct string_view *
sv_split_once_sv(struct string_view string, struct string_view sep)
{
	struct string_view *strings = nullptr;

	int found = sv_find(string, sep);

	if(found == -1){
		da_push(&strings, string);
	}else{
		da_push(&strings, sv_left(string, found));
		da_push(&strings, sv_chop_left(string, found + sep.len));
	}

	return(strings);
}

static inline
struct string_view *
sv_split_once(struct string_view string, char *sep)
{
	return(sv_split_once_sv(string, sv(sep)));
}

static inline
int
sv_count_sv(struct string_view string, struct string_view substring)
{
	int count = 0;

	for(int i = 0; i < string.len; ++i){
		if(sv_equal(sv_left(sv_chop_left(string, i), substring.len), substring)){
			++count;
			i += substring.len - 1;
		}
	}

	return(count);
}

static inline
int
sv_count(struct string_view string, char *substring)
{
	return(sv_count_sv(string, sv(substring)));
}

static inline
char *
sv_save(struct string_view string)
{
	char *cstr = malloc(string.len + 1);
	assert(cstr != nullptr);

	memcpy(cstr, string.ptr, string.len);
	cstr[string.len] = '\0';

	return(cstr);
}

static inline
int
sb_len(char *string)
{
	return(da_len(string));
}

static inline
struct string_view
sv_from_sb(char *string)
{
	return((struct string_view){ .ptr = string, .len = sb_len(string) });
}

static inline
int
sb_cap(char *string)
{
	return(da_cap(string));
}

static inline
void
sb_reserve(char **string, int cap)
{
	da_reserve(string, cap);
}

static inline
void
sb_append_sv(char **string, struct string_view other)
{
	da_push_items(string, other.ptr, other.len);
}

static inline
void
sb_append(char **string, char *other)
{
	sb_append_sv(string, sv(other));
}

static inline
void
sb_terminate(char **string)
{
	sb_append_sv(string, (struct string_view){ .ptr = "", .len = 1 });
}

static inline
char *
sb_from_file(char *path)
{
	FILE *f = fopen(path, "rb");

	if(f == nullptr){
		perror("string_from_file");
		return(nullptr);
	}

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = nullptr;
	da_reserve(&buf, fsize);
	fread(buf, fsize, 1, f);

	fclose(f);

	return(buf);
}

static inline
char *
sb_save(char *string)
{
	return(sv_save(sv_from_sb(string)));
}

static inline
void
sb_reset(char **string)
{
	da_reset(string);
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
	if(arena->start == nullptr){
		arena->start = mmap(nullptr, ma_size, PROT_READ | PROT_WRITE,
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
	if(arena->start == nullptr)
		ma_reset(arena);

	ma_align(arena, alignment);

	char *ptr = arena->end;

	arena->end += size;

	return(ptr);
}

#define ma_allocate(arena, T) ma_allocate_impl(arena, sizeof(T), alignof(T))

#define ma_allocate_n(arena, T, count) ma_allocate_impl(arena, sizeof(T) * (count), alignof(T))

static inline
struct memory_arena
ma_scratch(struct memory_arena arena)
{
	return((struct memory_arena){ arena.end, arena.end });
}

static inline
void ma_free(struct memory_arena arena)
{
	if(arena.start)
		munmap(arena.start, ma_size);
}

#define ma_da_reserve(arena, da, capacity)			\
	do{							\
		assert(*(da) == nullptr); /* !! */		\
								\
		struct da_header *header =			\
			ma_allocate_impl(			\
				arena,				\
				da_header_offset		\
				+ sizeof(**(da)) * capacity,	\
				alignof(struct da_header));	\
								\
		header->len = 0;				\
		header->cap = capacity;				\
		*da = (void *) &header->start;			\
	}while(0)

static inline
void
ma_sb_reserve(struct memory_arena *arena, char **string, int cap)
{
	ma_da_reserve(arena, string, cap);
}

/* TODO: reduce duplication for arena string functions */

static inline
char *
ma_sv_save(struct memory_arena *arena, struct string_view string)
{
	char *cstr = ma_allocate_n(arena, char, string.len + 1);

	memcpy(cstr, string.ptr, string.len);
	cstr[string.len] = '\0';

	return(cstr);
}

static inline
struct string_view *
ma_sv_split_sv(struct memory_arena *arena,
	       struct string_view string, struct string_view sep)
{
	struct string_view *strings = nullptr;

	ma_da_reserve(arena, &strings, sv_count_sv(string, sep) + 1);

	while(string.len > 0){
		int found = sv_find(string, sep);

		if(found == -1)
			found = string.len;

		da_push(&strings, sv_left(string, found));
		string = sv_chop_left(string, found + sep.len);
	}

	return(strings);
}

static inline
struct string_view *
ma_sv_split(struct memory_arena *arena,
	    struct string_view string, char *sep)
{
	return(ma_sv_split_sv(arena, string, sv(sep)));
}

static inline
struct string_view *
ma_sv_split_once_sv(struct memory_arena *arena,
		    struct string_view string, struct string_view sep)
{
	struct string_view *strings = nullptr;

	int found = sv_find(string, sep);

	if(found == -1){
		ma_da_reserve(arena, &strings, 1);
		da_push(&strings, string);
	}else{
		ma_da_reserve(arena, &strings, 2);
		da_push(&strings, sv_left(string, found));
		da_push(&strings, sv_chop_left(string, found + sep.len));
	}

	return(strings);
}

static inline
struct string_view *
ma_sv_split_once(struct memory_arena *arena,
		 struct string_view string, char *sep)
{
	return(ma_sv_split_once_sv(arena, string, sv(sep)));
}

static inline
struct string_view
ma_sv_from_file(struct memory_arena *arena, char *path)
{
	FILE *f = fopen(path, "rb");

	if(f == nullptr){
		perror("string_from_file");
		return((struct string_view){});
	}

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = ma_allocate_n(arena, char, fsize);
	fread(buf, fsize, 1, f);

	fclose(f);

	return((struct string_view){ .ptr = buf, .len = fsize });
}

#endif
