#include "../cut.c"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

int
main(void)
{
    int *q = NULL;

    enqueue(&q, 10);
    assert(cap(q) == 2);
    assert(q_head(q) == 0);
    assert(q_tail(q) == 1);

    enqueue(&q, 1);
    assert(cap(q) == 6);
    assert(q_head(q) == 0);
    assert(q_tail(q) == 2);

    assert(dequeue(&q) == 10);
    assert(q_head(q) == 1);
    assert(q_tail(q) == 2);

    assert(dequeue(&q) == 1);
    assert(q_head(q) == 2);
    assert(q_tail(q) == 2);

    int i = 0;
    enqueue(&q, i++);
    enqueue(&q, i++);
    enqueue(&q, i++);
    enqueue(&q, i++);
    enqueue(&q, i++);
    assert(cap(q) == 6);
    assert(q_head(q) == 2);
    assert(q_tail(q) == 1);

    enqueue(&q, i++);
    assert(cap(q) == 14);
    assert(q_head(q) == 10);
    assert(q_tail(q) == 2);

    for(int n = 0; n < i; n++){
        assert(q[(q_head(q) + n) % cap(q)] == n);
    }

    q_reset(&q);
    assert(q == NULL);

    printf("All tests passed.\n");

    return(0);
}
