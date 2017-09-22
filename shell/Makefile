load: test
	./test

test: test.o mytoc.o myString.o
	cc -o test test.o mytoc.o myString.o

CFLAGS= -g -O3

myToc.o: mytoc.c
	cc -c mytoc.c
  
myString.o: myString.c
	cc -c myString.c  

test.o: test.c mytoc.h myString.h
	cc -c test.c

clean:
	rm -f *.o test