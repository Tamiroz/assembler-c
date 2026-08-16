#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#define MAX_LINE_LENGTH 82
#define MAX_LABEL_LENGTH 31

/* ---- מבני נתונים — זמינים למעבר השני ---- */

typedef struct {
  int address;
  unsigned int code;
  int complete;
  char symbol[MAX_LABEL_LENGTH + 1];
} CodeEntry;

typedef struct {
  int address;
  unsigned char value;
} DataEntry;

typedef enum { SYM_CODE = 0, SYM_DATA = 1, SYM_EXTERN = 2 } SymbolType;

typedef struct Symbol {
  char name[MAX_LABEL_LENGTH + 1];
  int value;
  SymbolType type;
  struct Symbol *next;
} Symbol;

typedef struct EntryNode {
  char symbol[MAX_LABEL_LENGTH + 1];
  int line_number;
  int address;
  struct EntryNode *next;
} EntryNode;

typedef struct ExternNode {
  char symbol[MAX_LABEL_LENGTH + 1];
  int address;
  struct ExternNode *next;
} ExternNode;

/* ---- משתנים גלובליים — זמינים למעבר השני ---- */

extern int ICF;
extern int DCF;
extern CodeEntry *code_image;
extern int code_size;
extern DataEntry *data_image;
extern int data_size;
extern Symbol *symbol_table;
extern EntryNode *entry_list;
extern ExternNode *extern_list;

/* ---- פונקציות ---- */

int is_reserved_word(const char *word);
void skip_spaces(char *line, int *i);
void get_next_word(char *line, int *i, char *word);
void trim_whitespace(char *line);
void skip_label(char *line, int *i);
int is_label(char *line, char *label);
int is_empty_or_comment(char *line);
int is_instruction(char *line);
int is_data_directive(char *line);
int is_extern(char *line);
void add_symbol(const char *name, int value, int type);
Symbol *find_symbol(const char *name);
int first_pass(char *filename);
void print_code_image(CodeEntry *code_image, int code_size);
void print_data_image(DataEntry *data_image, int data_size);
void print_symbol_table(Symbol *symbol_table);
void print_entry_list(EntryNode *entry_list);
void print_extern_list(ExternNode *extern_list);

#endif
