/**
 * @file first_pass.h
 * @brief Header file for First Pass memory layout, data structures, and function declarations.
 */

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#define MAX_LINE_LENGTH 82
#define MAX_LABEL_LENGTH 31
#define STARTING_ADDRESS 100

/* ---- Data Structures ---- */

/**
 * @struct CodeEntry
 * @brief Represents an encoded 32-bit instruction word in code memory.
 */
typedef struct {
  int address;                        /**< Memory address (starting at 100) */
  unsigned int code;                 /**< Encoded 32-bit instruction word */
  int complete;                      /**< 1 if fully resolved, 0 if operand label needs Pass 2 */
  char symbol[MAX_LABEL_LENGTH + 1]; /**< Missing operand symbol name if complete == 0 */
} CodeEntry;

/**
 * @struct DataEntry
 * @brief Represents a single byte in data memory.
 */
typedef struct {
  int address;          /**< Final memory address (starting at ICF) */
  unsigned char value;  /**< Byte value */
} DataEntry;

/**
 * @enum SymbolType
 * @brief Symbol categorization (Code label, Data label, or External reference).
 */
typedef enum { SYM_CODE = 0, SYM_DATA = 1, SYM_EXTERN = 2 } SymbolType;

/**
 * @struct Symbol
 * @brief Node in the Symbol Table linked list.
 */
typedef struct Symbol {
  char name[MAX_LABEL_LENGTH + 1]; /**< Symbol label name */
  int value;                       /**< Memory address or 0 for EXTERN */
  SymbolType type;                 /**< Symbol type (CODE, DATA, EXTERN) */
  struct Symbol *next;             /**< Pointer to next symbol in linked list */
} Symbol;

/**
 * @struct EntryNode
 * @brief Node in the .entry declarations linked list.
 */
typedef struct EntryNode {
  char symbol[MAX_LABEL_LENGTH + 1]; /**< Entry label name */
  int line_number;                   /**< Source file line number for error logging */
  int address;                       /**< Resolved memory address from Symbol Table */
  struct EntryNode *next;            /**< Pointer to next entry node */
} EntryNode;

/**
 * @struct ExternNode
 * @brief Node in the .extern references linked list.
 */
typedef struct ExternNode {
  char symbol[MAX_LABEL_LENGTH + 1]; /**< External label name */
  int address;                       /**< Memory address of the instruction referencing it */
  struct ExternNode *next;           /**< Pointer to next extern node */
} ExternNode;

/* ---- Global Variables ---- */

extern int ICF;                     /**< Final Instruction Counter */
extern int DCF;                     /**< Final Data Counter */
extern CodeEntry *code_image;       /**< Dynamic array of code entries */
extern int code_size;               /**< Number of instructions in code_image */
extern DataEntry *data_image;       /**< Dynamic array of data bytes */
extern int data_size;               /**< Number of bytes in data_image */
extern Symbol *symbol_table;        /**< Head pointer of Symbol Table linked list */
extern EntryNode *entry_list;       /**< Head pointer of Entry declarations linked list */
extern ExternNode *extern_list;     /**< Head pointer of Extern references linked list */

/* ---- Function Declarations ---- */

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

/**
 * @brief Executes Pass 1 over an assembly source file.
 *
 * @param filename Name of the source file.
 * @return int 0 on success, 1 on error.
 */
int first_pass(char *filename);

void print_code_image(CodeEntry *code_image, int code_size);
void print_data_image(DataEntry *data_image, int data_size);
void print_symbol_table(Symbol *symbol_table);
void print_entry_list(EntryNode *entry_list);
void print_extern_list(ExternNode *extern_list);

/**
 * @brief Resets and frees all global memory allocations after processing a file.
 */
void free_assembler_memory(void);

#endif
