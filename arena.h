#ifndef INCLUDE_ARENA_H
#define INCLUDE_ARENA_H

#include "cut.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <assert.h>

#define ARENA_PAGE_SIZE 4096

struct arena {
    char *current_page;
    int end;
    char **pages;
};

struct arena_opt {
    size_t count;
    bool zero;
};

#define arena_alloc(arena, T, ...)                  \
    arena_alloc_impl(arena, sizeof(T), alignof(T),  \
                     (struct arena_opt){            \
                        .count = 1,                 \
                        .zero = true __VA_OPT__(,)  \
                        __VA_ARGS__ })

static inline
void *
arena_alloc_impl(struct arena *arena, size_t size, size_t alignment, struct arena_opt opt)
{
    if(arena->current_page == NULL){
        arena->current_page = malloc(ARENA_PAGE_SIZE);
        assert(arena->current_page != NULL);
    }

    if(arena->end % alignment != 0)
        arena->end += alignment - (arena->end % alignment);

    if(arena->end + size * opt.count > ARENA_PAGE_SIZE){
        assert(size * opt.count < ARENA_PAGE_SIZE);

        da_push(&arena->pages, arena->current_page);

        arena->current_page = malloc(ARENA_PAGE_SIZE);
        assert(arena->current_page != NULL);
        arena->end = 0;
    }

    void *ptr = arena->current_page + arena->end;
    arena->end += size * opt.count;

    if(opt.zero)
        memset(ptr, 0, size * opt.count);

    return(ptr);
}

static inline
void
arena_reset(struct arena *arena)
{
    free(arena->current_page);

    if(arena->pages != NULL){
        da_foreach(page, arena->pages){
            free(*page);
        }

        da_reset(&arena->pages);
    }
}

#endif
