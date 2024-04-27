all: encoder

encoder: encoder.o
	gcc -g -m32 -Wall -o encoder encoder.o

encoder.o : encoder.c
	gcc -g -m32 -Wall -c encoder.c -o encoder.o 

.PHONY: clean

clean:
	rm -f *.o all