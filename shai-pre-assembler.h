/*
 * validation.h
 * Course: 20465 - Systems Programming Lab
 */

#ifndef VALIDATION_H
#define VALIDATION_H

#define MAX_LINE_LENGTH  82
#define MAX_LABEL_LENGTH 31

#define VALID_LINE    0
#define EMPTY_LINE    1
#define INVALID_LINE  2

/* State machine states for comment removal */
#define STATE_NORMAL     0
#define STATE_SLASH      1
#define STATE_IN_COMMENT 2
#define STATE_STAR       3

/*כאן אני צריך שתערוך לי במאקרו את המילים השמורות*/
int is_reserved_word(const char *name);

/* Cleaning */
void clean_sidelines(char *line);
void clean_inlines(char *line);

/* Validation */
int is_empty_or_comment(const char *line);
int is_valid_line_length(const char *line);
int is_valid_label(const char *name);
int is_valid_opcode_name(const char *op);
int is_valid_register(const char *token, int *reg_num);
int has_valid_commas(const char *s);
int remove_comments(char *text);

/* Extraction */
char *extract_label(char *line, char *label_out);

/* Error reporting - checks entire line and reports all errors found */
int validate_instruction_errors(const char *line, int line_number);

/* Main entry point */
int clean_line(char *line, char *label_out, int line_number);

#endif
