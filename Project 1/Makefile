CFLAGS=-W -Wall

all: thv1 thv2 thv3 thv4

clean:
	rm *.o

thv1: thv1.o p1fxns.o
	cc -o thv1 thv1.o p1fxns.o

thv2: thv2.o p1fxns.o
	cc -o thv2 thv2.o p1fxns.o

thv3: thv3.o p1fxns.o
	cc -o thv3 thv3.o p1fxns.o

thv4: thv4.o p1fxns.o
	cc -o thv4 thv4.o p1fxns.o

thv1.o: thv1.c
	gcc -c thv1.c

thv2.o: thv2.c
	gcc -c thv2.c

thv3.o: thv3.c
	gcc -c thv3.c

thv4.o: thv4.c
	gcc -c thv4.c

p1fnxs.o: p1fnxs.c p1fnxs.h
	gcc -c p1fnxs.c
