/**
 * @file pre_assembler.h
 * @brief Header file for macro pre-processing phase of the assembler.
 */

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

/**
 * @struct mcro
 * @brief Represents a macro definition containing its name and expanded body.
 */
typedef struct mcro {
  char label[LABEL];  /**< Macro label/name */
  char *content;      /**< Dynamically allocated body text of the macro */
} Macro;

/* ---- Pre-assembler prototypes ---- */

/**
 * @brief Pre-assembles an input file by expanding all macro definitions.
 *
 * @param file_name Name of the input file (e.g., "filename.as").
 * @return int OK (0) on successful expansion, ERR (1) on syntax error or I/O failure.
 */
int pre_assemble(char *file_name);

/**
 * @brief Prints all loaded macro labels and contents to stdout for debugging.
 *
 * @param m_list Array of stored Macro structures.
 * @param mcro_count Total number of stored macros.
 */
void print_macro(Macro *m_list, int mcro_count);

/**
 * @brief Checks whether a candidate macro label has not been defined previously.
 *
 * @param label The macro name to check.
 * @param m_list Array of existing Macro structures.
 * @param mcro_count Number of existing macros.
 * @return int 1 if label is unique/free, 0 if it already exists.
 */
int is_free_macro_label(const char *label, Macro *m_list, int mcro_count);

/**
 * @brief Searches for a macro by name in the loaded macro list.
 *
 * @param label Name of the macro to locate.
 * @param m_list Array of existing Macro structures.
 * @param mcro_count Number of existing macros.
 * @return int Index in m_list if found, -1 if not found.
 */
int find_macro_label(const char *label, Macro *m_list, int mcro_count);

/**
 * @brief Validates whether a proposed macro name is a legal identifier.
 *
 * @param name The macro label candidate.
 * @return int 1 if valid, 0 if invalid (e.g. reserved word or illegal characters).
 */
int is_valid_macro_label(const char *name);

/**
 * @brief Frees all dynamically allocated memory associated with the macro list.
 *
 * @param m_list Array of Macro structures.
 * @param mcro_count Number of macros in the array.
 */
void free_memory_pre_assembler(Macro *m_list, int mcro_count);

#endif /* PRE_ASSEMBLER_H */
