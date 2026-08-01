#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include "globals.h"

#define LINE_SIZE MAX_LINE_LENGTH
#define LABEL 35

typedef struct mcro {
  char label[LABEL];
  char *content;
} Macro;

/* Pre-assembler prototypes */
int pre_assemble(char *file_name);
void print_macro(Macro *m_list, int mcro_count);
int is_repeated_label(const char *label, Macro *m_list, int mcro_count);
int find_label(const char *label, Macro *m_list, int mcro_count);

#endif /* PRE_ASSEMBLER_H */
