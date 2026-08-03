#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#define OK 1
#define ERR 0
#define LINE_SIZE 80
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
