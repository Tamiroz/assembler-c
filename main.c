#include "first_pass.h"
#include "pre_assembler.h"
#include "second_pass.h"

#include <stdio.h>
/*#include "second_pass.h"
#include "output_converter.h"*/

/*main function recives a call from user with argv as ann array of file names
to be run through the assembler.
for each fo the files it will pass it through the loop -
pre_assembler -> first_pass -> second_pass -> output_converter
each function will get the file name as a parameter.
*/
/*input file is in the format - name.as*/

/*output file is in the format - name.am, name.ob, name.ent, name.ext*/

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    printf("please provide a file name");
    return 1;
  }
  for (i = 1; i < argc; i++) {
    if (pre_assemble(argv[i]) == ERR) {
      printf("pre_assembler failed for file \"%s\"\n", argv[i]);
      continue;
    }
    printf("pre_assembler passed for file \"%s\"\n", argv[i]);

    if (first_pass(argv[i]) == ERR) {
      printf("first_pass failed for file \"%s\"\n", argv[i]);
    } else
      printf("first_pass passed for file \"%s\"\n", argv[i]);

    if (second_pass(argv[i]) == ERR) {
      printf("second_pass failed for file \"%s\"\n", argv[i]);
      continue;
    }
    printf("second_pass passed for file \"%s\"\n", argv[i]);

    /*
    if (output_converter(argv[i]) == ERR) { *are there possible faliures for
    conversion proccess?* printf("output_converter failed for file %s\n",
      argv[i]); continue;
    }*/

    /*free resources in each iterations, maybe nest stages*/
  }

  return 0;
}
