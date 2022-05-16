shell: shell_4.o
	gcc shell_4.o -o shell

shell_4.o: shell_4.c
	gcc -c shell_4.c

extra: shell_5.o
	gcc shell_5.o -o shell_extra

shell_5.o: shell_5.c
	gcc -c shell_5.c
