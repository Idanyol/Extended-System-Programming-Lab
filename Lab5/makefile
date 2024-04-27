all: task0.o start.o startup.o
		gcc -m32 -c task0.c -o task0.o
		ld -o task0 task0.o start.o startup.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2
                
task0.o: task0.c
		gcc -m32 -g -Wall -c task0.c -o task0.o

start.o: start.s
		nasm -f elf32 -o start.o start.s

.PHONY: clean

clean: 
		rm -f task0.o start.o
