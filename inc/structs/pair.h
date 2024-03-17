#pragma once

typedef struct pair_str {
  char* key;
  char* value;
} pair_str_t;

// looks for a key in set of pairs and returns its value (NULL if not found)
char* pairs_str_get_value(pair_str_t* pairs, int pairs_count, const char* key);