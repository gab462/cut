#ifndef INCLUDE_TASK_C
#define INCLUDE_TASK_C

#include "cut.c"

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

struct sequential_task {
    struct task interface;
    struct task **queue;
    bool init;
};

static inline
bool
sequential_task_poll(struct task *task)
{
    struct sequential_task *seq = (struct sequential_task *) task;

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
sequential_task(struct task **tasks, int count)
{
    struct sequential_task *seq = calloc(1, sizeof(struct sequential_task));
    seq->interface.poll = sequential_task_poll;

    for(int i = 0; i < count; i++)
        q_enqueue(&seq->queue, tasks[i]);

    return(&seq->interface);
}

struct concurrent_task {
    struct task interface;
    struct task **list;
    bool init;
};

static inline
bool
concurrent_task_poll(struct task *task)
{
    struct concurrent_task *group = (struct concurrent_task *) task;

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
concurrent_task(struct task **tasks, int count)
{
    struct concurrent_task *group = calloc(1, sizeof(struct concurrent_task));
    group->interface.poll = concurrent_task_poll;

    da_push_items(&group->list, tasks, count);

    return(&group->interface);
}

#define task_countof(arr) (sizeof(arr) / sizeof((arr)[0]))

#define task_sequence(...)                                              \
    sequential_task(((struct task *[]){ __VA_ARGS__ }),                 \
                    task_countof(((struct task *[]){ __VA_ARGS__ })))

#define task_group(...)                                                 \
    concurrent_task(((struct task *[]){ __VA_ARGS__ }),                 \
                    task_countof(((struct task *[]){ __VA_ARGS__ })))

#endif
