all:
	gcc -c -Wall -Werror -fpic module_a.c -o module_a.o
	gcc -shared -o module_a.so module_a.o
	gcc -c -Wall -Werror -fpic module_b.c -o module_b.o
	gcc -shared -o module_b.so module_b.o
	gcc -c -Wall -Werror -fpic module_c.c -o module_c.o
	gcc -shared -o module_c.so module_c.o
	gcc -Wall main.c -o main

clean:
	rm -f *.o *.so
