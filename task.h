#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H

// https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

#include <stddef.h>
#include <stdlib.h>

#define task_context_begin() struct task_context { int task__line
#define task_context_end() }

#define task_ctx(ctx) (*((struct task_context **) ctx))

#define task_begin(ctx)                                                         \
    *(ctx) = *(ctx) == NULL ? calloc(1, sizeof(struct task_context)) : *(ctx);  \
    assert(*(ctx) != NULL);                                                     \
    switch(task_ctx(ctx)->task__line){ case 0:;

#define task_yield(ctx, ...)                    \
    do{                                         \
        task_ctx(ctx)->task__line = __LINE__;   \
        return __VA_ARGS__;                     \
        case __LINE__:;                         \
    }while(0)

#define task_abort(ctx, ...) do{ free(*(ctx)); *(ctx) = NULL; return __VA_ARGS__; }while(0)

#define task_end(ctx, ...) } free(*(ctx)); *(ctx) = NULL; return __VA_ARGS__

#endif
