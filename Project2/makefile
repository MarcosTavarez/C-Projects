all:	project2 server

project2.o:	project2.c
		gcc -c project2.c

project2:	project2.o	storage_remote.o
	gcc project2.o	storage_remote.o  -o project2

server.o:	server.c
	gcc -c server.c

server:	server.o	storage.o
	gcc server.o storage.o -o server

storage_remote.o: storage_remote.c storage_common.h storage_remote.h
	gcc -c storage_remote.c

storage.o: storage.c storage.h
	gcc -c storage.c
pipes:
	mkfifo pipe_in
	mkfifo pipe_out

clean:
		rm -f *.o project2
		rm pipe_in
		rm pipe_out
