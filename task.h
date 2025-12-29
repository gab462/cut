#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H

// https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

#include "arena.h"
#include "cut.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <assert.h>

enum task_state {
    TASK_IDLE,
    TASK_RUNNING
};

struct task_context {
    enum task_state state;
    int line;
    struct arena arena;
    void **ptrs;
    int current_ptr;
};

#define task_ctx_alloc(ctx, T, ...)                     \
    task_ctx_alloc_impl(ctx, sizeof(T), alignof(T),     \
                        (struct arena_opt){             \
                           .count = 1,                  \
                           .zero = true __VA_OPT__(,)   \
                           __VA_ARGS__ })

static inline
void *
task_ctx_alloc_impl(struct task_context *ctx, size_t size, size_t alignment, struct arena_opt opt)
{
    void *ptr;

    if(ctx->state == TASK_IDLE){
        // alloc if first run
        ptr = arena_alloc_impl(&ctx->arena, size, alignment, opt);
        da_push(&ctx->ptrs, ptr);

        return(ptr);
    }else{
        // otherwise reuse pointers
        assert(ctx->current_ptr < da_len(ctx->ptrs));
        ptr = ctx->ptrs[ctx->current_ptr++];
    }

    return(ptr);
}

#define task_begin(ctx) (ctx)->state = TASK_RUNNING; switch((ctx)->line){ case 0:;

#define task_return(ctx, ...) do{ (ctx)->current_ptr = 0; return __VA_ARGS__; }while(0)

#define task_yield(ctx, ...)                \
    do{                                     \
        (ctx)->line = __LINE__;             \
        task_return(ctx, __VA_ARGS__);      \
        case __LINE__:;                     \
    }while(0)

// TODO: yield_while and yield_from for functions returning values

#define task_yield_while(ctx, pred) \
    do{                             \
        if(pred){                   \
            task_yield(ctx);        \
                                    \
            if(pred)                \
                task_return(ctx);   \
        }                           \
    }while(0)

#define task_yield_from(ctx, other, other_ctx, ...)     \
    do{                                                 \
        other(other_ctx __VA_OPT__(,) __VA_ARGS__);     \
                                                        \
        if(!task_done(*(other_ctx))){                   \
            task_yield(ctx);                            \
                                                        \
            other(other_ctx __VA_OPT__(,) __VA_ARGS__); \
                                                        \
            if(!task_done(*(other_ctx)))                \
                task_return(ctx);                       \
        }                                               \
    }while(0)

#define task_ctx_reset(ctx)             \
    do{                                 \
        da_reset(&(ctx)->ptrs);         \
        arena_reset(&(ctx)->arena);     \
        memset(ctx, 0, sizeof(*(ctx))); \
    }while(0)

#define task_abort(ctx, ...)    \
    do{                         \
        task_ctx_reset(ctx);    \
        return __VA_ARGS__;     \
    }while(0)

#define task_end(ctx, ...) } task_ctx_reset(ctx); return __VA_ARGS__

#define task_done(ctx) ((ctx).state == TASK_IDLE)

#endif
