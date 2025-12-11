CC := cc
CFLAGS := -std=gnu11 -Wall -Wextra

all: da sb q chan

%: %.c ../cut.c
	$(CC) $(CFLAGS) -o $@ $< -ggdb -fsanitize=address,undefined
