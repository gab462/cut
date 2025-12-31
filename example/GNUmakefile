CC := cc
CFLAGS := -std=gnu11 -Wall -Wextra -Wno-initializer-overrides

all: da sb q text_task numbers_task tcp_client tcp_server

%: %.c
	$(CC) $(CFLAGS) -o $@ $< -ggdb -fsanitize=address,undefined
