/**
 * @file pre_assembler.h
 * @brief header file for macro expansion phase
 */

#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Constants & Macro Data Structures ---- */

#define OK 0
#define ERR 1
#define LINE_SIZE 80
#define LABEL 35
#define MAX_LABEL_LENGTH 31

/**
 * @struct mcro
 * @brief macro struct keeping macro label name and content body text
 */
typedef struct mcro {
  char label[LABEL];  /**< macro label name */
  char *content;      /**< allocated text body of macro */
} Macro;

/* ---- Main Pre-Assembler Pipeline ---- */

/**
 * @brief pre assemble input file by expanding all macro definitions
 *
 * @param file_name name of input file (like "file.as")
 * @return int OK (0) if pass ok, ERR (1) if error found
 */
int pre_assemble(char *file_name);

/* ---- Macro Validation & Lookup Helpers ---- */

/**
 * @brief check if macro label is not used before in macro list
 *
 * @param label macro name to check
 * @param m_list list of saved macros
 * @param mcro_count number of macros saved
 * @return int 1 if label free, 0 if already exists
 */
int is_free_macro_label(const char *label, Macro *m_list, int mcro_count);

/**
 * @brief search for macro by name in macro list
 *
 * @param label macro name to locate
 * @param m_list list of saved macros
 * @param mcro_count number of macros saved
 * @return int index in m_list if found, -1 if not found
 */
int find_macro_label(const char *label, Macro *m_list, int mcro_count);

/**
 * @brief check if macro name is legal identifier
 *
 * @param name candidate macro label
 * @return int 1 if valid, 0 if invalid (like reserved word or bad chars)
 */
int is_valid_macro_label(const char *name);

/**
 * @brief print loaded macros to stdout for debugging
 *
 * @param m_list list of saved macros
 * @param mcro_count total count of macros
 */
void print_macro(Macro *m_list, int mcro_count);

/* ---- Memory Cleanup ---- */

/**
 * @brief free all allocated memory of macro list
 *
 * @param m_list list of Macro structures
 * @param mcro_count number of macros in list
 */
void free_memory_pre_assembler(Macro *m_list, int mcro_count);

#endif /* PRE_ASSEMBLER_H */
