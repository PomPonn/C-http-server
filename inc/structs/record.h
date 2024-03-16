#pragma once

typedef struct record_str {
  char* key;
  char* value;
} record_str_t;

// looks for a key in set of records and returns its value (NULL if not found)
char* records_str_get_value(record_str_t* records, int records_count, const char* key);