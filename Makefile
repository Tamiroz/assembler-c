CC = gcc
CFLAGS = -Wall -ansi -pedantic

assembler: main.o first_pass.o pre_assembler.o 
	$(CC) $(CFLAGS) main.o first_pass.o pre_assembler.o -o assembler

main.o: main.c first_pass.h pre_assembler.h 
	$(CC) $(CFLAGS) -c main.c -o main.o

first_pass.o: first_pass.c first_pass.h
	$(CC) $(CFLAGS) -c first_pass.c -o first_pass.o

pre_assembler.o: pre_assembler.c pre_assembler.h first_pass.h
	$(CC) $(CFLAGS) -c pre_assembler.c -o pre_assembler.o



