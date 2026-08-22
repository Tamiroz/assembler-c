/**
 * @file first_pass.c
 * @brief Implementation of Pass 1 of the 2-pass Assembler.
 *
 * Reads expanded assembly code (.am line by line), parses instruction opcodes
 * and directives, constructs code_image and data_image, builds symbol_table,
 * and computes final memory offsets.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "first_pass.h"

/* ---- Reserved Keywords & Opcode Lookup Tables ---- */

static const char *reserved_words[] = {
    "add",  "sub",  "and",    "or",     "nor",     "move", "mvhi",
    "mvlo", "addi", "subi",   "andi",   "ori",     "nori", "bne",
    "beq",  "blt",  "bgt",    "lb",     "sb",      "lw",   "sw",
    "lh",   "sh",   "jmp",    "la",     "call",    "hlt",  ".db",
    ".dh",  ".dw",  ".asciz", ".entry", ".extern", NULL};

/**
 * @brief Checks if a word is a reserved assembly instruction opcode or
 * directive.
 * @param word The candidate string to evaluate.
 * @return int TRUE if reserved word, FALSE otherwise.
 */
int is_reserved_word(const char *word) {
  int i = 0;
  while (reserved_words[i] != NULL) {
    if (strcmp(reserved_words[i], word) == STR_MATCH)
      return TRUE;
    i++;
  }
  return FALSE;
}

typedef struct {
  char name[NAME_SIZE];
  int opcode;
  int funct;
} RInstruction;

typedef struct {
  char name[NAME_SIZE];
  int opcode;
} IInstruction;

typedef struct {
  char name[NAME_SIZE];
  int opcode;
} JInstruction;

static const RInstruction r_instructions[] = {
    {"add", 0, 1},  {"sub", 0, 2},  {"and", 0, 3},  {"or", 0, 4}, {"nor", 0, 5},
    {"move", 1, 1}, {"mvhi", 1, 2}, {"mvlo", 1, 3}, {"", 0, 0}};

static const IInstruction i_instructions[] = {
    {"addi", 10}, {"subi", 11}, {"andi", 12}, {"ori", 13},
    {"nori", 14}, {"bne", 15},  {"beq", 16},  {"blt", 17},
    {"bgt", 18},  {"lb", 19},   {"sb", 20},   {"lw", 21},
    {"sw", 22},   {"lh", 23},   {"sh", 24},   {"", 0}};

static const JInstruction j_instructions[] = {
    {"jmp", 30}, {"la", 31}, {"call", 32}, {"hlt", 63}, {"", 0}};

/* ---- Global Assembler Memory State ---- */

int ICF = 0;
int DCF = 0;
CodeEntry *code_image = NULL;
int code_size = 0;
int code_capacity = 0;
DataEntry *data_image = NULL;
int data_size = 0;
int data_capacity = 0;
Symbol *symbol_table = NULL;
EntryNode *entry_list = NULL;
ExternNode *extern_list = NULL;

/* ---- Extern List Operations ---- */

/**
 * @brief Adds a new external reference node to extern_list.
 * @param name External label name.
 * @param address Address of instruction referencing external symbol.
 */
void add_extern(const char *name, int address) {
  ExternNode *ext = (ExternNode *)malloc(sizeof(ExternNode));
  strncpy(ext->symbol, name, MAX_LABEL_LENGTH);
  ext->symbol[MAX_LABEL_LENGTH] = '\0';
  ext->address = address;
  ext->next = extern_list;
  extern_list = ext;
}

/* ---- String & Token Parsing Helpers ---- */

/**
 * @brief Advances string index past leading spaces and tabs.
 * @param line Input line buffer.
 * @param i Pointer to current index in line.
 */
void skip_spaces(char *line, int *i) {
  while (line[*i] == ' ' || line[*i] == '\t')
    (*i)++;
}

/**
 * @brief Extracts the next space/tab-delimited word token from line.
 * @param line Input line buffer.
 * @param i Pointer to current index in line.
 * @param word Buffer to store extracted word token.
 */
void get_next_word(char *line, int *i, char *word) {
  int j = 0;
  while (line[*i] != '\0' && line[*i] != ' ' && line[*i] != '\t' &&
         line[*i] != '\n' && line[*i] != '\r')
    word[j++] = line[(*i)++];
  word[j] = '\0';
}

