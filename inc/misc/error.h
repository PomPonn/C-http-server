#pragma once

#include <stdio.h>

typedef void (*ERROR_CALLBACK)(void);

#define TEMP_SIZE 8

#define error_last_print_message() \
error_print_message(error_get_last_code())

// required temp[TEMP_SIZE] buffer to use
#define error_set_last_with_code(error_code, what_code) \
_itoa_s(what_code, temp, TEMP_SIZE, 10); \
error_set_last(error_code, temp); \

/// @brief sets output file for error messages
/// @param output_file file to write to
void error_set_output_file(FILE* output_file);

/// @brief sets callback whenever there is an error recorded
/// @param callback callback function to call
void error_set_callback(ERROR_CALLBACK callback);

/// @brief sets last occured error
/// @param error_code
/// @param what additional information to show at the end of the error message
void error_set_last(int error_code, const char* what);

/// @returns last occured error code
int error_get_last_code();

/// @brief prints error code message to the file set by error_set_output_file() (by default it's stderr)
void error_print_message(int err_code);