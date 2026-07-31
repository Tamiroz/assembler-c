#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define OK 1
#define ERR 0
#define LINE_SIZE 80
#define LABEL 35

typedef struct mcro {
  char label[LABEL];
  char *content;
} Macro;

/*so the main idea for the algorithm is, you go through the file, one line at a
time, assuming each line is max 80, then, for each line 1.check if its a macro
and if so save it to the macro table 1.1.check if the label is correct - not a
saved word 1.2 save each line to the content of the macro 1.3 when we get macro
end, stop saving lines, and go back to regular search #what do we have to check
for macro and macroend to make sure it is valid? 2.if its not a macro, check if
the label is in the macro table and if so, add the macro to the res file 3.when
in the end of the file close it and return it.*/

/*so lets begin with a function that gets a file, and reads it one line at a
 * time*/

void print_macro(Macro *m_list, int mcro_count);
int pre_assemble(char *file_name);

int main(void) {
  char file[] = "test.as";
  int res = pre_assemble(file);
  printf("%d", res);

  return OK;
}

int pre_assemble(char *file_name) {
  FILE *rfp;
  FILE *wfp;
  char new_file_name[LINE_SIZE];
  char line[LINE_SIZE];
  char work_line[LINE_SIZE];
  char *token;
  char mcro_count = 0;
  char mcro_lines = 0;
  char *content;
  char *m_label;
  Macro *m_list = NULL;

  if ((file_name = strtok(file_name, ".")) == NULL) {
    printf("error, invalid file name\n");
  } else if (strcmp(strtok(NULL, "."), "as") != 0) {
    printf("error, invalid file type\n");
  } else if (strtok(NULL, ".") != NULL) {
    printf("error, invalid file name\n");
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
    return ERR;
  }

  /*go through each line in the source file*/
  while (fgets(line, LINE_SIZE, rfp) != NULL) {
    /*copy the line to work line*/
    strcpy(work_line, line);
    token = strtok(work_line, " \t\n\r");

    /*handle case, macro*/
    if (token != NULL && strcmp(token, "mcro") == 0) {

      /*if it is a macro label we get more memory for another macro, save the
       * label if its legal*/
      mcro_count++;
      m_list = (Macro *)realloc(m_list, mcro_count * sizeof(Macro));
      if (m_list == NULL) {
        printf("error, memory allocation did not succeed");
        return ERR;
      }

      m_label = strtok(NULL, " \t\n\r");
      if (m_label == NULL) {
        printf("error, no label to macro");
        return ERR; //**must abort and clean memory */
      }
      if ((token = strtok(NULL, " \t\n\r")) !=
          NULL) { // checks if there is content after mcro label
        printf("error, too many labels to macro");
        return ERR; //**must abort and clean memory */
      }
      /*if (!is_legal_label(m_label))
      {
              printf("error, illegal label");
              return ERR;
      }*/

      /****we have to check if a label is legal, saved word or if there are
       already in mcrolist*****/

      strcpy(m_list[mcro_count - 1].label, m_label);

      /*load the next line into line,then as long as it isnt mcroend we add
       * memory and load it into it, then we assign into mcro.content*/

      content = (char *)malloc(LINE_SIZE);
      if (content == NULL) {
        printf("error, memory allocation did not succeed");
        return ERR;
      }
      /*make sure memory allocation succeded*/

      strcpy(content, "");
      mcro_lines = 0;
      fgets(line, LINE_SIZE, rfp);
      strcpy(work_line, line);
      token = strtok(work_line, " \t\n\r");

      while (token == NULL || strcmp(token, "mcroend") != 0) {
        mcro_lines++;
        content = (char *)realloc(content, mcro_lines * LINE_SIZE);
        if (content == NULL) {
          printf("error, memory allocation did not succeed");
          return ERR;
        }

        strcat(content, line);
        fgets(line, LINE_SIZE, rfp);

        strcpy(work_line, line);
        token = strtok(work_line, " \t\n\r");
      }
      if ((token = strtok(NULL, " \t\n\r")) !=
          NULL) { // check if there is more than macroend in the line
        printf("error, too many labels to macro");
        return ERR; //**must abort and clean memory */
      }
      m_list[mcro_count - 1].content = content;
    }
    /*else if ((macro_num = is_a_lable(mlist, work_line) != -1))****needs to be
    worked on,**** -- check if current token is a mcro label, check if its not a
    LABEL (eg. check if theere is no : after it. or content in general) and if
    so, pastes the content into the output file
    {
            fputs(mcro[macro_num].content, wfp);
    }*/
    else {
      fputs(line, wfp);
    }
  }
  print_macro(m_list, mcro_count);

  return OK;
}

void print_macro(Macro *m_list, int mcro_count) {
  int i;
  for (i = 0; i < mcro_count; i++) {
    printf("%s: \n", m_list[i].label);
    printf("%s\n", m_list[i].content);
  }
}

/*int is_a_lable(mcro mlist[], char label[]);*/ // checks if a given token is a
                                                // lable in the mcro list