/**
 * @brief Extracts the next operand token handling optional comma delimiters.
 * @param line Input line buffer.
 * @param i Pointer to current index in line.
 * @param token Buffer to store extracted token.
 */
void get_next_token(char *line, int *i, char *token) {
  int j = 0;
  skip_spaces(line, i);
  if (line[*i] == ',') {
    (*i)++;
    skip_spaces(line, i);
  }
  while (line[*i] != '\0' && line[*i] != ' ' && line[*i] != '\t' &&
         line[*i] != '\n' && line[*i] != '\r' && line[*i] != ',')
    token[j++] = line[(*i)++];
  token[j] = '\0';
}

/**
 * @brief Trims leading and trailing whitespace characters in place.
 * @param line Input line buffer.
 */
void trim_whitespace(char *line) {
  int start = 0;
  int end;
  int len;
  int i;

  while (line[start] == ' ' || line[start] == '\t')
    start++;

  if (start > 0) {
    len = (int)strlen(line) - start;
    for (i = 0; i <= len; i++)
      line[i] = line[start + i];
  }

  end = (int)strlen(line) - 1;
  while (end >= 0 && (line[end] == ' ' || line[end] == '\t' ||
                      line[end] == '\n' || line[end] == '\r'))
    end--;
  line[end + 1] = '\0';
}

/* ---- Symbol Table Operations ---- */

/**
 * @brief Allocates and prepends a new symbol node to symbol_table.
 * @param name Symbol label string.
 * @param value Address or offset value.
 * @param type Category (SYM_CODE, SYM_DATA, SYM_EXTERN).
 */
void add_symbol(const char *name, int value, SymbolType type) {
  Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
  strncpy(sym->name, name, MAX_LABEL_LENGTH);
  sym->name[MAX_LABEL_LENGTH] = '\0';
  sym->value = value;
  sym->type = type;
  sym->next = symbol_table;
  symbol_table = sym;
}

/**
 * @brief Searches symbol_table for an existing symbol node by name.
 * @param name Symbol name to find.
 * @return Symbol* Pointer to node if found, NULL otherwise.
 */
