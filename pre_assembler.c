#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "first_pass.h"
#include "pre_assembler.h"

int pre_assemble(char *file_name) {
  FILE *rfp;
  FILE *wfp;
  char new_file_name[LINE_SIZE];
  char line[LINE_SIZE];
  char work_line[LINE_SIZE];
  char *token;
  int mcro_count = 0;
  int mcro_lines = 0;
  char *content;
  char *m_label;
  Macro *m_list = NULL;
  int macro_idx;
  int line_num = 0;

  /* Check if file name ends with .as */
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

  /* Go through each line in the source file */
  while (fgets(line, LINE_SIZE, rfp) != NULL) {
    strcpy(work_line, line);
    token = strtok(work_line, " \t\n\r");

    /* --------------- handle case: macro definition --------------- */
    if (token != NULL && strcmp(token, "mcro") == 0) {
      m_label = strtok(NULL, " \t\n\r");
      if (m_label == NULL) {
        printf("error: line %d, no label for macro\n", line_num);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if ((token = strtok(NULL, " \t\n\r")) != NULL) {
        printf("error: line %d, excess content after mcro label\n", line_num);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (!is_valid_macro_label(m_label)) {
        printf("error: line %d, illegal macro label: %s\n", line_num, m_label);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (!is_free_macro_label(m_label, m_list, mcro_count)) {
        printf("error: line %d, repeated macro label: %s\n", line_num, m_label);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }

      /* Save macro label */
      mcro_count++;
      m_list = (Macro *)realloc(m_list, mcro_count * sizeof(Macro));
      if (m_list == NULL) {
        printf("error, memory allocation failed\n");
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      strcpy(m_list[mcro_count - 1].label, m_label);

      /* Allocate memory for macro content */
      content = (char *)malloc(LINE_SIZE);
      if (content == NULL) {
        printf("error, memory allocation failed\n");
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }

      strcpy(content, "");
      mcro_lines = 0;

      fgets(line, LINE_SIZE, rfp);
      strcpy(work_line, line);
      token = strtok(work_line, " \t\n\r");

      /*save macro content*/

      while (token == NULL || strcmp(token, "mcroend") != 0) {
        mcro_lines++;
        content = (char *)realloc(content, mcro_lines * LINE_SIZE);
        if (content == NULL) {
          printf("error, memory allocation failed\n");
          fclose(rfp);
          fclose(wfp);
          return ERR;
        }

        strcat(content, line);
        if (fgets(line, LINE_SIZE, rfp) == NULL)
          break;

        strcpy(work_line, line);
        token = strtok(work_line, " \t\n\r");
      }
      if (token != NULL && (token = strtok(NULL, " \t\n\r")) != NULL) {
        printf("error: line %d, excess content after mcroend\n", line_num);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      m_list[mcro_count - 1].content = content;
    }
    /* --------------- handle case: macro call expansion --------------- */
    else if (token != NULL &&
             (macro_idx = find_macro_label(token, m_list, mcro_count)) != -1) {
      fputs(m_list[macro_idx].content, wfp);
    }
    /* --------------- handle case: regular line --------------- */
    else {
      fputs(line, wfp);
    }
    line_num++;
  }
  /* free memory */
  fclose(rfp);
  fclose(wfp);

  return OK;
}

int is_free_macro_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == 0) {
      return ERR;
    }
  }
  return OK;
}

int find_macro_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == 0) {
      return i;
    }
  }
  return -1;
}

int is_valid_macro_label(const char *name) {
  int i, len;
  if (!name || !isalpha((unsigned char)name[0]))
    return ERR;
  len = (int)strlen(name);
  if (len > MAX_LABEL_LENGTH)
    return ERR;
  for (i = 1; i < len; i++)
    if (!isalnum((unsigned char)name[i]) && (unsigned char)name[i] != '_')
      return ERR;
  return !is_reserved_word(name);
}
