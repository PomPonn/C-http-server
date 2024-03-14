#pragma once

/// @brief extracts line from the string buffer
/// @param strbuf the string buffer to extract line from
/// @param linebuff contains extracted line (optional)
/// @param linebuff_size defines linebuff maximum size (if linebuf=NULL, set this to 0)
/// @return pointer to next line or NULL if there was an error
char* get_buffer_line(char* strbuf, char* linebuf, unsigned int linebuff_size);

/// @brief read file in specified path in binary mode
/// @param path path to the file
/// @param content returns file content
/// @return error code
int read_file(const char* const path, char** const content);

/// @brief frees allocated resource
/// @param content pointer to the resource
void free_resource(char* content);

int get_working_directory(char* buffer, int buffer_size);