Symbol *find_symbol(const char *name) {
  Symbol *cur = symbol_table;
  while (cur) {
    if (strcmp(cur->name, name) == STR_MATCH)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

/* ---- Entry List Operations ---- */

/**
 * @brief Allocates and prepends a new entry node to entry_list.
 * @param name Entry label identifier.
 * @param line_number Line number where declared.
 */
void add_entry(const char *name, int line_number) {
  EntryNode *ent = (EntryNode *)malloc(sizeof(EntryNode));
  strncpy(ent->symbol, name, MAX_LABEL_LENGTH);
  ent->symbol[MAX_LABEL_LENGTH] = '\0';
  ent->line_number = line_number;
  ent->next = entry_list;
  entry_list = ent;
}

/**
 * @brief Searches entry_list for an existing entry node by symbol name.
 * @param name Entry label to find.
 * @return EntryNode* Pointer to node if found, NULL otherwise.
 */
EntryNode *find_entry(const char *name) {
  EntryNode *cur = entry_list;
  while (cur) {
    if (strcmp(cur->symbol, name) == 0)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

/* ---- Code & Data Image Memory Management ---- */

/**
 * @brief Appends an encoded instruction word to the dynamic code_image array.
 * @param address Instruction memory address.
 * @param code 32-bit encoded instruction word.
 * @param complete 1 if instruction encoding is full, 0 if label resolution
 * needed in Pass 2.
 * @param symbol Missing operand label string if complete == 0.
 */
void add_to_code_image(int address, unsigned int code, int complete,
                       const char *symbol) {
  if (code_size >= code_capacity) {
    code_capacity = code_capacity == 0 ? 100 : code_capacity * 2;
    code_image =
        (CodeEntry *)realloc(code_image, sizeof(CodeEntry) * code_capacity);
  }
  code_image[code_size].address = address;
  code_image[code_size].code = code;
  code_image[code_size].complete = complete;
  if (symbol)
    strncpy(code_image[code_size].symbol, symbol, MAX_LABEL_LENGTH);
  else
    code_image[code_size].symbol[0] = '\0';
  code_size++;
}

/**
 * @brief Appends a data byte to the dynamic data_image array.
 * @param address Data memory address.
 * @param value Byte value to store.
 */
void add_to_data_image(int address, unsigned char value) {
  if (data_size >= data_capacity) {
    data_capacity = data_capacity == 0 ? 100 : data_capacity * 2;
    data_image =
        (DataEntry *)realloc(data_image, sizeof(DataEntry) * data_capacity);
  }
  data_image[data_size].address = address;
  data_image[data_size].value = value;
  data_size++;
}

/* ---- Line Classification & Syntax Identification ---- */

/**
 * @brief Advances index past label declaration prefix if present.
 * @param line Input line buffer.
 * @param i Pointer to current index in line.
 */
void skip_label(char *line, int *i) {
  int j = 0;
  char first_word[MAX_LABEL_LENGTH + 1];

  skip_spaces(line, &j);
  get_next_word(line, &j, first_word);

  if (first_word[0] != '\0' && first_word[strlen(first_word) - 1] == ':') {
    *i = j;
    return;
  }

  skip_spaces(line, &j);
  if (line[j] == ':') {
    *i = j + 1;
    return;
  }

  if (strcmp(first_word, "END") == 0) {
    *i = j;
    return;
  }

  *i = 0;
}

/**
 * @brief Determines whether line contains a valid label definition.
 * @param line Input line buffer.
 * @param label Buffer to store extracted label name.
 * @return int 1 if label definition found, 0 otherwise.
 */
/**
 * @brief Determines whether line contains a valid label definition.
 * @param line Input line buffer.
 * @param label Buffer to store extracted label name.
 * @return int TRUE if label definition found, FALSE otherwise.
 */
int is_label(char *line, char *label) {
  int i = 0;
  char first_word[MAX_LABEL_LENGTH + 1];
  int len;

  get_next_word(line, &i, first_word);
  len = (int)strlen(first_word);

  if (len > 0 && first_word[len - 1] == ':') {
    first_word[len - 1] = '\0';
    strcpy(label, first_word);
    if (!isalpha((unsigned char)label[0])) {
      label[0] = '\0';
      return FALSE;
    }
    return TRUE;
  }

  skip_spaces(line, &i);
  if (line[i] == ':') {
    strcpy(label, first_word);
    if (!isalpha((unsigned char)label[0])) {
      label[0] = '\0';
      return FALSE;
    }
    return TRUE;
  }

  if (strcmp(first_word, "END") == STR_MATCH) {
    strcpy(label, "END");
    return TRUE;
  }

  label[0] = '\0';
  return FALSE;
}

int is_end(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  return (strcmp(word, "END") == STR_MATCH);
}

/**
 * @brief Checks if a line is blank or contains a comment starting with ';'.
 * @param line Input line buffer.
 * @return int TRUE if empty or comment, FALSE otherwise.
 */
int is_empty_or_comment(char *line) {
  int i = 0;
  skip_spaces(line, &i);
  return (line[i] == '\0' || line[i] == '\n' || line[i] == ';');
}

/**
 * @brief Checks if a line contains an instruction opcode statement.
 * @param line Input line buffer.
 * @return int TRUE if instruction statement, FALSE otherwise.
 */
int is_instruction(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  if (word[0] == '.')
    return FALSE;

  return is_reserved_word(word);
}

/**
 * @brief Checks if a line contains a data directive (.db, .dh, .dw, .asciz).
 * @param line Input line buffer.
 * @return int TRUE if data directive, FALSE otherwise.
 */
int is_data_directive(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  if (strcmp(word, ".db") == STR_MATCH)
    return TRUE;
  if (strcmp(word, ".dh") == STR_MATCH)
    return TRUE;
  if (strcmp(word, ".dw") == STR_MATCH)
    return TRUE;
  if (strcmp(word, ".asciz") == STR_MATCH)
    return TRUE;

  return FALSE;
}

/**
 * @brief Checks if a line contains an .extern directive.
 * @param line Input line buffer.
 * @return int TRUE if .extern directive, FALSE otherwise.
 */
int is_extern(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  return (strcmp(word, ".extern") == STR_MATCH);
}

/**
 * @brief Checks if a line contains an .entry directive.
 * @param line Input line buffer.
 * @return int TRUE if .entry directive, FALSE otherwise.
 */
int is_entry(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  return (strcmp(word, ".entry") == STR_MATCH);
}

/* ---- Binary Instruction & Data Encoding ---- */

/**
 * @brief Extracts numerical register index from a '$' prefixed token.
 * @param token Register token string (e.g. "$5").
 * @return int Register number (0-31), or -1 if malformed.
 */
static int get_register(const char *token) {
  int val;
  char *endptr;
  if (!token || token[0] != '$' || token[1] == '\0')
    return -1;
  val = (int)strtol(token + 1, &endptr, 10);
  if (*endptr != '\0' || val < 0 || val > 31)
    return -1;
  return val;
}

/**
 * @brief Validates 16-bit signed integer immediate value.
 */
static int is_valid_immediate(const char *token, long *val_out) {
  char *endptr;
  long val;
  if (!token || token[0] == '\0')
    return FALSE;
  val = strtol(token, &endptr, 10);
  if (*endptr != '\0' || val < -32768 || val > 32767)
    return FALSE;
  if (val_out)
    *val_out = val;
  return TRUE;
}

/**
 * @brief Encodes an R-type instruction word.
 * @param line Input instruction line buffer.
 * @return unsigned int 32-bit encoded R-type machine word.
 */
/**
 * @brief Validates comma syntax in an instruction statement.
 */
static int validate_comma_syntax(char *line, int expected_ops, int line_number,
                                 int *error_found) {
  int i = 0;
  int comma_count = 0;
  int token_count = 0;
  int in_token = 0;
  int last_was_comma = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  skip_spaces(line, &i);

  if (expected_ops == 0) {
    if (line[i] != '\0' && line[i] != '\n' && line[i] != ';') {
      printf("Error: line %d - extra text after '%s' instruction\n",
             line_number, word);
      *error_found = 1;
      return ERR;
    }
    return OK;
  }

  if (line[i] == ',') {
    printf("Error: line %d - leading comma before operands in '%s'\n",
           line_number, word);
    *error_found = 1;
    return ERR;
  }

  while (line[i] != '\0' && line[i] != '\n' && line[i] != ';') {
    if (line[i] == ' ' || line[i] == '\t' || line[i] == '\r') {
      in_token = 0;
      i++;
      continue;
    }

    if (line[i] == ',') {
      if (last_was_comma || token_count == 0) {
        printf("Error: line %d - invalid or consecutive comma in '%s'\n",
               line_number, word);
        *error_found = 1;
        return ERR;
      }
      comma_count++;
      last_was_comma = 1;
      in_token = 0;
      i++;
      continue;
    }

    if (!in_token) {
      if (token_count > 0 && !last_was_comma) {
        printf("Error: line %d - missing comma between operands in '%s'\n",
               line_number, word);
        *error_found = 1;
        return ERR;
      }
      token_count++;
      in_token = 1;
      last_was_comma = 0;
    }
    i++;
  }

  if (last_was_comma) {
    printf("Error: line %d - comma at end of '%s'\n", line_number, word);
    *error_found = 1;
    return ERR;
  }

  if (token_count > 0 && comma_count != token_count - 1) {
    printf("Error: line %d - comma syntax error in '%s'\n", line_number, word);
    *error_found = 1;
    return ERR;
  }

  return OK;
}

/**
 * @brief Encodes an R-type instruction word.
 * @param line Input instruction line buffer.
 * @param line_number Line number in source file.
 * @param error_found Pointer to error flag.
 * @return unsigned int 32-bit encoded R-type machine word.
 */
static unsigned int encode_r(char *line, int line_number, int *error_found) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];
  char op1[NAME_SIZE], op2[NAME_SIZE], op3[NAME_SIZE];
  int opcode = 0, funct = 0;
  int rs = 0, rt = 0, rd = 0;
  unsigned int code = 0;

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  for (j = 0; r_instructions[j].name[0] != '\0'; j++) {
    if (strcmp(r_instructions[j].name, word) == 0) {
      opcode = r_instructions[j].opcode;
      funct = r_instructions[j].funct;
      break;
    }
  }

  if (validate_comma_syntax(line, (opcode == 1 ? 2 : 3), line_number,
                            error_found) == ERR)
    return 0;

  skip_spaces(line, &i);

  if (opcode == 1) {
    get_next_token(line, &i, op1);
    get_next_token(line, &i, op2);

    if (op1[0] == '\0' || op2[0] == '\0') {
      printf("Error: line %d - instruction '%s' missing operand\n", line_number,
             word);
      *error_found = 1;
      return 0;
    }
    rd = get_register(op1);
    rs = get_register(op2);
    if (rd == -1 || rs == -1) {
      printf("Error: line %d - instruction '%s' invalid register operand\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
    rt = 0;
  } else {
    get_next_token(line, &i, op1);
    get_next_token(line, &i, op2);
    get_next_token(line, &i, op3);

    if (op1[0] == '\0' || op2[0] == '\0' || op3[0] == '\0') {
      printf("Error: line %d - instruction '%s' missing operand\n", line_number,
             word);
      *error_found = 1;
      return 0;
    }
    rs = get_register(op1);
    rt = get_register(op2);
    rd = get_register(op3);
    if (rs == -1 || rt == -1 || rd == -1) {
      printf("Error: line %d - instruction '%s' invalid register operand\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
  }

  code |= (unsigned int)(opcode & 0x3F) << 26;
  code |= (unsigned int)(rs & 0x1F) << 21;
  code |= (unsigned int)(rt & 0x1F) << 16;
  code |= (unsigned int)(rd & 0x1F) << 11;
  code |= (unsigned int)(funct & 0x1F) << 6;

  return code;
}

/**
 * @brief Encodes an I-type instruction word.
 * @param line Input instruction line buffer.
 * @param IC Current instruction counter address.
 * @param complete Output flag (1 if resolved, 0 if label operand needs Pass 2).
 * @param sym_out Output buffer for missing operand symbol name if complete ==
 * 0.
 * @param line_number Line number in source file.
 * @param error_found Pointer to error flag.
 * @return unsigned int 32-bit encoded I-type machine word.
 */
static unsigned int encode_i(char *line, int IC, int *complete, char *sym_out,
                             int line_number, int *error_found) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];
  char op1[MAX_LABEL_LENGTH + 1];
  char op2[MAX_LABEL_LENGTH + 1];
  char op3[MAX_LABEL_LENGTH + 1];
  int opcode = 0;
  int rs = 0, rt = 0;
  long immed = 0;
  unsigned int code = 0;
  int is_branch = 0;

  (void)IC;

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  for (j = 0; i_instructions[j].name[0] != '\0'; j++) {
    if (strcmp(i_instructions[j].name, word) == 0) {
      opcode = i_instructions[j].opcode;
      break;
    }
  }

  if (validate_comma_syntax(line, 3, line_number, error_found) == ERR)
    return 0;

  is_branch = (opcode >= 15 && opcode <= 18);

  get_next_token(line, &i, op1);
  get_next_token(line, &i, op2);
  get_next_token(line, &i, op3);

  if (op1[0] == '\0' || op2[0] == '\0' || op3[0] == '\0') {
    printf("Error: line %d - instruction '%s' missing operand\n", line_number,
           word);
    *error_found = 1;
    return 0;
  }

  if (is_branch) {
    if (op3[0] == '$') {
      printf("Error: line %d - branch instruction '%s' requires label operand, "
             "register given\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
    rs = get_register(op1);
    rt = get_register(op2);
    if (rs == -1 || rt == -1) {
      printf(
          "Error: line %d - branch instruction '%s' invalid register operand\n",
          line_number, word);
      *error_found = 1;
      return 0;
    }
    immed = 0;
    *complete = INCOMPLETE;
    strncpy(sym_out, op3, MAX_LABEL_LENGTH);
    sym_out[MAX_LABEL_LENGTH] = '\0';
  } else {
    rs = get_register(op1);
    rt = get_register(op3);
    if (rs == -1 || rt == -1) {
      printf("Error: line %d - instruction '%s' invalid register operand\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
    if (!is_valid_immediate(op2, &immed)) {
      printf("Error: line %d - instruction '%s' invalid 16-bit immediate '%s'\n",
             line_number, word, op2);
      *error_found = 1;
      return 0;
    }
    *complete = COMPLETE;
    sym_out[0] = '\0';
  }

  code |= (unsigned int)(opcode & 0x3F) << 26;
  code |= (unsigned int)(rs & 0x1F) << 21;
  code |= (unsigned int)(rt & 0x1F) << 16;
  code |= (unsigned int)((short)immed & 0xFFFF);

  return code;
}

/**
 * @brief Encodes a J-type instruction word.
 * @param line Input instruction line buffer.
 * @param complete Output flag (1 if resolved, 0 if label operand needs Pass 2).
 * @param sym_out Output buffer for missing operand symbol name if complete ==
 * 0.
 * @param line_number Line number in source file for error reporting.
 * @param error_found Pointer to error flag in first_pass.
 * @return unsigned int 32-bit encoded J-type machine word.
 */
static unsigned int encode_j(char *line, int *complete, char *sym_out,
                             int line_number, int *error_found) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];
  char op1[MAX_LABEL_LENGTH + 1];
  int opcode = 0;
  int reg = 0;
  unsigned int code = 0;

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  for (j = 0; j_instructions[j].name[0] != '\0'; j++) {
    if (strcmp(j_instructions[j].name, word) == 0) {
      opcode = j_instructions[j].opcode;
      break;
    }
  }

  if (validate_comma_syntax(line, (opcode == OPCODE_HLT ? 0 : 1), line_number,
                            error_found) == ERR)
    return 0;

  code |= (unsigned int)(opcode & 0x3F) << 26;

  if (opcode == OPCODE_HLT) {
    skip_spaces(line, &i);
    if (line[i] != '\0' && line[i] != '\n') {
      printf("Error: line %d - extraneous text after hlt instruction\n",
             line_number);
      *error_found = 1;
    }
    *complete = COMPLETE;
    sym_out[0] = '\0';
    return code;
  }

  get_next_token(line, &i, op1);

  if (op1[0] == '\0') {
    printf("Error: line %d - instruction '%s' missing operand\n", line_number,
           word);
    *error_found = 1;
    return 0;
  }

  if (opcode == 31 || opcode == 32) { /* la, call */
    if (op1[0] == '$') {
      printf("Error: line %d - instruction '%s' requires label operand, "
             "register given\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
  }

  if (op1[0] == '$') {
    reg = get_register(op1);
    if (reg == -1) {
      printf("Error: line %d - instruction '%s' invalid register operand\n",
             line_number, word);
      *error_found = 1;
      return 0;
    }
    code |= (unsigned int)(1) << 25;
    code |= (unsigned int)(reg) & 0x1F;
    *complete = COMPLETE;
    sym_out[0] = '\0';
  } else {
    *complete = INCOMPLETE;
    strncpy(sym_out, op1, MAX_LABEL_LENGTH);
    sym_out[MAX_LABEL_LENGTH] = '\0';
  }

  return code;
}

/**
 * @brief Encodes an instruction statement (R, I, or J type).
 * @param line Input line buffer.
 * @param IC Current instruction counter address.
 * @param complete Output flag set to 1 if fully encoded, 0 if target symbol
 * pending.
 * @param sym_out Output buffer to store target symbol name if complete == 0.
 * @param line_number Line number in source file for error reporting.
 * @param error_found Pointer to error flag in first_pass.
 * @return unsigned int 32-bit encoded machine code word.
 */
static unsigned int encode_instruction(char *line, int IC, int *complete,
                                       char *sym_out, int line_number,
                                       int *error_found) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  for (j = 0; r_instructions[j].name[0] != '\0'; j++)
    if (strcmp(r_instructions[j].name, word) == 0) {
      *complete = COMPLETE;
      sym_out[0] = '\0';
      return encode_r(line, line_number, error_found);
    }

  for (j = 0; i_instructions[j].name[0] != '\0'; j++)
    if (strcmp(i_instructions[j].name, word) == 0)
      return encode_i(line, IC, complete, sym_out, line_number, error_found);

  for (j = 0; j_instructions[j].name[0] != '\0'; j++)
    if (strcmp(j_instructions[j].name, word) == 0)
      return encode_j(line, complete, sym_out, line_number, error_found);

  *complete = INCOMPLETE;
  sym_out[0] = '\0';
  return 0;
}

/**
 * @brief Encodes data directive arguments (.db, .dh, .dw, .asciz) into
 * data_image.
 * @param line Input line buffer.
 * @param DC Pointer to current data counter address.
 */
static void encode_data(char *line, int *DC, int line_number, int *error_found) {
  int i = 0;
  int bytes = 1;
  int k = 0;
  int b = 0;
  long val = 0;
  char directive[MAX_LABEL_LENGTH + 1];
  char token[MAX_LINE_LENGTH];
  char *endptr = NULL;

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, directive);
  skip_spaces(line, &i);

  if (strcmp(directive, ".asciz") == 0) {
    if (line[i] != '"') {
      printf("Error: line %d - missing opening quote in .asciz directive\n", line_number);
      *error_found = 1;
      return;
    }
    i++;
    while (line[i] != '\0' && line[i] != '"') {
      add_to_data_image(*DC, (unsigned char)line[i]);
      (*DC)++;
      i++;
    }
    if (line[i] != '"') {
      printf("Error: line %d - missing closing quote in .asciz directive\n", line_number);
      *error_found = 1;
      return;
    }
    i++;
    add_to_data_image(*DC, 0);
    (*DC)++;
  } else {
    bytes = 1;
    if (strcmp(directive, ".dh") == 0)
      bytes = 2;
    if (strcmp(directive, ".dw") == 0)
      bytes = 4;

    while (line[i] != '\0' && line[i] != '\n') {
      k = 0;

      skip_spaces(line, &i);
      if (line[i] == '\0' || line[i] == '\n')
        break;

      while (line[i] != '\0' && line[i] != ',' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
        token[k++] = line[i++];
      token[k] = '\0';

      if (k == 0)
        break;

      val = strtol(token, &endptr, 10);
      if (*endptr != '\0') {
        printf("Error: line %d - invalid numeric argument '%s' in data directive\n", line_number, token);
        *error_found = 1;
        return;
      }

      for (b = 0; b < bytes; b++) {
        add_to_data_image(*DC, (unsigned char)(val & 0xFF));
        (*DC)++;
        val >>= 8;
      }

      skip_spaces(line, &i);
      if (line[i] == ',')
        i++;
    }
  }
}

/* ---- Pass 1 Main Pipeline ---- */

/**
 * @brief Executes Pass 1 over an expanded assembly source file (.am).
 *
 * @param filename Name of the source file.
 * @return int 0 on success, 1 on error.
 */
int first_pass(char *filename) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  char file_name[MAX_LINE_LENGTH];
  char label[MAX_LABEL_LENGTH + 1];
  char word[MAX_LABEL_LENGTH + 1];
  char sym[MAX_LABEL_LENGTH + 1];
  int IC = STARTING_ADDRESS;
  int DC = 0;
  int line_number = 0;
  int error_found = 0;
  int complete = COMPLETE;
  int idx = 0;
  int i = 0;
  unsigned int code = 0;
  Symbol *existing_sym = NULL;
  EntryNode *existing_ent = NULL;
  Symbol *cur = NULL;

  /* -------- Step 1: Open Expanded Assembly File (.am) -------- */
  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".am");
  fp = fopen(file_name, "r");
  if (!fp) {
    printf("Error: cannot open file %s\n", file_name);
    return ERR;
  }

  /* -------- Step 2: Line-by-Line Parsing & Symbol Collection -------- */
  while (fgets(line, MAX_LINE_LENGTH, fp)) {
    line_number++;
    trim_whitespace(line);

    if (is_empty_or_comment(line))
      continue;

    /* Step 2.1: Label Declaration Detection & Validation */
    if (is_label(line, label)) {
      existing_sym = find_symbol(label);
      if (existing_sym) {
        if (existing_sym->type == SYM_EXTERN) {
          printf("Error: line %d - label '%s' was declared as .extern and "
                 "cannot be defined in this file\n",
                 line_number, label);
        } else {
          printf("Error: line %d - label '%s' defined more than once\n",
                 line_number, label);
        }
        error_found = 1;
      } else if (is_data_directive(line))
        add_symbol(label, DC, SYM_DATA);
      else
        add_symbol(label, IC, SYM_CODE);
    }

    /* Step 2.2: Line Statement Encoding */
    if (is_instruction(line)) {
      complete = COMPLETE;
      code = encode_instruction(line, IC, &complete, sym, line_number,
                                &error_found);

      add_to_code_image(IC, code, complete, complete ? NULL : sym);
      IC += 4;
    } else if (is_data_directive(line)) {
      encode_data(line, &DC, line_number, &error_found);
    } else if (is_extern(line)) {
      idx = 0;

      skip_label(line, &idx);
      skip_spaces(line, &idx);
      get_next_word(line, &idx, word);
      skip_spaces(line, &idx);
      get_next_word(line, &idx, sym);

      if (sym[0] == '\0') {
        printf("Error: line %d - missing label after .extern\n", line_number);
        error_found = 1;
      } else {
        skip_spaces(line, &idx);
        if (line[idx] != '\0' && line[idx] != '\n') {
          printf("Error: line %d - extraneous text after .extern directive\n",
                 line_number);
          error_found = 1;
        } else {
          existing_sym = find_symbol(sym);
          existing_ent = find_entry(sym);

          if (existing_sym && existing_sym->type != SYM_EXTERN) {
            printf("Error: line %d - label '%s' is defined locally and cannot "
                   "be declared as .extern\n",
                   line_number, sym);
            error_found = 1;
          } else if (existing_ent) {
            printf("Error: line %d - label '%s' cannot be both .entry and "
                   ".extern\n",
                   line_number, sym);
            error_found = 1;
          } else if (!existing_sym) {
            add_symbol(sym, 0, SYM_EXTERN);
          }
        }
      }
    } else if (is_entry(line)) {
      idx = 0;

      skip_label(line, &idx);
      skip_spaces(line, &idx);
      get_next_word(line, &idx, word);
      skip_spaces(line, &idx);
      get_next_word(line, &idx, sym);

      if (sym[0] == '\0') {
        printf("Error: line %d - missing label after .entry\n", line_number);
        error_found = 1;
      } else {
        skip_spaces(line, &idx);
        if (line[idx] != '\0' && line[idx] != '\n') {
          printf("Error: line %d - extraneous text after .entry directive\n",
                 line_number);
          error_found = 1;
        } else {
          existing_sym = find_symbol(sym);

          if (existing_sym && existing_sym->type == SYM_EXTERN) {
            printf("Error: line %d - label '%s' cannot be both .entry and "
                   ".extern\n",
                   line_number, sym);
            error_found = 1;
          } else if (!find_entry(sym)) {
            add_entry(sym, line_number);
          }
        }
      }
    } else {
      printf("Error: line %d - unknown instruction\n", line_number);
      error_found = 1;
    }
  }

  fclose(fp);

  if (error_found)
    return ERR;

  /* -------- Step 3: Compute Final Memory Offsets -------- */
  ICF = IC;
  DCF = DC;

  /* Step 3.1: Adjust Data Symbol Addresses by Adding ICF */
  cur = symbol_table;
  while (cur) {
    if (cur->type == SYM_DATA)
      cur->value += ICF;
    cur = cur->next;
  }

  /* Step 3.2: Adjust Data Image Byte Addresses by Adding ICF */
  for (i = 0; i < data_size; i++)
    data_image[i].address += ICF;

  return OK;
}

/* ---- Memory Cleanup ---- */

/**
 * @brief Resets and frees all global memory allocations after processing a
 * file.
 */
void free_assembler_memory(void) {
  if (code_image) {
    free(code_image);
    code_image = NULL;
  }
  code_size = 0;
  code_capacity = 0;

  if (data_image) {
    free(data_image);
    data_image = NULL;
  }
  data_size = 0;
  data_capacity = 0;

  while (symbol_table) {
    Symbol *temp = symbol_table;
    symbol_table = symbol_table->next;
    free(temp);
  }

  while (entry_list) {
    EntryNode *temp = entry_list;
    entry_list = entry_list->next;
    free(temp);
  }

  while (extern_list) {
    ExternNode *temp = extern_list;
    extern_list = extern_list->next;
    free(temp);
  }
}
