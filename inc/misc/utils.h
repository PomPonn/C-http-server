#pragma once

// returns pointer to the first occurrence of char c in string str
char* str_find_char(const char* str, const char c);

// returns pointer to last occurrence of char c in string str
char* str_find_char_reversed(const char* str, const char c);

/// @brief converts 'value' to string, and stores the result in 'buffer'
/// @param buffer_size byte size of the buffer
void int_to_string(int value, char* const buffer, int buffer_size);

// return 1 if string are equal, otherwise 0
int str_is_equal(const char* str1, const char* str2);

// return length of string (0 if NULL is passed)
int str_length(const char* str);

// return 0 if both strings are equal, 1 if first is longer and -1 if last is longer
int str_compare(const char* str1, const char* str2);

/// @brief concat 'source' string to 'destination' string
/// @param byte_size byte size of destination buffer
void str_concat
(char* destination, unsigned int byte_size, const char* source);

/// @brief retrieves line from the string buffer
/// @param strbuff the string buffer to retrieve line from
/// @param linebuff contains extracted line (optional)
/// @param linebuff_size defines maximum size of 'linebuff' (if linebuf=NULL, set this to 0)
/// @return pointer to the next line, NULL if there are no more lines or if there was an error
char* get_buffer_line(char* strbuff, char* linebuff, unsigned int linebuff_size);

/// @brief read whole file in specified path in binary mode
/// @param path path to the file
/// @param content contains read file content
/// @return < 0 -> error code; >= 0 -> size of content
int read_file(const char* const path, char** const content);

/// @brief frees read file content
/// @param content pointer to the read file content
void free_content(char* content);

/// @brief stores current working directory in 'buffer'
/// @return 1 if successful, 0 otherwise
int get_working_directory(char* buffer, int buffer_size);

/// @brief retrieves file extension string
/// @param filepath path to the file (or just a filename)
/// @param extension buffer to write extension string to
void get_file_extension(const char* const filepath, char* const extension);