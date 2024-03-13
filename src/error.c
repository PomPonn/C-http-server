#include "error.h"

int g_last_error = -1;
const char* g_what = NULL;
FILE* g_output_file;

void error_set_output_file(FILE* output_file)
{
  g_output_file = output_file;
}

void error_set_last(int error_code, const char* what)
{
  g_last_error = error_code;
  g_what = what;
}

int error_get_last_code()
{
  return g_last_error;
}

void error_last_print_message()
{
  if (!g_last_error) return;

  if (!g_output_file) {
    g_output_file = stderr;
  }

  fprintf(g_output_file, "[SERVER ERROR] ");

  switch (g_last_error)
  {
  case 1:
    fprintf(g_output_file, "Inavild arguments passed to the function: %s\n", g_what);
    break;
  case 2:
    fprintf(g_output_file, "accept() call failed with code: %s\n", g_what);
    break;
  case 3:
    fprintf(g_output_file, "select() call failed with code: %s\n", g_what);
    break;
  case 4:
    fprintf(g_output_file, "WSAStartup() call failed in fuction: %s\n", g_what);
    break;
  case 5:
    fprintf(g_output_file, "getaddrinfo() call failed with code: %s\n", g_what);
    break;
  case 6:
    fprintf(g_output_file, "Failed to create listen socket with code: %s\n", g_what);
    break;
  case 7:
    fprintf(g_output_file, "listen() call failed with code: %s\n", g_what);
    break;
  case 8:
    fprintf(g_output_file, "recv() call failed with code: %s\n", g_what);
    break;
  case 9:
    fprintf(g_output_file, "Failed to parse http protocol in function: %s\n", g_what);
    break;
  case 10:
    fprintf(g_output_file, "send() call failed with code: %s\n", g_what);
    break;
  default:
    fprintf(g_output_file, "Undefined error: %d", g_last_error);
    break;
  }
}