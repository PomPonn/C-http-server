#include "structs/record.h"

#include "misc/utils.h"

char* records_str_get_value(record_str_t* records, int records_count, const char* key) {
  for (int i = 0; i < records_count; i++) {
    if (str_is_equal(records[i].key, key)) {
      return records[i].value;
    }
  }

  return (void*)0;
}