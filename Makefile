all: unot

unot: unot.c
	cc unot.c -o unot -lX11 -lXft -I/usr/include/freetype2

clean:
	rm unot

.PHONY: all unot clean
