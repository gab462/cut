#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H

#include "cut.h"
#include <stddef.h>
#include <stdlib.h>

struct task {
    void *data;
    bool (*poll)(struct task *task);
};

static inline
bool
task_poll(struct task *task)
{
    if(task->poll == NULL)
        return(true);

    return(task->poll(task));
}

struct task_sequence {
    struct task interface;
    struct task **queue;
    bool init;
};

static inline
bool
task_sequence_poll(struct task *task)
{
    struct task_sequence *seq = (struct task_sequence *) task;

    if(q_empty(seq->queue))
        return(true);

    struct task *current = seq->queue[q_head(seq->queue)];

    if(!seq->init){ // Set input for first task
        current->data = task->data;
        seq->init = true;
    }

    bool done = task_poll(current);

    if(done){
        task->data = current->data; // Get result from task

        struct task *completed = q_dequeue(&seq->queue);
        free(completed);

        if(!q_empty(seq->queue)){
            struct task *next = seq->queue[q_head(seq->queue)];
            next->data = task->data; // Pass result as input to next task
        }else{
            q_reset(&seq->queue);
        }
    }

    return(q_empty(seq->queue));
}

static inline
struct task *
task_sequence_impl(struct task **tasks, int count)
{
    struct task_sequence task = {
        .interface.poll = task_sequence_poll
    };

    for(int i = 0; i < count; i++)
        q_enqueue(&task.queue, tasks[i]);

    return(cut_memdup(&task, sizeof(task)));
}

struct task_group {
    struct task interface;
    struct task **list;
    bool init;
};

static inline
bool
task_group_poll(struct task *task)
{
    struct task_group *group = (struct task_group *) task;

    for(int i = 0; i < da_len(group->list); i++){
        struct task *current = group->list[i];

        if(!group->init)
            current->data = task->data;

        bool done = task_poll(current);

        if(done){
            free(current);
            da_swap_delete(&group->list, i);
            i--;

            if(da_len(group->list) == 0)
                da_reset(&group->list);
        }
    }

    group->init = true;

    return(da_len(group->list) == 0);
}

static inline
struct task *
task_group_impl(struct task **tasks, int count)
{
    struct task_group task = {
        .interface.poll = task_group_poll
    };

    da_push_items(&task.list, tasks, count);

    return(cut_memdup(&task, sizeof(task)));
}

#define task_countof(arr) (sizeof(arr) / sizeof((arr)[0]))

#define task_sequence(...)                                              \
    task_sequence_impl(((struct task *[]){ __VA_ARGS__ }),              \
                       task_countof(((struct task *[]){ __VA_ARGS__ })))

#define task_group(...)                                                 \
    task_group_impl(((struct task *[]){ __VA_ARGS__ }),                 \
                    task_countof(((struct task *[]){ __VA_ARGS__ })))

#endif
