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

        struct task *completed = dequeue(&seq->queue);
        free(completed);

        if(q_empty(seq->queue))
            q_reset(&seq->queue);
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
        enqueue(&seq->queue, tasks[i]);

    return &seq->interface;
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
        struct task *current = group->list[i];

        current->data = task->data;

        bool done = task_poll(current);

        if(done){
            free(current);
            swap_delete(&group->list, i);

            if(len(group->list) == 0)
                da_reset(&group->list);

            i--;
        }
    }

    return(len(group->list) == 0);
}

static inline
struct task *
concurrent_task(struct task **tasks, int count)
{
    struct concurrent_task *group = calloc(1, sizeof(struct concurrent_task));
    group->interface.poll = concurrent_task_poll;

    push_items(&group->list, tasks, count);

    return &group->interface;
}

#define task_countof(arr) (sizeof(arr) / sizeof((arr)[0]))

#define task_sequence(...)                                              \
    sequential_task(((struct task *[]){ __VA_ARGS__ }),                 \
                    task_countof(((struct task *[]){ __VA_ARGS__ })))

#define task_group(...)                                                 \
    concurrent_task(((struct task *[]){ __VA_ARGS__ }),                 \
                    task_countof(((struct task *[]){ __VA_ARGS__ })))

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
struct task *
sleep_task(float until)
{
    struct sleep_task *sleeper = calloc(1, sizeof(struct sleep_task));
    sleeper->interface.poll = sleep_task_poll;
    sleeper->until = until * 1000.f;

    return &sleeper->interface;
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
    struct task *task =
        task_sequence(
            sleep_task(1.f),
            task_group(
                sleep_task(2.f),
                sleep_task(2.f)
            )
        );

    int64_t previous = current_time_millis();
    int64_t dt = 0;

    task->data = &dt;

    while(!task_poll(task)){
        int64_t now = current_time_millis();
        dt = now - previous;
        previous = now;
    }

    free(task);

    // TODO: cleanup memory

    return(0);
}
