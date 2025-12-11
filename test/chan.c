#include "../cut.c"

struct in_out {
    int *in;
    int *out;
};

void *
worker(void *arg)
{
    struct in_out *chan = arg;

    int n = get(&chan->in);
    printf("got %d\n", n);

    put(&chan->out, n);
    printf("sent %d\n", n);

    return(NULL);
}

int
main(void)
{
    int *chan = NULL;

    chan_grow(&chan);

    // Queue tests

    put(&chan, 10);
    assert(cap(chan) == 2);
    assert(q_head(chan) == 0);
    assert(q_tail(chan) == 1);

    put(&chan, 1);
    assert(cap(chan) == 6);
    assert(q_head(chan) == 0);
    assert(q_tail(chan) == 2);

    assert(get(&chan) == 10);
    assert(q_head(chan) == 1);
    assert(q_tail(chan) == 2);

    assert(get(&chan) == 1);
    assert(q_head(chan) == 2);
    assert(q_tail(chan) == 2);

    // Threading tests
    // FIXME: deadlocking

    pthread_t *threads = NULL;
    da_reserve(&threads, 5);
    da_header(threads)->length = 5;

    struct in_out inout = {0};
    chan_grow(&inout.in);
    chan_grow(&inout.out);

    foreach(thread, threads){
        pthread_create(thread, NULL, worker, &inout);
    }

    for(int i = 0; i < len(threads); i++){
        printf("put %d\n", i);
        put(&inout.in, i);
    }

    foreach(thread, threads){
        pthread_join(*thread, NULL);
    }

    int sum = 0;
    for(int i = 0; i < len(threads); i++){
        sum += get(&inout.out);
    }

    assert(sum == 10);

    // Cleanup

    chan_reset(&chan);
    assert(chan == NULL);

    printf("All tests passed.\n");

    return(0);
}
