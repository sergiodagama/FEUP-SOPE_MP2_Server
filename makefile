all: c s

s: server.o lib.o delay.c delay.h utils.c utils.h queue.c queue.h
		gcc -Wall -DDELAY=100 -o s delay.c queue.c utils.c lib.o server.o -pthread -g

c: client.c common.h
		gcc -Wall -o c client.c -pthread

server.o: server.c common.h 
		gcc -Wall -c -o server.o server.c

lib.o: lib.c lib.h
		gcc -Wall -c -o lib.o lib.c

clean:
		rm -f s c
