#include "../cut.c"

int
main(void)
{
    char *sb = NULL;

    char *hello = "Hello";

    append(&sb, hello);
    assert(cap(sb) == (int) strlen(hello) * 2);
    assert(len(sb) == (int) strlen(hello));
    assert(strncmp(sb, hello, strlen(hello)) == 0);

    char *hello_number = "Hello, 4 3.2";
    appendf(&sb, ", %d %.1f", 4, 3.2f);
    assert(cap(sb) == (int) strlen(hello_number) * 2);
    assert(len(sb) == (int) strlen(hello_number));
    assert(strncmp(sb, hello_number, strlen(hello_number)) == 0);

    da_reset(&sb);
    assert(sb == NULL);

    printf("All tests passed.\n");

    return(0);
}
