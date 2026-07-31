/* find errors and clean input*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "validation.h"

/* clean spaces */
static const char *skip_spaces(const char *p)
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* clean sideline spaces*/
void clean_sidelines(char *line)
{
    char tmp[MAX_LINE_LENGTH];
    int end;

    if (!line) return;

    strcpy(tmp, (char *)skip_spaces(line));
    strcpy(line, tmp);

    end = (int)strlen(line) - 1;
    while (end >= 0 && (line[end]==' ' || line[end]=='\t' ||
                        line[end]=='\n' || line[end]=='\r'))
        end--;
    line[end + 1] = '\0';
}

/* valid line length */
int is_valid_line_length(const char *line)
{
    int len = 0;
    while (line[len] && line[len] != '\n' && line[len] != '\r') len++;
    return (len <= 80);
}

/* valid label */
int is_valid_label(const char *name)
{
    int i, len;
    if (!name || !isalpha((unsigned char)name[0])) return 0;
    len = (int)strlen(name);
    if (len > MAX_LABEL_LENGTH) return 0;
    for (i = 1; i < len; i++)
        if (!isalnum((unsigned char)name[i])) return 0;
    return !is_reserved_word(name);
}

/* valid opcode name */
int is_valid_opcode_name(const char *op)
{
    int i;
    if (!op || !op[0]) return 0;
    for (i = 0; op[i]; i++)
        if (!isalpha((unsigned char)op[i])) return 0;
    return 1;
}

/* valid register */
int is_valid_register(const char *token, int *reg_num)
{
    int num, i;
    if (!token || token[0]!='$' || token[1]=='\0') return 0;
    for (i = 1; token[i]; i++)
        if (!isdigit((unsigned char)token[i])) return 0;
    num = atoi(token + 1);
    if (num < 0 || num > 31) return 0;
    if (reg_num) *reg_num = num;
    return 1;
}

