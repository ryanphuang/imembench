default: malloc.o

.PHONY: default clean

malloc.o: malloc.c
	gcc -std=c99 -c malloc.c

test: malloc.o
	gcc -std=c99 test.c malloc.o -o test

clean:
	rm -f test *.o a.out

