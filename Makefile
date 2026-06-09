CC     = gcc
CFLAGS = -O3 -Wall
LDLIBS = -lm
SDL    = $(shell sdl2-config --cflags --libs)

ripple: ripple.c
	$(CC) $(CFLAGS) ripple.c -o ripple $(SDL) $(LDLIBS)

run: ripple
	./ripple

clean:
	rm -f ripple

.PHONY: run clean
