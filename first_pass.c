#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "first_pass.h"

/* ---- מילים שמורות ---- */

static const char *reserved_words[] = {
    "add",  "sub",  "and",    "or",     "nor",     "move", "mvhi",
    "mvlo", "addi", "subi",   "andi",   "ori",     "nori", "bne",
    "beq",  "blt",  "bgt",    "lb",     "sb",      "lw",   "sw",
    "lh",   "sh",   "jmp",    "la",     "call",    "hlt",  ".db",
    ".dh",  ".dw",  ".asciz", ".entry", ".extern", NULL};

int is_reserved_word(const char *word) {
  int i = 0;
  while (reserved_words[i] != NULL) {
    if (strcmp(reserved_words[i], word) == 0)
      return 1;
    i++;
  }
  return 0;
}

/* ---- טבלאות הוראות ---- */

typedef struct {
  char name[10];
  int opcode;
  int funct;
} RInstruction;

typedef struct {
  char name[10];
  int opcode;
} IInstruction;

typedef struct {
  char name[10];
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

/* ---- משתנים גלובליים ---- */

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

/* functions for printing tables */

void print_binary_32(unsigned int num) {
  int i;
  for (i = 31; i >= 0; i--) {
    putchar((num & (1U << i)) ? '1' : '0');
  }
}

void print_binary_8(unsigned char num) {
  int i;
  for (i = 7; i >= 0; i--) {
    putchar((num & (1 << i)) ? '1' : '0');
  }
}

void print_code_image(CodeEntry *code_image, int code_size) {
  int i;

  for (i = 0; i < code_size; i++) {
    printf("%d | ", code_image[i].address);
    print_binary_32(code_image[i].code);
    printf(" | %d | %s\n", code_image[i].complete, code_image[i].symbol);
  }
}

void print_data_image(DataEntry *data_image, int data_size) {
  int i;
  for (i = 0; i < data_size; i++) {
    printf("%d | ", data_image[i].address);
    print_binary_8(data_image[i].value);
    printf("\n");
  }
}

void print_symbol_table(Symbol *symbol_table) {
  Symbol *cur = symbol_table;
  while (cur) {
    printf("%s | %d | %d\n", cur->name, cur->value, cur->type);
    cur = cur->next;
  }
}

void print_entry_list(EntryNode *entry_list) {
  EntryNode *cur = entry_list;
  while (cur) {
    printf("%s | %d\n", cur->symbol, cur->line_number);
    cur = cur->next;
  }
}

/* ---- פונקציות עזר ---- */

void skip_spaces(char *line, int *i) {
  while (line[*i] == ' ' || line[*i] == '\t')
    (*i)++;
}

void get_next_word(char *line, int *i, char *word) {
  int j = 0;
  while (line[*i] != '\0' && line[*i] != ' ' && line[*i] != '\t')
    word[j++] = line[(*i)++];
  word[j] = '\0';
}

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

/* ---- פונקציות טבלת סמלים ---- */

void add_symbol(const char *name, int value, int type) {
  Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
  strncpy(sym->name, name, MAX_LABEL_LENGTH);
  sym->name[MAX_LABEL_LENGTH] = '\0';
  sym->value = value;
  sym->type = type;
  sym->next = symbol_table;
  symbol_table = sym;
}

Symbol *find_symbol(const char *name) {
  Symbol *cur = symbol_table;
  while (cur) {
    if (strcmp(cur->name, name) == 0)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

/* ---- פונקציות לאנטרי ---- */

void add_entry(const char *name, int line_number) {
  EntryNode *ent = (EntryNode *)malloc(sizeof(EntryNode));
  strncpy(ent->symbol, name, MAX_LABEL_LENGTH);
  ent->symbol[MAX_LABEL_LENGTH] = '\0';
  ent->line_number = line_number;
  ent->next = entry_list;
  entry_list = ent;
}

EntryNode *find_entry(const char *name) {
  EntryNode *cur = entry_list;
  while (cur) {
    if (strcmp(cur->symbol, name) == 0)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

/* ---- פונקציות תמונת קוד ---- */

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

/* ---- פונקציות תמונת נתונים ---- */

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

/* ---- פונקציות זיהוי ---- */

void skip_label(char *line, int *i) {
  int j = 0;
  while (line[j] != '\0' && line[j] != ':' && line[j] != ' ' && line[j] != '\t')
    j++;

  if (line[j] == ':')
    *i = j + 1;
  else
    *i = 0;
}

int is_label(char *line, char *label) {
  int i = 0;
  char first_word[MAX_LABEL_LENGTH + 1];

  get_next_word(line, &i, first_word);

  if (line[i] == ':') {
    strcpy(label, first_word);
    if (!isalpha((unsigned char)label[0])) {
      label[0] = '\0';
      return 0;
    }
    return 1;
  }

  label[0] = '\0';
  return 0;
}

int is_empty_or_comment(char *line) {
  int i = 0;
  skip_spaces(line, &i);
  return (line[i] == '\0' || line[i] == '\n' || line[i] == ';');
}

int is_instruction(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  if (word[0] == '.')
    return 0;

  return is_reserved_word(word);
}

int is_data_directive(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  if (strcmp(word, ".db") == 0)
    return 1;
  if (strcmp(word, ".dh") == 0)
    return 1;
  if (strcmp(word, ".dw") == 0)
    return 1;
  if (strcmp(word, ".asciz") == 0)
    return 1;

  return 0;
}

int is_extern(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  return (strcmp(word, ".extern") == 0);
}

int is_entry(char *line) {
  int i = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  return (strcmp(word, ".entry") == 0);
}

/* ---- קידוד הוראות ---- */

static int get_register(const char *token) {
  if (token[0] != '$')
    return -1;
  return atoi(token + 1);
}

static unsigned int encode_r(char *line) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];
  char op1[10], op2[10], op3[10];
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

  skip_spaces(line, &i);

  if (opcode == 1) {
    get_next_word(line, &i, op1);
    if (line[i] == ',')
      i++;
    skip_spaces(line, &i);
    get_next_word(line, &i, op2);

    rd = get_register(op1);
    rs = get_register(op2);
    rt = 0;
  } else {
    get_next_word(line, &i, op1);
    if (line[i] == ',')
      i++;
    skip_spaces(line, &i);
    get_next_word(line, &i, op2);
    if (line[i] == ',')
      i++;
    skip_spaces(line, &i);
    get_next_word(line, &i, op3);

    rs = get_register(op1);
    rt = get_register(op2);
    rd = get_register(op3);
  }

  code |= (unsigned int)(opcode & 0x3F) << 26;
  code |= (unsigned int)(rs & 0x1F) << 21;
  code |= (unsigned int)(rt & 0x1F) << 16;
  code |= (unsigned int)(rd & 0x1F) << 11;
  code |= (unsigned int)(funct & 0x1F) << 6;

  return code;
}

static unsigned int encode_i(char *line, int IC, int *complete, char *sym_out) {
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

  is_branch = (opcode >= 15 && opcode <= 18);

  skip_spaces(line, &i);
  get_next_word(line, &i, op1);
  if (line[i] == ',')
    i++;
  skip_spaces(line, &i);
  get_next_word(line, &i, op2);
  if (line[i] == ',')
    i++;
  skip_spaces(line, &i);
  get_next_word(line, &i, op3);

  if (is_branch) {
    rs = get_register(op1);
    rt = get_register(op2);
    immed = 0;
    *complete = 0;
    strncpy(sym_out, op3, MAX_LABEL_LENGTH);
    sym_out[MAX_LABEL_LENGTH] = '\0';
  } else {
    rs = get_register(op1);
    immed = atol(op2);
    rt = get_register(op3);
    *complete = 1;
    sym_out[0] = '\0';
  }

  code |= (unsigned int)(opcode & 0x3F) << 26;
  code |= (unsigned int)(rs & 0x1F) << 21;
  code |= (unsigned int)(rt & 0x1F) << 16;
  code |= (unsigned int)((short)immed & 0xFFFF);

  return code;
}

static unsigned int encode_j(char *line, int *complete, char *sym_out) {
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

  code |= (unsigned int)(opcode & 0x3F) << 26;

  if (opcode == 63) {
    *complete = 1;
    sym_out[0] = '\0';
    return code;
  }

  skip_spaces(line, &i);
  get_next_word(line, &i, op1);

  if (op1[0] == '$') {
    reg = get_register(op1);
    code |= (unsigned int)(1) << 25;
    code |= (unsigned int)(reg) & 0x1F;
    *complete = 1;
    sym_out[0] = '\0';
  } else {
    *complete = 0;
    strncpy(sym_out, op1, MAX_LABEL_LENGTH);
    sym_out[MAX_LABEL_LENGTH] = '\0';
  }

  return code;
}

static unsigned int encode_instruction(char *line, int IC, int *complete,
                                       char *sym_out) {
  int i = 0, j = 0;
  char word[MAX_LABEL_LENGTH + 1];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, word);

  for (j = 0; r_instructions[j].name[0] != '\0'; j++)
    if (strcmp(r_instructions[j].name, word) == 0) {
      *complete = 1;
      sym_out[0] = '\0';
      return encode_r(line);
    }

  for (j = 0; i_instructions[j].name[0] != '\0'; j++)
    if (strcmp(i_instructions[j].name, word) == 0)
      return encode_i(line, IC, complete, sym_out);

  for (j = 0; j_instructions[j].name[0] != '\0'; j++)
    if (strcmp(j_instructions[j].name, word) == 0)
      return encode_j(line, complete, sym_out);

  *complete = 0;
  sym_out[0] = '\0';
  return 0;
}

/* ---- קידוד נתונים ---- */

static void encode_data(char *line, int *DC) {
  int i = 0;
  char directive[MAX_LABEL_LENGTH + 1];
  char token[MAX_LINE_LENGTH];

  skip_label(line, &i);
  skip_spaces(line, &i);
  get_next_word(line, &i, directive);
  skip_spaces(line, &i);

  if (strcmp(directive, ".asciz") == 0) {
    if (line[i] == '"')
      i++;
    while (line[i] != '\0' && line[i] != '"') {
      add_to_data_image(*DC, (unsigned char)line[i]);
      (*DC)++;
      i++;
    }
    add_to_data_image(*DC, 0);
    (*DC)++;
  } else {
    int bytes = 1;
    if (strcmp(directive, ".dh") == 0)
      bytes = 2;
    if (strcmp(directive, ".dw") == 0)
      bytes = 4;

    while (line[i] != '\0') {
      int k = 0;
      long val;

      skip_spaces(line, &i);
      while (line[i] != '\0' && line[i] != ',' && line[i] != ' ' &&
             line[i] != '\t')
        token[k++] = line[i++];
      token[k] = '\0';

      if (k == 0)
        break;

      val = atol(token);

      {
        int b;
        for (b = 0; b < bytes; b++) {
          add_to_data_image(*DC, (unsigned char)(val & 0xFF));
          (*DC)++;
          val >>= 8;
        }
      }

      if (line[i] == ',')
        i++;
    }
  }
}

/* ---- הפונקציה הראשית ---- */

int first_pass(char *filename) {
  FILE *fp;
  char line[MAX_LINE_LENGTH];
  char file_name[MAX_LINE_LENGTH];
  char label[MAX_LABEL_LENGTH + 1];
  int IC = 0;
  int DC = 0;
  int line_number = 0;
  int error_found = 0;

  filename = strtok(filename, ".");
  strcpy(file_name, filename);
  strcat(file_name, ".am");
  fp = fopen(file_name, "r");
  if (!fp) {
    printf("Error: cannot open file %s\n", file_name);
    return 1;
  }

  while (fgets(line, MAX_LINE_LENGTH, fp)) {
    line_number++;

    trim_whitespace(line);

    if (is_empty_or_comment(line))
      continue;

    if (is_label(line, label)) {
      if (find_symbol(label)) {
        printf("Error: line %d - label '%s' defined more than once\n",
               line_number, label);
        error_found = 1;
      } else if (is_data_directive(line))
        add_symbol(label, DC, 1);
      else
        add_symbol(label, IC, 0);
    }

    if (is_instruction(line)) {
      int complete = 1;
      char sym[MAX_LABEL_LENGTH + 1];
      unsigned int code = encode_instruction(line, IC, &complete, sym);

      add_to_code_image(IC, code, complete, complete ? NULL : sym);
      IC += 4;
    } else if (is_data_directive(line)) {
      encode_data(line, &DC);
    } else if (is_extern(line)) {
      int i = 0;
      char word[MAX_LABEL_LENGTH + 1];
      char sym[MAX_LABEL_LENGTH + 1];

      skip_label(line, &i);
      skip_spaces(line, &i);
      get_next_word(line, &i, word);
      skip_spaces(line, &i);
      get_next_word(line, &i, sym);

      if (sym[0] == '\0') {
        printf("Error: line %d - missing label after .extern\n", line_number);
        error_found = 1;
      } else {
        skip_spaces(line, &i);
        if (line[i] != '\0' && line[i] != '\n') {
          printf("Error: line %d - extraneous text after .extern directive\n",
                 line_number);
          error_found = 1;
        } else if (!find_symbol(sym)) {
          add_symbol(sym, 0, 2);
        }
      }
    } else if (is_entry(line)) {
      int i = 0;
      char word[MAX_LABEL_LENGTH + 1];
      char sym[MAX_LABEL_LENGTH + 1];

      skip_label(line, &i);
      skip_spaces(line, &i);
      get_next_word(line, &i, word);
      skip_spaces(line, &i);
      get_next_word(line, &i, sym);

      if (sym[0] == '\0') {
        printf("Error: line %d - missing label after .entry\n", line_number);
        error_found = 1;
      } else {
        skip_spaces(line, &i);
        if (line[i] != '\0' && line[i] != '\n') {
          printf("Error: line %d - extraneous text after .entry directive\n",
                 line_number);
          error_found = 1;
        } else if (!find_entry(sym)) {
          add_entry(sym, line_number);
        }
      }
    } else {
      printf("Error: line %d - unknown instruction\n", line_number);
      error_found = 1;
    }
    printf("line %d - %s\n", line_number, line); /*need to remove*/
  }

  fclose(fp);

  if (error_found)
    return 1;

  ICF = IC;
  DCF = DC;

  { /*why are the code blocks here?*/
    Symbol *cur = symbol_table;
    while (cur) {
      if (cur->type == 1)
        cur->value += ICF;
      cur = cur->next;
    }
  }

  {
    int i;
    for (i = 0; i < data_size; i++)
      data_image[i].address += ICF;
  }
  printf("------------------------------\n"); /*need to remove*/
  print_code_image(code_image, code_size);
  printf("------------------------------\n");
  print_data_image(data_image, data_size);
  printf("------------------------------\n");
  print_symbol_table(symbol_table);
  printf("------------------------------\n");
  print_entry_list(entry_list);

  return 0;
}
