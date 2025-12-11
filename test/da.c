#include "../cut.c"

int
main(void)
{
    int *numbers = NULL;

    push(&numbers, 10);
    assert(cap(numbers) == 2);
    assert(len(numbers) == 1);
    assert(numbers[0] == 10);

    push(&numbers, 1, 2, 3);
    assert(cap(numbers) == 8);
    assert(len(numbers) == 4);
    assert(numbers[0] == 10
           && numbers[1] == 1
           && numbers[2] == 2
           && numbers[3] == 3);

    assert(pop(&numbers) == 3);
    assert(len(numbers) == 3);
    assert(numbers[0] == 10
           && numbers[1] == 1
           && numbers[2] == 2);

    swap_delete(&numbers, 0);
    assert(len(numbers) == 2);
    assert(numbers[0] == 2 && numbers[1] == 1);

    da_push(&numbers, 0, -1, -2);
    assert(len(numbers) == 5);
    assert(cap(numbers) == 8);

    int i = 0;
    foreach(n, numbers){
        assert(*n == numbers[i]);
        i++;
    }
    assert(i == 5);

    da_reset(&numbers);
    assert(numbers == NULL);

    printf("All tests passed.\n");

    return(0);
}
