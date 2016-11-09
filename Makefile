CFLAGS=-std=gnu11 -O3 -Wall -Wextra -pedantic
LN=-lcurses

marquee:	marquee.c
	gcc $(CFLAGS) $< -o $@ $(LN)
