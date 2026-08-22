/**
 * @file first_pass.h
 * @brief header file for first pass structures and functions
 */

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

/* ---- Memory Architecture & Constant Definitions ---- */

#ifndef OK
#define OK 0
#endif

#ifndef ERR
#define ERR 1
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define COMPLETE 1
#define INCOMPLETE 0
#define OPCODE_HLT 63
#define NAME_SIZE 10
#define STR_MATCH 0

#define MAX_LINE_LENGTH 82
#define MAX_LABEL_LENGTH 31
#define STARTING_ADDRESS 100

/* ---- Data Structure Definitions ---- */

/**
 * @struct CodeEntry
 * @brief code image entry holding address and 32bit binary instruction
 */
typedef struct {
  int address;                        /**< memory address starting 100 */
  unsigned int code;                 /**< 32bit machine code */
  int complete;                      /**< 1 if encoded ok, 0 if label operand needs pass 2 */
  char symbol[MAX_LABEL_LENGTH + 1]; /**< missing symbol name if complete == 0 */
} CodeEntry;

/**
 * @struct DataEntry
 * @brief data byte entry holding address and byte value
 */
typedef struct {
  int address;          /**< final data address */
  unsigned char value;  /**< byte value */
} DataEntry;

/**
 * @enum SymbolType
 * @brief symbol type (code, data, extern)
 */
typedef enum { SYM_CODE = 0, SYM_DATA = 1, SYM_EXTERN = 2 } SymbolType;

/**
 * @struct Symbol
 * @brief node in symbol table linked list
 */
typedef struct Symbol {
  char name[MAX_LABEL_LENGTH + 1]; /**< symbol label name */
  int value;                       /**< address value or 0 for extern */
  SymbolType type;                 /**< symbol type */
  struct Symbol *next;             /**< next symbol in list */
} Symbol;

/**
 * @struct EntryNode
 * @brief node for .entry list
 */
typedef struct EntryNode {
  char symbol[MAX_LABEL_LENGTH + 1]; /**< entry label name */
  int line_number;                   /**< line number where declared */
  int address;                       /**< resolved symbol address */
  struct EntryNode *next;            /**< next entry in list */
} EntryNode;

/**
 * @struct ExternNode
 * @brief node for .extern list
 */
typedef struct ExternNode {
  char symbol[MAX_LABEL_LENGTH + 1]; /**< extern label name */
  int address;                       /**< address of instruction referencing extern */
  struct ExternNode *next;           /**< next extern node */
} ExternNode;

/* ---- Global Assembler Variables ---- */

extern int ICF;                     /**< final instruction counter */
extern int DCF;                     /**< final data counter */
extern CodeEntry *code_image;       /**< dynamic array for code entries */
extern int code_size;               /**< amount of instructions in code_image */
extern DataEntry *data_image;       /**< dynamic array for data bytes */
extern int data_size;               /**< amount of bytes in data_image */
extern Symbol *symbol_table;        /**< symbol table list head */
extern EntryNode *entry_list;       /**< entry list head */
extern ExternNode *extern_list;     /**< extern list head */

/* ---- String & Token Parsing Helpers ---- */

/**
 * @brief check if word is reserved opcode or directive
 * @param word token to check
 * @return int 1 if reserved, 0 if not
 */
int is_reserved_word(const char *word);

/**
 * @brief skip spaces and tabs in line
 * @param line input line string
 * @param i current index pointer
 */
void skip_spaces(char *line, int *i);

/**
 * @brief get next word token from line
 * @param line input line string
 * @param i current index pointer
 * @param word output buffer for word
 */
void get_next_word(char *line, int *i, char *word);

/**
 * @brief trim spaces from start and end of line
 * @param line input line string
 */
void trim_whitespace(char *line);

/**
 * @brief skip label prefix at start of line
 * @param line input line string
 * @param i current index pointer
 */
void skip_label(char *line, int *i);

/* ---- Syntax Classification Functions ---- */

/**
 * @brief check if line defines a label
 * @param line input line string
 * @param label output buffer for label name
 * @return int 1 if label found, 0 if not
 */
int is_label(char *line, char *label);

/**
 * @brief check if line is empty or comment line
 * @param line input line string
 * @return int 1 if empty or comment, 0 if not
 */
int is_empty_or_comment(char *line);

/**
 * @brief check if line is instruction statement
 * @param line input line string
 * @return int 1 if instruction, 0 if not
 */
int is_instruction(char *line);

/**
 * @brief check if line is data directive (.db, .dh, .dw, .asciz)
 * @param line input line string
 * @return int 1 if data directive, 0 if not
 */
int is_data_directive(char *line);

/**
 * @brief check if line is .extern directive
 * @param line input line string
 * @return int 1 if .extern, 0 if not
 */
int is_extern(char *line);

/* ---- Symbol Table & Linked List Functions ---- */

/**
 * @brief add new symbol node to symbol table
 * @param name symbol label name
 * @param value address or offset
 * @param type symbol type (code/data/extern)
 */
void add_symbol(const char *name, int value, SymbolType type);

/**
 * @brief find symbol in symbol table by name
 * @param name symbol name to search
 * @return Symbol* pointer to symbol node if found, NULL if not
 */
Symbol *find_symbol(const char *name);

/* ---- Pass 1 Main Pipeline ---- */

/**
 * @brief run first pass over .am file
 *
 * @param filename name of source file
 * @return int 0 if pass ok, 1 if error
 */
int first_pass(char *filename);

/* ---- Memory Cleanup ---- */

/**
 * @brief free allocated memory for assembler tables
 */
void free_assembler_memory(void);

#endif
