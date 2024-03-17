#pragma once

#include <stdio.h>

typedef void (*ERROR_CALLBACK)(void);

#define TEMP_SIZE 8

// required temp[TEMP_SIZE] buffer to use
#define error_set_last_with_code(error_code, what_code) \
_itoa_s(what_code, temp, TEMP_SIZE, 10); \
error_set_last(error_code, temp); \

void error_set_output_file(FILE* output_file);

void error_set_callback(ERROR_CALLBACK callback);

void error_set_last(int error_code, const char* what);

int error_get_last_code();

void error_last_print_message();