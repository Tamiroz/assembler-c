/**
 * @file second_pass.h
 * @brief Header file for the second pass of the assembler.
 */

#ifndef SECOND_PASS_H
#define SECOND_PASS_H

/**
 * @brief Executes the second pass of assembly processing.
 *
 * Resolves missing label addresses in code_image instructions using symbol_table,
 * validates entry directives in entry_list, and tracks external symbol references.
 *
 * @param filename Name of the file being processed.
 * @return int 0 on success, 1 on error (undefined labels or conflicting declarations).
 */
int second_pass(const char *filename);

#endif
