/**
 * @file main.c
 * @brief main file for assembler project
 *
 * runs all steps for files given in cmd args:
 *   1. pre-assembler: expand macros in .as to get .am file
 *   2. first pass: build symbol table and encode instructions & data
 *   3. second pass: fix label addresses and check entry/extern
 *   4. output files: create .ob, .ent and .ext files
 */

#include "first_pass.h"
#include "output_conversion.h"
#include "pre_assembler.h"
#include "second_pass.h"

#include <stdio.h>

/**
 * @brief main function running assembler pipeline over files passed in argv
 *
 * @param argc amount of arguments
 * @param argv list of file names
 * @return int 0 if done all files ok, 1 if no file names passed
 */
int main(int argc, char *argv[]) {
  int i;
  int file_error;

  /* -------- Step 1: check command line args -------- */
  if (argc < 2) {
    printf("please provide a file name\n");
    return ERR;
  }

  /* -------- Step 2: process files one by one -------- */
  for (i = 1; i < argc; i++) {
    file_error = OK;

    /* Step 2.1: pre-assembler macro expansion */
    if (pre_assemble(argv[i]) == ERR) {
      printf("pre_assembler failed for file \"%s\"\n", argv[i]);
      continue;
    }
    printf("pre_assembler passed for file \"%s\"\n", argv[i]);

    /* Step 2.2: first pass (encoding and build symbols) */
    if (first_pass(argv[i]) == ERR) {
      printf("first_pass failed for file \"%s\"\n", argv[i]);
      file_error = ERR;
    } else {
      printf("first_pass passed for file \"%s\"\n", argv[i]);
    }

    /* Step 2.3: second pass (complete label addresses) */
    if (second_pass(argv[i]) == ERR) {
      printf("second_pass failed for file \"%s\"\n", argv[i]);
      file_error = ERR;
    } else {
      printf("second_pass passed for file \"%s\"\n", argv[i]);
    }

    /* Step 2.4: write output files (.ob, .ent, .ext) if no errors */
    if (!file_error) {
      if (output_conversion(argv[i]) == ERR) {
        printf("output_conversion failed for file %s\n", argv[i]);
      } else {
        printf("output_conversion passed for file \"%s\"\n", argv[i]);
      }
    }

    /* Step 2.5: free allocated memory for file */
    free_assembler_memory();
  }

  return OK;
}
