#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pre_assembler.h"
#include "error_check.h"

int is_repeated_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == 0) {
      return 1; /* 1 means label IS found in table */
    }
  }
  return 0; /* 0 means NOT found */
}

int find_label(const char *label, Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    if (strcmp(m_list[i].label, label) == 0) {
      return i;
    }
  }
  return -1; /* -1 means NOT found */
}

void print_macro(Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    printf("%s: \n", m_list[i].label);
    printf("%s\n", m_list[i].content);
  }
}

int main(void) {
  char file[] = "test.as";
  int res = pre_assemble(file);
  printf("Result code: %d\n", res);

  return OK;
}

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
        printf("error, no label for macro\n");
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if ((token = strtok(NULL, " \t\n\r")) != NULL) {
        printf("error, excess content after mcro label\n");
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (!is_valid_label(m_label)) {
        printf("error, illegal macro label: %s\n", m_label);
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      if (is_repeated_label(m_label, m_list, mcro_count)) {
        printf("error, repeated macro label: %s\n", m_label);
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
        printf("error, excess content after mcroend\n");
        fclose(rfp);
        fclose(wfp);
        return ERR;
      }
      m_list[mcro_count - 1].content = content;
    }
    /* --------------- handle case: macro call expansion --------------- */
    else if (token != NULL && (macro_idx = find_label(token, m_list, mcro_count)) != -1) {
      fputs(m_list[macro_idx].content, wfp);
    }
    /* --------------- handle case: regular line --------------- */
    else {
      fputs(line, wfp);
    }
  }

  fclose(rfp);
  fclose(wfp);
  print_macro(m_list, mcro_count);

  return OK;
}
