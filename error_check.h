#ifndef ERROR_CHECK_H
#define ERROR_CHECK_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 80
#define MAX_LABEL_LENGTH 30

typedef enum { TYPE_R, TYPE_I, TYPE_J } InstructionType;

typedef enum {
  OP_ADD,
  OP_SUB,
  OP_AND,
  OP_OR,
  OP_NOR,
  OP_MOVE,
  OP_MVHI,
  OP_MVLO,
  OP_ADDI,
  OP_SUBI,
  OP_ANDI,
  OP_ORI,
  OP_NORI,
  OP_BNE,
  OP_BEQ,
  OP_BLT,
  OP_BGT,
  OP_LB,
  OP_SB,
  OP_LW,
  OP_SW,
  OP_LH,
  OP_SH,
  OP_JMP,
  OP_LA,
  OP_CALL,
  OP_HLT,
  OP_INVALID = -1
} Opcode;

typedef struct {
  const char *name;
  Opcode op;
  InstructionType type;
  int opcode_val;
  int funct_val;
} Instruction;

typedef enum {
  DIR_DB,
  DIR_DW,
  DIR_DH,
  DIR_ASCIZ,
  DIR_ENTRY,
  DIR_EXTERN,
  DIR_INVALID = -1
} DirectiveType;

typedef struct {
  const char *name;   /* e.g. ".db" */
  DirectiveType type; /* e.g. DIR_DB */
} Directive;

/* External table declarations */
extern const Instruction instruction_table[];
extern const Directive directive_table[];

/* Validation function prototypes */
int is_instruction(const char *word);
int is_directive(const char *word);
int is_reserved_word(const char *word);
int is_valid_label(const char *name);
int is_valid_opcode_name(const char *op);
int is_valid_register(const char *token, int *reg_num);

#endif /* ERROR_CHECK_H */
