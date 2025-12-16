CC := cc
CFLAGS := -std=gnu11 -Wall -Wextra

all: da sb q sleep_task tcp_client tcp_server

%: %.c ../cut.c
	$(CC) $(CFLAGS) -o $@ $< -ggdb -fsanitize=address,undefined
