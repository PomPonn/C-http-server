#include "misc/error.h"

// global variables
int g_last_error = 0;
const char* g_what = NULL;
FILE* g_output_file = NULL;

// callbacks
ERROR_CALLBACK err_cb = NULL;

void error_set_output_file(FILE* output_file) {
  g_output_file = output_file;
}

void error_set_last(int error_code, const char* what) {
  g_last_error = error_code;
  g_what = what;

  // run error callback
  if (err_cb)
    err_cb();
}

void error_set_callback(ERROR_CALLBACK callback) {
  err_cb = callback;
}

int error_get_last_code() {
  return g_last_error;
}

void error_last_print_message() {
  if (!g_last_error) return;

  if (!g_output_file) {
    g_output_file = stderr;
  }

  fprintf(g_output_file, "[SERVER ERROR] ");

  switch (g_last_error)
  {
  case 1:
    fprintf(g_output_file, "Invalid arguments passed to the function: %s\n", g_what);
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
  case 11:
    fprintf(g_output_file, "socket bind() call failed with code: %s\n", g_what);
    break;
  case 12:
    fprintf(g_output_file, "ioctlsocket() call failed with code: %s\n", g_what);
    break;
  case 13:
    fprintf(g_output_file, "call timed limit expired in function: %s\n", g_what);
    break;
  case 14:
    fprintf(g_output_file, "failed to set control handler with code: %s\n", g_what);
    break;
  case 15:
    fprintf(g_output_file, "failed to get main thread handle with code: %s\n", g_what);
    break;
  case 16:
    fprintf(g_output_file, "failed to shutdown socket with code: %s\n", g_what);
    break;
  case 17:
    fprintf(g_output_file, "failed to close socket with code: %s\n", g_what);
    break;
  default:
    fprintf(g_output_file, "Undefined error: %d", g_last_error);
    break;
  }
}