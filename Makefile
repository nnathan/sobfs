all: sobfs

libhydrogen/libhydrogen.a: libhydrogen/hydrogen.h libhydrogen/hydrogen.c
	cd libhydrogen && make

sobfs: Makefile src/enc.c libhydrogen/libhydrogen.a
	$(CC) -O3 -std=c99 -Iinclude -Ilibhydrogen -o $@ src/enc.c libhydrogen/libhydrogen.a

clean:
	rm -f sobfs
	cd libhydrogen && make clean
