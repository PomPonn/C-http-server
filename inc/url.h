#pragma once

#define _URL_PROT_SIZE 32
#define _URL_HOSTNAME_SIZE 256
#define _URL_PORT_SIZE 8
#define _URL_TLD_SIZE 64

typedef struct url_parts {
  char protocol[_URL_PROT_SIZE];
  char hostname[_URL_HOSTNAME_SIZE];
  char port[_URL_PORT_SIZE];
  char TLD[_URL_TLD_SIZE];
  char* path;
  char* query_string;
} url_parts;

void free_url_parts(url_parts* url);

/// @brief safely append string to url
/// @param url url to work on
/// @param max_url_size maximum size of url string
/// @param string_to_append
void url_append(char* const url, int max_url_size, const char* const string_to_append);

/// @brief parses specified url
/// @param url url to parse
/// @param parsed_url struct containing parsed url elements
void url_parse(char* const url, url_parts* const parsed_url);