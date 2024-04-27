all: task1

#task0: main.o start.o util.o
#	ld -m elf_i386 start.o main.o util.o -o task0
task1: start.o util.o
	ld -m elf_i386 start.o util.o -o task1

#main.o: main.c
#	 gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

start.o: start.s
	 nasm -f elf32 start.s -o start.o

util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

encoder.o: encoder.s
	nasm -f elf32 encoder.s -o encoder.o

clean:
	rm -f *.o main.o strat.o util.o task1 encoder.o