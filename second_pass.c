/**
 * @file second_pass.c
 * @brief Implementation of the second pass of assembly processing.
 *
 * Scans code_image to complete binary encoding by looking up missing operand symbols
 * in symbol_table. Validates all .entry label definitions and records external symbol usages.
 */

#include "second_pass.h"
#include "first_pass.h"
#include <stdio.h>

/* Prototype for helper defined in first_pass.c */
void add_extern(const char *name, int address);

/**
 * @brief Performs pass 2 resolution over code_image and entry_list.
 *
 * @param filename Name of the assembly file.
 * @return int 0 on successful pass 2 execution, 1 on error.
 */
int second_pass(const char *filename) {
  int i;
  Symbol *sym_found;
  EntryNode *ent_found = entry_list;

  for (i = 0; i < code_size; i++) {
    if (code_image[i].complete == 0) {
      sym_found = find_symbol(code_image[i].symbol);
      if (sym_found == NULL) {
        printf("Error: address %d - undefined label '%s'\n",
               code_image[i].address, code_image[i].symbol);
        return 1;
      }

      if (sym_found->type == SYM_EXTERN) {
        add_extern(sym_found->name, code_image[i].address);
      }

      code_image[i].code = code_image[i].code | sym_found->value;
      code_image[i].complete = 1;
    }
  }

  while (ent_found != NULL) {
    sym_found = find_symbol(ent_found->symbol);
    if (sym_found == NULL) {
      printf("Error: line %d - entry label '%s' is not defined\n",
             ent_found->line_number, ent_found->symbol);
      return 1;
    }
    if (sym_found->type == SYM_EXTERN) {
      printf("Error: line %d - label '%s' cannot be both .entry and .extern\n",
             ent_found->line_number, ent_found->symbol);
      return 1;
    }
    ent_found->address = sym_found->value;
    ent_found = ent_found->next;
  }

  /*
  printf("------------------------------\n");
  print_code_image(code_image, code_size);
  printf("------------------------------\n");
  print_data_image(data_image, data_size);
  printf("------------------------------\n");
  print_symbol_table(symbol_table);
  printf("------------------------------\n");
  print_entry_list(entry_list);
  printf("------------------------------\n");
  print_extern_list(extern_list);
  */

  return 0;
}
