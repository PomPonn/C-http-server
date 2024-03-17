#include "structs/pair.h"

#include "misc/utils.h"

char* pairs_str_get_value(pair_str_t* pairs, int pairs_count, const char* key) {
  for (int i = 0; i < pairs_count; i++) {
    if (str_is_equal(pairs[i].key, key)) {
      return pairs[i].value;
    }
  }

  return (void*)0;
}