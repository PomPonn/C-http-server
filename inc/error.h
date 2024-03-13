#pragma once

#include <stdio.h>

void error_set_output_file(FILE* output_file);

void error_set_last(int error_code, const char* what);

int error_get_last_code();

void error_last_print_message();