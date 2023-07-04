all: myshell mypipeline

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -m32 -g -Wall -c -o myshell.o myshell.c

LineParser.o:LineParser.c LineParser.h
	gcc -m32 -g -Wall -c -o LineParser.o LineParser.c

mypipeline:mypipeline.o
	gcc -g -m32 -Wall -o mypipeline mypipeline.o

mypipeline.o:mypipeline.c
	gcc -m32 -g -Wall -c -o mypipeline.o mypipeline.c

.PHONY:clean
clean:
	rm -f *.o myshell mypipeline
