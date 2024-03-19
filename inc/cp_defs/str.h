#pragma once

#ifdef _WIN32

#define ITOA(value, buffer, buffer_size, base) \
_itoa_s(value, buffer, buffer_size, base) \

#define STRCAT(str1, byte_size, str2) \
strcat_s(str1, byte_size, str2) \

#define STRNCPY(dest, dest_size, src, count) \
strncpy_s(dest, dest_size, src, count) \

#define FOPEN(fpp, pathname, modes) \
fopen_s(fpp, pathname, modes) \

#define SPRINTF(string, byte_size, format, ...) \
sprintf_s(string, byte_size, format, __VA_ARGS__) 

#elif __linux__

#define ITOA(value, buffer, buffer_size, base) \
itoa(value, buffer, base) \

#define STRCAT(str1, byte_size, str2) \
strcat(str1, str2) \

#define STRNCPY(dest, dest_size, src, count) \
strncpy(dest, src, count) \

#define FOPEN(fpp, filename, modes) \
*fpp = fopen(filename, modes) \

#define SPRINTF(string, byte_size, format, ...) \
sprintf(string, format, __VA_ARGS__) 

#endif