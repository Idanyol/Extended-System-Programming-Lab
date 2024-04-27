all : menu

menu : menu.o
	gcc -g -m32 -Wall -o menu menu.o

menu.o : menu.c
	gcc -m32 -Wall -c menu.c -o menu.o

.PHONY: clean

clean: 
	rm -f *.o all