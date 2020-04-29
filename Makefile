all:
	gcc -c process_control.c
	gcc -c scheduler.c
	gcc -c main.c
	gcc main.o scheduler.o process_control.o -o main
