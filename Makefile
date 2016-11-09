CFLAGS=-std=gnu11 -O3 -Wall -Wextra -pedantic
LN=-lcurses

vmarquee:	vmarquee.c
	gcc $(CFLAGS) $< -o $@ $(LN)
