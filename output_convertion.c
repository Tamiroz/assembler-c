#include "first_pass.h"
#include "output_conversion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* convert_to_object_file: convert code image to object file */
int convert_to_object_file(char *filename) {
  FILE *ob_file;
  char file_name[MAX_LINE_LENGTH];

  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".ob");
  ob_file = fopen(file_name, "w");
  if (!ob_file) {
    printf("Error: cannot create file %s\n", file_name);
    return 1;
  }

  fprintf(ob_file, "%d %d\n", ICF, DCF);
}

/* convert_to_entry_file: convert entry list to entry file */
int convert_to_entry_file(char *input_filename) {}

/* convert_to_extern_file: convert extern list to extern file */
int convert_to_extern_file(char *input_filename) {}

/* output_conversion: convert code image to object file */
int output_conversion(char *input_filename) {}