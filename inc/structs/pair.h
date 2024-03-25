#pragma once

typedef struct pair_str {
  char* key;
  char* value;
} pair_str_t;

/// @brief returns value of given key in specified pairs array
/// @param pairs pointer to pairs array
/// @param pairs_count length of pairs array
/// @param key key to search for
char* pairs_str_get_value(pair_str_t* pairs, int pairs_count, const char* key);