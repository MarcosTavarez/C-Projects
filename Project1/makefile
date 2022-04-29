all:	project1

project1.o:	project1.c
		gcc -c project1.c

project1:	project1.o	storage.o
	gcc project1.o	storage.o  -o project1

storage.o: storage.c storage.h
	gcc -c storage.c

clean:
		rm -f *.o project1
