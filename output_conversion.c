/**
 * @file output_conversion.c
 * @brief implementation for output files generation (.ob, .ent, .ext)
 */

#include "output_conversion.h"
#include "first_pass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- List Output Printing Helpers ---- */

/**
 * @brief print extern symbols recursively in chronological order
 *
 * @param extern_list head of ExternNode list
 * @param ext_file pointer to .ext file
 */
void print_externs(ExternNode *extern_list, FILE *ext_file) {
  if (extern_list == NULL)
    return;

  print_externs(extern_list->next, ext_file);
  fprintf(ext_file, "%s %04d\n", extern_list->symbol, extern_list->address);
}

/**
 * @brief print entry symbols recursively in definition order
 *
 * @param entry_list head of EntryNode list
 * @param ent_file pointer to .ent file
 */
void print_entries(EntryNode *entry_list, FILE *ent_file) {
  if (entry_list == NULL)
    return;

  print_entries(entry_list->next, ent_file);
  fprintf(ent_file, "%s %04d\n", entry_list->symbol, entry_list->address);
}

/* ---- Target Output File Generators ---- */

/**
 * @brief write code and data image into .ob object file
 *
 * @param filename name of input file
 * @return int 0 if ok, 1 if error
 */
int convert_to_object_file(char *filename) {
  FILE *ob_file;
  char file_name[MAX_LINE_LENGTH];
  int i;
  int j;
  unsigned char *code_ptr;

  /* -------- Step 1: open object file (.ob) -------- */
  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".ob");
  ob_file = fopen(file_name, "w");
  if (!ob_file) {
    printf("Error: cannot create file %s\n", file_name);
    return ERR;
  }

  /* -------- Step 2: write header line with IC and DC counts -------- */
  fprintf(ob_file, "%7d %d\n", ICF - STARTING_ADDRESS, DCF);

  /* -------- Step 3: write code section machine words -------- */
  for (i = 0; i < code_size; i++) {
    code_ptr = (unsigned char *)&(code_image[i].code);
    fprintf(ob_file, "%04d", code_image[i].address);
    for (j = 0; j < 4; j++) {
      fprintf(ob_file, " %02X", (unsigned char)code_ptr[j]);
    }
    fprintf(ob_file, "\n");
  }

  /* -------- Step 4: write data section bytes -------- */
  for (i = 0; i < data_size; i++) {
    if (i % 4 == 0)
      fprintf(ob_file, "%04d", data_image[i].address);
    fprintf(ob_file, " %02X", (unsigned char)data_image[i].value);
    if ((i + 1) % 4 == 0)
      fprintf(ob_file, "\n");
  }
  fclose(ob_file);
  return OK;
}

/**
 * @brief write .ent file if entry list is not empty
 *
 * @param filename name of file
 * @return int 0 if ok, 1 if file error
 */
int convert_to_entry_file(char *filename) {
  FILE *ent_file;
  char file_name[MAX_LINE_LENGTH];

  if (entry_list == NULL)
    return OK;

  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".ent");
  ent_file = fopen(file_name, "w");
  if (!ent_file) {
    printf("Error: cannot create entry file %s\n", file_name);
    return ERR;
  }

  print_entries(entry_list, ent_file);
  fclose(ent_file);
  return OK;
}

/**
 * @brief write .ext file if extern list is not empty
 *
 * @param filename name of file
 * @return int 0 if ok, 1 if file error
 */
int convert_to_extern_file(char *filename) {
  FILE *ext_file;
  char *temp;
  char file_name[MAX_LINE_LENGTH];

  if (extern_list == NULL)
    return OK;

  strcpy(file_name, filename);
  temp = strtok(file_name, ".");
  if (temp == NULL) {
    printf("Error: invalid filename %s\n", filename);
    return ERR;
  }

  strcat(file_name, ".ext");
  ext_file = fopen(file_name, "w");
  if (!ext_file) {
    printf("Error: cannot create extern file %s\n", file_name);
    return ERR;
  }

  print_externs(extern_list, ext_file);
  fclose(ext_file);
  return OK;
}

/* ---- Main Output Conversion Driver ---- */

/**
 * @brief driver function to generate all output files (.ob, .ent, .ext)
 *
 * @param input_filename name of input file
 * @return int 0 if ok, 1 if error
 */
int output_conversion(char *input_filename) {
  /* -------- Step 1: make .ob object file -------- */
  if (convert_to_object_file(input_filename)) {
    printf("Error: cannot create object file %s\n", input_filename);
    return ERR;
  }

  /* -------- Step 2: make .ent entry file -------- */
  if (convert_to_entry_file(input_filename)) {
    printf("Error: cannot create entry file %s\n", input_filename);
    return ERR;
  }

  /* -------- Step 3: make .ext extern file -------- */
  if (convert_to_extern_file(input_filename)) {
    printf("Error: cannot create extern file %s\n", input_filename);
    return ERR;
  }
  return OK;
}
