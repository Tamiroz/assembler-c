/**
 * @file output_conversion.c
 * @brief Implementation of target file generation (.ob, .ent, .ext).
 */

#include "output_conversion.h"
#include "first_pass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Recursively prints external symbol occurrences in chronological order.
 *
 * @param extern_list Pointer to head of the ExternNode linked list.
 * @param ext_file Pointer to open .ext output file handle.
 */
void print_externs(ExternNode *extern_list, FILE *ext_file) {
  if (extern_list == NULL)
    return;

  print_externs(extern_list->next, ext_file);
  fprintf(ext_file, "%s %04d\n", extern_list->symbol, extern_list->address);
}

/**
 * @brief Iterates and prints entry symbols and their resolved addresses.
 *
 * @param entry_list Pointer to head of the EntryNode linked list.
 * @param ent_file Pointer to open .ent output file handle.
 */
void print_entries(EntryNode *entry_list, FILE *ent_file) {
  while (entry_list != NULL) {
    fprintf(ent_file, "%s %04d\n", entry_list->symbol, entry_list->address);
    entry_list = entry_list->next;
  }
}

/**
 * @brief Formats and writes instruction and data images into the .ob object file.
 *
 * @param filename Name of the file being processed.
 * @return int 0 on success, 1 on file creation error.
 */
int convert_to_object_file(char *filename) {
  FILE *ob_file;
  char file_name[MAX_LINE_LENGTH];
  int i;
  int j;
  unsigned char *code_ptr;

  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".ob");
  ob_file = fopen(file_name, "w");
  if (!ob_file) {
    printf("Error: cannot create file %s\n", file_name);
    return 1;
  }

  fprintf(ob_file, "%6d %d\n", ICF - STARTING_ADDRESS, DCF);
  for (i = 0; i < code_size; i++) {
    code_ptr = (unsigned char *)&(code_image[i].code);
    fprintf(ob_file, "%d", code_image[i].address);
    for (j = 0; j < 4; j++) {
      fprintf(ob_file, " %02X", (unsigned char)code_ptr[j]);
    }
    fprintf(ob_file, "\n");
  }
  for (i = 0; i < data_size; i++) {
    if (i % 4 == 0)
      fprintf(ob_file, "%d", data_image[i].address);
    fprintf(ob_file, " %02X", (unsigned char)data_image[i].value);
    if ((i + 1) % 4 == 0)
      fprintf(ob_file, "\n");
  }
  fclose(ob_file);
  return 0;
}

/* convert_to_entry_file: convert entry list to entry file*/
int convert_to_entry_file(char *filename) {
  FILE *ent_file;
  char file_name[MAX_LINE_LENGTH];

  if (entry_list == NULL)
    return 0;

  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".ent");
  ent_file = fopen(file_name, "w");
  if (!ent_file) {
    printf("Error: cannot create entry file %s\n", file_name);
    return 1;
  }

  print_entries(entry_list, ent_file);
  fclose(ent_file);
  return 0;
}

/* convert_to_extern_file: convert extern list to extern file*/
int convert_to_extern_file(char *filename) {
  FILE *ext_file;
  char *temp;
  char file_name[MAX_LINE_LENGTH];

  if (extern_list == NULL)
    return 0;

  strcpy(file_name, filename);
  temp = strtok(file_name, ".");
  if (temp == NULL) {
    printf("Error: invalid filename %s\n", filename);
    return 1;
  }

  strcat(file_name, ".ext");
  ext_file = fopen(file_name, "w");
  if (!ext_file) {
    printf("Error: cannot create extern file %s\n", file_name);
    return 1;
  }

  print_externs(extern_list, ext_file);
  fclose(ext_file);
  return 0;
}

/* output_conversion: convert code image to object file */
int output_conversion(char *input_filename) {
  if (convert_to_object_file(input_filename)) {
    printf("Error: cannot create object file %s\n", input_filename);
    return 1;
  }

  if (convert_to_entry_file(input_filename)) {
    printf("Error: cannot create entry file %s\n", input_filename);
    return 1;
  }

  if (convert_to_extern_file(input_filename)) {
    printf("Error: cannot create extern file %s\n", input_filename);
    return 1;
  }
  return 0;
}
