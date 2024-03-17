#include "misc/path.h"

#include "misc/utils.h"

int path_join(char* const origin, int origin_max_length, const char* string)
{
  int origin_len = str_length(origin);
  // guards
  if (!origin || !string || (origin_len + str_length(string) > origin_max_length)) {
    return 0;
  }

  if (origin[origin_len - 1] == '/' && string[0] == '/') {
    str_concat(origin, origin_max_length, string + 1);
  }
  else {
    str_concat(origin, origin_max_length, string);
  }

  return 1;
}