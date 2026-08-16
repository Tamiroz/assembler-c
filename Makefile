CC = gcc
CFLAGS = -Wall -ansi -pedantic

assembler: main.o first_pass.o pre_assembler.o second_pass.o
	$(CC) $(CFLAGS) main.o first_pass.o pre_assembler.o second_pass.o -o assembler

main.o: main.c first_pass.h pre_assembler.h second_pass.h
	$(CC) $(CFLAGS) -c main.c -o main.o

first_pass.o: first_pass.c first_pass.h
	$(CC) $(CFLAGS) -c first_pass.c -o first_pass.o

pre_assembler.o: pre_assembler.c pre_assembler.h first_pass.h
	$(CC) $(CFLAGS) -c pre_assembler.c -o pre_assembler.o

second_pass.o: second_pass.c second_pass.h first_pass.h
	$(CC) $(CFLAGS) -c second_pass.c -o second_pass.o



