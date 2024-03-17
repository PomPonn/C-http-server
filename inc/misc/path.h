#pragma once

/// @brief safely joins 'string' to 'origin' path
/// @param origin path to join the string to
/// @param origin_max_length maximum length of 'origin' path
/// @param string string to join
/// @returns 1 on success, 0 on failure
int path_join(char* const origin, int origin_max_length, const char* string);