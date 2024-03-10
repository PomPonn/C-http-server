#pragma once

/// @brief extracts line from the string buffer
/// @param strbuf
/// @param linebuff contains extracted line
/// @param linebuff_sizen defines linebuff maximum size
/// @return pointer to next line or NULL if there was an error
char* get_buffer_line(char* strbuf, char* linebuf, unsigned int linebuff_size);