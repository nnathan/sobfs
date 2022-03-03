all: sobfs

sobfs: Makefile src/enc.c include/monocypher.h src/monocypher.c
	$(CC) -std=c99 -Iinclude -o $@ src/enc.c src/monocypher.c

clean:
	rm -f sobfs
