/**
 * @file second_pass.c
 * @brief second pass implementation
 *
 * scans code image array, looks up missing labels in symbol table and completes machine code words
 */

#include "second_pass.h"
#include "first_pass.h"
#include <stdio.h>

/* ---- Helper Prototype Declarations ---- */

void add_extern(const char *name, int address);

/* ---- Second Pass Resolution Engine ---- */

/**
 * @brief run second pass over code image and entry list
 *
 * @param filename name of assembly file
 * @return int 0 if pass ok, 1 if error
 */
int second_pass(const char *filename) {
  int i;
  Symbol *sym_found;
  EntryNode *ent_found = entry_list;

  /* -------- Step 1: fill missing symbol addresses in code image -------- */
  for (i = 0; i < code_size; i++) {
    if (code_image[i].complete == 0) {
      sym_found = find_symbol(code_image[i].symbol);
      if (sym_found == NULL) {
        printf("Error: address %d - undefined label '%s'\n",
               code_image[i].address, code_image[i].symbol);
        return 1;
      }

      /* check if symbol is extern, add to extern list */
      if (sym_found->type == SYM_EXTERN) {
        add_extern(sym_found->name, code_image[i].address);
      }

      /* add symbol value to instruction code word */
      code_image[i].code = code_image[i].code | sym_found->value;
      code_image[i].complete = 1;
    }
  }

  /* -------- Step 2: check entry labels are defined and get their addresses -------- */
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

  return 0;
}
