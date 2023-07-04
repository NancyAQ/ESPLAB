all:task2

task2:task2.c
	gcc -m32 -g -c task2.c -o task2.o
	ld -o task2 task2.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

clean:
	rm -f task2 task2.o
