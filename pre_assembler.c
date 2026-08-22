/**
 * @file pre_assembler.c
 * @brief implementation of macro expansion pass
 *
 * reads .as file, saves macros ('mcro'...'mcroend'), expands macro calls and
 * writes to .am file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "first_pass.h"
#include "pre_assembler.h"

/* ---- Main Pre-Assembler Engine ---- */

/**
 * @brief pre assemble input file by expanding macros
 *
 * reads .as file, saves macros ('mcro'...'mcroend'), expands macro calls and
 * writes to .am file
 *
 * @param file_name name of input file
 * @return int OK (0) if pass succeeded, ERR (1) if failed
 */
int pre_assemble(char *file_name) {
  FILE *rfp;
  FILE *wfp;
  char new_file_name[LINE_SIZE];
  char line[LINE_SIZE];
  char work_line[LINE_SIZE];
  char *token;
  char *check_token;
  char *temp_content;
  int mcro_count = 0;
  int mcro_lines = 0;
  char *content;
  char *m_label;
  Macro *m_list = NULL;
  Macro *temp = NULL;
  int macro_idx;
  int line_num = 0;

  /* -------- Step 1: check file extension -------- */
  if ((file_name = strtok(file_name, ".")) == NULL) {
    printf("error, invalid file name\n");
    return ERR;
  } else if (strcmp(strtok(NULL, "."), "as") != 0) {
    printf("error, invalid file type\n");
    return ERR;
  } else if (strtok(NULL, ".") != NULL) {
    printf("error, invalid file name\n");
    return ERR;
  }

  /* -------- Step 2: open source file (.as) and destination file (.am) --------
   */
  strcpy(new_file_name, file_name);
  strcat(new_file_name, ".as");

  if ((rfp = fopen(new_file_name, "r")) == NULL) {
    printf("\ncould not open file\n");
    return ERR;
  }

  strcpy(new_file_name, file_name);
  strcat(new_file_name, ".am");

  if ((wfp = fopen(new_file_name, "w")) == NULL) {
    printf("could not create new file\n");
    fclose(rfp);
    return ERR;
  }

  /* -------- Step 3: loop lines and do macro expansion -------- */
  while (fgets(line, LINE_SIZE, rfp) != NULL) {
    strcpy(work_line, line);
    token = strtok(work_line, " \t\n\r");

    /* Step 3.1: check if mcro appears after another word on line */
    if (token != NULL && strcmp(token, "mcro") != 0) {
      check_token = strtok(NULL, " \t\n\r");
      while (check_token != NULL) {
        if (strcmp(check_token, "mcro") == 0) {
          printf("error: line %d, excess content before mcro\n", line_num);
          free_memory_pre_assembler(m_list, mcro_count);
          fclose(rfp);
          fclose(wfp);
          return ERR;
        }
        check_token = strtok(NULL, " \t\n\r");
      }
    }

    /* Step 3.2: process mcro definition block */
    if (token != NULL && strcmp(token, "mcro") == 0) {
      m_label = strtok(NULL, " \t\n\r");
      if (m_label == NULL) {
        printf("error: line %d, no label for macro\n", line_num);
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if ((token = strtok(NULL, " \t\n\r")) != NULL) {
        printf("error: line %d, excess content after mcro label\n", line_num);
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (!is_valid_macro_label(m_label)) {
        printf("error: line %d, illegal macro label: %s\n", line_num, m_label);
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (!is_free_macro_label(m_label, m_list, mcro_count)) {
        printf("error: line %d, repeated macro label: %s\n", line_num, m_label);
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }

      /* realloc memory for macro array */
      temp = (Macro *)realloc(m_list, (mcro_count + 1) * sizeof(Macro));
      if (temp == NULL) {
        printf("error, memory allocation failed\n");
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      m_list = temp;
      m_list[mcro_count].content = NULL;
      strcpy(m_list[mcro_count].label, m_label);
      mcro_count++;

      /* read macro body lines until mcroend */
      content = (char *)malloc(LINE_SIZE);
      if (content == NULL) {
        printf("error, memory allocation failed\n");
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }

      strcpy(content, "");
      mcro_lines = 0;

      fgets(line, LINE_SIZE, rfp);
      strcpy(work_line, line);
      token = strtok(work_line, " \t\n\r");

      while (token == NULL || strcmp(token, "mcroend") != 0) {
        mcro_lines++;
        temp_content = (char *)realloc(content, mcro_lines * LINE_SIZE);
        if (temp_content == NULL) {
          printf("error, memory allocation failed\n");
          free(content);
          free_memory_pre_assembler(m_list, mcro_count);
          fclose(rfp);
          fclose(wfp);
          return ERR;
        }
        content = temp_content;

        strcat(content, line);
        if (fgets(line, LINE_SIZE, rfp) == NULL)
          break;

        strcpy(work_line, line);
        token = strtok(work_line, " \t\n\r");
      }
      if (token != NULL && (token = strtok(NULL, " \t\n\r")) != NULL) {
        printf("error: line %d, excess content after mcroend\n", line_num);
        free(content);
        free_memory_pre_assembler(m_list, mcro_count);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      m_list[mcro_count - 1].content = content;
    }
    /* Step 3.3: expand macro call if name matches */
    else if (token != NULL && (macro_idx = find_macro_label(
                                   token, m_list, mcro_count)) != NOT_FOUND) {
      fputs(m_list[macro_idx].content, wfp);
    }
    /* Step 3.4: write regular code line to output */
    else {
      fputs(line, wfp);
    }
    line_num++;
  }

  /* -------- Step 4: clean memory and close files -------- */
  free_memory_pre_assembler(m_list, mcro_count);
  fclose(rfp);
  fclose(wfp);

  return OK;
}

/* ---- Macro Validation & Lookup Helpers ---- */

/**
 * @brief check if label was not used as macro name before
 *
 * @param label macro name to check
 * @param m_list list of saved macros
 * @param mcro_count number of macros saved
 * @return int TRUE if label free, FALSE if already exists
 */
int is_free_macro_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == STR_MATCH) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * @brief search macro in macro array by label
 *
 * @param label macro name to locate
 * @param m_list list of saved macros
 * @param mcro_count number of macros saved
 * @return int index in m_list if found, NOT_FOUND if not found
 */
int find_macro_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == STR_MATCH) {
      return i;
    }
  }
  return NOT_FOUND;
}

/**
 * @brief check if macro name is valid identifier
 *
 * @param name candidate macro label
 * @return int TRUE if valid, FALSE if invalid (like reserved word or bad chars)
 */
int is_valid_macro_label(const char *name) {
  int i, len;
  if (!name || !isalpha((unsigned char)name[0]))
    return FALSE;
  len = (int)strlen(name);
  if (len > MAX_LABEL_LENGTH)
    return FALSE;
  for (i = 1; i < len; i++)
    if (!isalnum((unsigned char)name[i]) && (unsigned char)name[i] != '_')
      return FALSE;
  return !is_reserved_word(name);
}

/* ---- Memory Management ---- */

/**
 * @brief free allocated memory of macro list
 *
 * @param m_list list of Macro structures
 * @param mcro_count number of macros in list
 */
void free_memory_pre_assembler(Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    free(m_list[i].content);
  }
  free(m_list);
}
