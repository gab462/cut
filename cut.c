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

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

struct da_header
{
    int len, cap;
    max_align_t start[];
};

static inline
struct da_header *
da_header(void *da)
{
    if(da != NULL)
        return(da - sizeof(struct da_header));
    else
        return(NULL);
}

static inline
int
da_len(void *da)
{
    if(da != NULL)
        return(da_header(da)->len);
    else
        return(0);
}

static inline
int
da_cap(void *da)
{
    if(da != NULL)
        return(da_header(da)->cap);
    else
        return(0);
}

#define da_reserve(da, capacity)                        \
    do{                                                 \
        struct da_header *header;                       \
                                                        \
        header = realloc(da_header(*(da)),              \
                         sizeof(struct da_header)       \
                         + sizeof(**(da)) * capacity);  \
                                                        \
        assert(header != NULL);                         \
                                                        \
        header->cap = capacity;                         \
                                                        \
        *(da) = (void *) header->start;                 \
    }while(0)

#define da_push_items(da, items, item_count)                        \
    do{                                                             \
        int len = da_len(*(da));                                    \
        int cap = da_cap(*(da));                                    \
                                                                    \
        if(len + item_count > cap)                                  \
            da_reserve(da, (len + item_count) * 2);                 \
                                                                    \
        memcpy(*(da) + len, items, sizeof(**(da)) * item_count);    \
                                                                    \
        da_header(*(da))->len = len + item_count;                   \
    }while(0)

#define da_push(da, ...)                                    \
    do{                                                     \
        __typeof__(**(da)) items[] = { __VA_ARGS__ };       \
        int item_count = sizeof(items) / sizeof(items[0]);  \
        da_push_items(da, items, item_count);               \
    }while(0)

#define da_pop(da)                              \
    do{                                         \
        assert(*(da) != NULL);                  \
                                                \
        da_header(*(da))->len -= 1;             \
    }while(0)

#define da_swap_delete(da, idx)                     \
    do{                                             \
        assert(*(da) != NULL);                      \
                                                    \
        (*(da))[idx] = (*(da))[da_len(*(da)) - 1];  \
        da_pop(da);                                 \
    }while(0)

#define da_reset(da)                            \
    do{                                         \
        if(*(da) != NULL){                      \
            free(da_header(*(da)));             \
            *(da) = NULL;                       \
        }                                       \
    }while(0)

#define da_for(it, da) for(__typeof__(da) it = (da); it != (da) + da_len(da); ++it)

#define sb_append(sb, str) da_push_items(sb, str, strlen(str))

#define sb_appendf(sb, fmt, ...)                                        \
    do{                                                                 \
        int size = snprintf(NULL, 0, fmt, __VA_ARGS__);                 \
                                                                        \
        if(da_len(*(sb)) + size + 1 > da_cap(*(sb)))                    \
            da_reserve(sb, da_len(*(sb)) + size + 1);                   \
                                                                        \
        snprintf(*(sb) + da_len(*(sb)), size + 1, fmt, __VA_ARGS__);    \
                                                                        \
        da_header(*(sb))->len += size;                                  \
    }while(0)

struct q_header
{
    int head;
    int tail;
    struct da_header da;
};

static inline
struct q_header *
q_header(void *q)
{
    if(q != NULL)
        return(q - sizeof(struct q_header));
    else
        return(NULL);
}

static inline
int
q_head(void *q)
{
    if(q != NULL)
        return(q_header(q)->head);
    else
        return(0);
}

static inline
int
q_tail(void *q)
{
    if(q != NULL)
        return(q_header(q)->tail);
    else
        return(0);
}

#define q_grow(q)                               \
    do{                                         \
        int old_cap = da_cap(*(q));             \
                                                \
        da_reserve(q, (old_cap + 1) * 2);       \
                                                \
        int growth = da_cap(*(q)) - old_cap;    \
        int head = q_head(*(q));                \
                                                \
        if(q_tail(*(q)) < head){                \
            memmove(*(q) + head + growth,       \
                    *(q) + head,                \
                    old_cap - head);            \
                                                \
            q_header(*(q))->head += growth;     \
        }                                       \
    }while(0)

#define q_enq(q, item)                                              \
    do{                                                             \
        if(da_cap(*(q)) == 0                                        \
           || (q_tail(*(q)) + 1) % da_cap(*(q)) == q_head(*(q)))    \
            q_grow(q);                                              \
                                                                    \
        (*(q))[q_tail(*(q))] = item;                                \
        q_header(*(q))->tail = (q_tail(*(q)) + 1) % da_cap(*(q));   \
    }while(0)

#define q_deq(q, item)                                                  \
    (                                                                   \
        assert(*(q) != NULL),                                           \
        q_header(*(q))->head = (q_head(*(q)) + 1) % da_cap(*(q)),       \
        (*(q))[q_head(*(q)) > 0 ? q_head(*(q)) - 1 : da_cap(*(q)) - 1]  \
    )

#define q_reset(q)                              \
    do{                                         \
        if(*(q) != NULL){                       \
            free(q_header(*(q)));               \
            *(q) = NULL;                        \
        }                                       \
    }while(0)

#define defer(exp) for(bool done = false; !done; ({exp;}), done = true)

#endif
