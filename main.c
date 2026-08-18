/**
 * @file main.c
 * @brief Main entry point for the 2-pass MIPS-style Assembler pipeline.
 *
 * Drives the compilation flow for each input assembly file provided via command-line:
 *   1. Pre-Assembler: Macro definition extraction & text expansion (.as -> .am)
 *   2. First Pass: Code/Data binary encoding & Symbol Table generation
 *   3. Second Pass: Symbol address resolution & entry/extern validation
 *   4. Output Converter: Object (.ob), Entry (.ent), and External (.ext) file generation
 */

#include "first_pass.h"
#include "output_conversion.h"
#include "pre_assembler.h"
#include "second_pass.h"

#include <stdio.h>

/**
 * @brief Main function executing assembly processing over all command-line file arguments.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings (filenames without or with extension).
 * @return int Returns 0 on completion of all processed files, or 1 if no input arguments provided.
 */
int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    printf("please provide a file name\n");
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
      free_assembler_memory();
      continue;
    }
    printf("first_pass passed for file \"%s\"\n", argv[i]);

    if (second_pass(argv[i]) == ERR) {
      printf("second_pass failed for file \"%s\"\n", argv[i]);
      free_assembler_memory();
      continue;
    }
    printf("second_pass passed for file \"%s\"\n", argv[i]);

    if (output_conversion(argv[i]) == ERR) {
      printf("output_conversion failed for file %s\n", argv[i]);
      free_assembler_memory();
      continue;
    }
    printf("output_conversion passed for file \"%s\"\n", argv[i]);

    free_assembler_memory();
  }

  return 0;
}
