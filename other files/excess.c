
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

/* Define the static table containing all your action metadata */
const Instruction instruction_table[] = {
    /* Name,  Enum,    Type,   Opcode, Funct */
    {"add", OP_ADD, TYPE_R, 0, 1},
    {"sub", OP_SUB, TYPE_R, 0, 2},
    {"and", OP_AND, TYPE_R, 0, 3},
    {"or", OP_OR, TYPE_R, 0, 4},
    {"nor", OP_NOR, TYPE_R, 0, 5},
    {"move", OP_MOVE, TYPE_R, 1, 0},
    {"mvhi", OP_MVHI, TYPE_R, 2, 0},
    {"mvlo", OP_MVLO, TYPE_R, 3, 0},
    {"addi", OP_ADDI, TYPE_I, 10, 0},
    {"subi", OP_SUBI, TYPE_I, 11, 0},
    {"andi", OP_ANDI, TYPE_I, 12, 0},
    {"ori", OP_ORI, TYPE_I, 13, 0},
    {"nori", OP_NORI, TYPE_I, 14, 0},
    {"bne", OP_BNE, TYPE_I, 15, 0},
    {"beq", OP_BEQ, TYPE_I, 16, 0},
    {"blt", OP_BLT, TYPE_I, 17, 0},
    {"bgt", OP_BGT, TYPE_I, 18, 0},
    {"lb", OP_LB, TYPE_I, 19, 0},
    {"sb", OP_SB, TYPE_I, 20, 0},
    {"lw", OP_LW, TYPE_I, 21, 0},
    {"sw", OP_SW, TYPE_I, 22, 0},
    {"lh", OP_LH, TYPE_I, 23, 0},
    {"sh", OP_SH, TYPE_I, 24, 0},
    {"jmp", OP_JMP, TYPE_J, 30, 0},
    {"la", OP_LA, TYPE_J, 31, 0},
    {"call", OP_CALL, TYPE_J, 32, 0},
    {"hlt", OP_HLT, TYPE_J, 63, 0},
    {NULL, OP_INVALID, 0, 0, 0} /* Sentinel element to mark end of table */
};

const Directive directive_table[] = {
    {".db", DIR_DB},       {".dw", DIR_DW},       {".dh", DIR_DH},
    {".asciz", DIR_ASCIZ}, {".entry", DIR_ENTRY}, {".extern", DIR_EXTERN},
    {NULL, DIR_INVALID} /* Sentinel */
};

int is_instruction(const char *word) {
  int i;
  for (i = 0; instruction_table[i].name != NULL; i++)
    if (strcmp(instruction_table[i].name, word) == 0)
      return instruction_table[i].op;
  return 0;
}

int is_directive(const char *word) {
  int i;
  for (i = 0; directive_table[i].name != NULL; i++)
    if (strcmp(directive_table[i].name, word) == 0)
      return directive_table[i].type;
  return 0;
}

int is_reserved_word(const char *word) {
  return is_instruction(word) || is_directive(word);
}

/* valid label */
int is_valid_label(const char *name) {
  int i, len;
  if (!name || !isalpha((unsigned char)name[0]))
    return 0;
  len = (int)strlen(name);
  if (len > MAX_LABEL_LENGTH)
    return 0;
  for (i = 1; i < len; i++)
    if (!isalnum((unsigned char)name[i]))
      return 0;
  return !is_reserved_word(name);
}

/*____________________first run checks___________________________________*/

/* valid opcode name */
int is_valid_opcode_name(const char *op) {
  int i;
  if (!op || !op[0])
    return 0;
  for (i = 0; op[i]; i++)
    if (!isalpha((unsigned char)op[i]))
      return 0;
  return 1;
}

/* valid register */
int is_valid_register(const char *token, int *reg_num) {
  int num, i;
  if (!token || token[0] != '$' || token[1] == '\0')
    return 0;
  for (i = 1; token[i]; i++)
    if (!isdigit((unsigned char)token[i]))
      return 0;
  num = atoi(token + 1);
  if (num < 0 || num > 31)
    return 0;
  if (reg_num)
    *reg_num = num;
  return 1;
}