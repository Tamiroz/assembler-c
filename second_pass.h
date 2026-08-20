/**
 * @file second_pass.h
 * @brief header file for second pass of assembler
 */

#ifndef SECOND_PASS_H
#define SECOND_PASS_H

/* ---- Second Pass Main Pipeline ---- */

/**
 * @brief runs second pass to complete label addresses in code image and check entry/extern
 *
 * @param filename name of assembly file
 * @return int 0 if pass ok, 1 if error found
 */
int second_pass(const char *filename);

#endif
