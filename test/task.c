#include "../cut.c"
#include <sys/time.h>

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
};

static inline
bool
sequential_task_poll(struct task *task)
{
    struct sequential_task *seq = (struct sequential_task *) task;

    if(q_empty(seq->queue))
        return(true);

    struct task *current = seq->queue[q_head(seq->queue)];

    current->data = task->data;

    bool done = task_poll(current);

    if(done){
        task->data = current->data;
        dequeue(&seq->queue);
    }

    return(q_empty(seq->queue));
}

struct sequential_task
sequential_task(void)
{
    return((struct sequential_task){
        .interface.poll = sequential_task_poll
    });
}

struct concurrent_task {
    struct task interface;
    struct task **list;
};

static inline
bool
concurrent_task_poll(struct task *task)
{
    struct concurrent_task *group = (struct concurrent_task *) task;

    for(int i = 0; i < len(group->list); i++){
        group->list[i]->data = task->data;

        bool done = task_poll(group->list[i]);

        if(done){
            swap_delete(&group->list, i);
            i--;
        }
    }

    return(len(group->list) == 0);
}

struct concurrent_task
concurrent_task(void)
{
    return((struct concurrent_task){
        .interface.poll = concurrent_task_poll
    });
}

struct sleep_task {
    struct task interface;
    int64_t until;
};

static inline
bool
sleep_task_poll(struct task *task)
{
    struct sleep_task *sleeper = (struct sleep_task *) task;
    const int64_t *dt = task->data;

    sleeper->until -= *dt;

    return(sleeper->until <= 0);
}

static inline
struct sleep_task
sleep_task(float until)
{
    return((struct sleep_task){
        .interface.poll = sleep_task_poll,
        .until = (until * 1000.f)
    });
}

static inline
int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return((int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000);
}

int
main(void)
{
    struct sleep_task first = sleep_task(1.f);

    struct sleep_task second_a = sleep_task(2.f);
    struct sleep_task second_b = sleep_task(2.f);

    struct concurrent_task second = concurrent_task();
    push(&second.list, &second_a.interface, &second_b.interface);

    struct sequential_task task = sequential_task();
    enqueue(&task.queue, &first.interface);
    enqueue(&task.queue, &second.interface);

    int64_t previous = current_time_millis();
    int64_t dt = 0;

    task.interface.data = &dt;

    while(!task_poll(&task.interface)){
        int64_t now = current_time_millis();
        dt = now - previous;
        previous = now;
    }

    da_reset(&second.list);
    q_reset(&task.queue);

    return(0);
}
