all: sobfs

sobfs: Makefile src/enc.c include/gimli.h src/gimli.c
	$(CC) -std=c99 -Iinclude -o $@ src/enc.c src/gimli.c

clean:
	rm -f sobfs
