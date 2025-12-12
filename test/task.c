#include "../cut.c"
#include <sys/time.h>

struct task {
    bool (*poll)(struct task *task);
};

static inline
bool
task_poll(struct task *task)
{
    return task->poll(task);
}

struct sequential_task {
    struct task interface;
    struct task **queue;
};

static inline
bool
sequential_task_poll(struct task *arg)
{
    struct sequential_task *task = (struct sequential_task *) arg;

    if(q_empty(task->queue))
        return true;

    struct task *current = task->queue[q_head(task->queue)];

    bool done = task_poll(current);

    if(done)
        dequeue(&task->queue);

    return(q_empty(task->queue));
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
concurrent_task_poll(struct task *arg)
{
    struct concurrent_task *task = (struct concurrent_task *) arg;

    for(int i = 0; i < len(task->list); i++){
        bool done = task_poll(task->list[i]);

        if(done){
            swap_delete(&task->list, i);
            i--;
        }
    }

    return(len(task->list) == 0);
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
    uint64_t previous;
    int64_t until;
};

static inline
int64_t
current_time_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

static inline
bool
sleep_task_poll(struct task *arg)
{
    struct sleep_task *task = (struct sleep_task *) arg;

    if(task->previous == 0)
        task->previous = current_time_millis();

    int64_t current = current_time_millis();
    int64_t dt = current - task->previous;

    task->until -= dt;

    task->previous = current;

    return task->until <= 0;
}

static inline
struct sleep_task
sleep_task(float until)
{
    return((struct sleep_task){
        .interface.poll = sleep_task_poll,
        .previous = 0,
        .until = (until * 1000.f)
    });
}

int
main(void)
{
    struct sequential_task task = sequential_task();

    struct sleep_task first = sleep_task(1.f);
    enqueue(&task.queue, &first.interface);

    struct concurrent_task second = concurrent_task();

    struct sleep_task second_a = sleep_task(2.f);
    struct sleep_task second_b = sleep_task(2.f);
    push(&second.list, &second_a.interface, &second_b.interface);

    enqueue(&task.queue, &second.interface);

    while(!task_poll(&task.interface));

    q_reset(&task.queue);

    return(0);
}
