load: test
	./test

test: test.o mytoc.o
	cc -o test test.o mytoc.o

CFLAGS= -g -O3

myToc.o: mytoc.c
	cc -c mytoc.c

test.o: test.c mytoc.h
	cc -c test.c

clean:
	rm -f *.o test