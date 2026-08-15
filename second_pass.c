#include "second_pass.h"
#include "first_pass.h"

/*this file will handle second pass
-run through the code image and entry list and fill
in the values of the labels*/

int second_pass(const char *filename) {
  int i;
  for (i = 0; i < code_size; i++) {
    if (code_image[i].complete == 0) {
    }
  }
  /*fill in labels in image code*/

  /*fill labels in entry list*/

  return 0;
}