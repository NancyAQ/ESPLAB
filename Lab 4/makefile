all: Task4 Task1

Task4: Task4.o
	gcc -g -m32 -Wall -o Task4 Task4.o -fno-pie -fno-stack-protector

Task4.o: Task4.c
	gcc -m32 -g -Wall -c -o Task4.o Task4.c -fno-pie -fno-stack-protector
Task1: Task1.o
	gcc -g -m32 -Wall -o Task1 Task1.o
Task1.o: Task1.c
	gcc -m32 -g -Wall -c -o Task1.o Task1.c

.PHONY: clean
clean:
	rm -f *.o Task4 Task1
