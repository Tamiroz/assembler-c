pre_assembler: pre_assembler.o error_check.o
	gcc -Wall -ansi -pedantic pre_assembler.o error_check.o -o pre_assembler

pre_assembler.o: pre_assembler.c pre_assembler.h error_check.h globals.h
	gcc -Wall -ansi -pedantic -c pre_assembler.c -o pre_assembler.o

error_check.o: error_check.c error_check.h pre_assembler.h globals.h
	gcc -Wall -ansi -pedantic -c error_check.c -o error_check.o

