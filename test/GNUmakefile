CC := cc
CFLAGS := -std=gnu11 -Wall -Wextra

all: da

%: %.c ../cut.c
	$(CC) $(CFLAGS) -o $@ $< -ggdb -fsanitize=address,undefined
