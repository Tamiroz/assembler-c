#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define ERR 1
#define LINE_SIZE 80
#define LABEL 35
#define MAX_LABEL_LENGTH 31

typedef struct mcro {
  char label[LABEL];
  char *content;
} Macro;

/* Pre-assembler prototypes */
int pre_assemble(char *file_name);
void print_macro(Macro *m_list, int mcro_count);
int is_free_macro_label(const char *label, Macro *m_list, int mcro_count);
int find_macro_label(const char *label, Macro *m_list, int mcro_count);
int is_valid_macro_label(const char *name);

#endif /* PRE_ASSEMBLER_H */
