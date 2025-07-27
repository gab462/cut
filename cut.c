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
#include <assert.h>
#include <stdio.h>

struct da_header
{
	int len, cap;
	max_align_t start[0];
};

static inline
struct da_header *
da_header(void *da)
{
	return(da - sizeof(struct da_header));
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

#define da_reserve(da, capacity)					\
	do{								\
		struct da_header *header;				\
									\
		if (*(da) == nullptr){					\
			header = malloc(sizeof(struct da_header)	\
					+ sizeof(**(da)) * capacity);	\
									\
			assert(header != nullptr);			\
									\
			header->len = 0;				\
		}else{							\
			header = realloc(da_header(*(da)),		\
					 sizeof(struct da_header)	\
					 + sizeof(**(da)) * capacity);	\
									\
			assert(header != nullptr);			\
		}							\
									\
		header->cap = capacity;					\
									\
		*(da) = (void *) header->start;				\
	}while(0)

#define da_push_items(da, items, item_count)				\
	do{								\
		int len = da_len(*(da));				\
		int cap = da_cap(*(da));				\
									\
		if(len + item_count > cap){				\
			cap = cap == 0 ? 1 : cap;			\
			while(cap < len) cap *= 2;			\
			da_reserve(da, cap);				\
		}							\
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

#define da_for(it, da) for(typeof(da) it = (da); it != (da) + da_len(da); ++it)

#define sb_append(sb, str) da_push_items(sb, str, strlen(str))

#define sb_appendf(sb, fmt, ...)						\
	do{									\
		int size = snprintf(nullptr, 0, fmt, __VA_ARGS__);		\
		if(da_len(*(sb)) + size + 1 > da_cap(*(sb))) 			\
			da_reserve(sb, da_len(*(sb)) + size + 1);		\
		snprintf(*(sb) + da_len(*(sb)), size + 1, fmt, __VA_ARGS__);	\
		da_header(*(sb))->len += size;					\
	}while(0)

#define defer(exp) for(bool done = false; !done; ({exp;}), done = true)

#endif
