all: LineParser.o myshell Looper mypipe mypipeline

LineParser.o: LineParser.c
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

myshell: myshell.c
	gcc -m32 -g -Wall -o myshell myshell.c

Looper: Looper.c
	gcc -m32 -g -Wall -o  Looper  Looper.c

mypipe: mypipe.c
	gcc -m32 -g -Wall -o mypipe mypipe.c

mypipeline: mypipeline.c
	gcc -m32 -g -Wall -o mypipeline mypipeline.c

.PHONY: clean
clean:
	rm -f *.o LineParser.o myshell  Looper mypipe mypipeline
