CFLAGS=-std=gnu99 -O3 -Wall -Wextra -pedantic
LN=-lcurses

all:	vmarquee

vmarquee:	vmarquee.c
	gcc $(CFLAGS) $< -o $@ $(LN)

.PHONY:	clean
clean:	vmarquee
	$(RM) $^
