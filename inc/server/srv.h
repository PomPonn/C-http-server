#pragma once

#ifdef _WIN32
typedef unsigned long long SRV_SERVER;
#elif __linux__
typedef int SRV_SERVER;
#endif
typedef int SRV_RESULT;

#define SRV_RESULT_OK 0

SRV_SERVER srv_server_create(const char* host, const char* port);

void srv_server_close(SRV_SERVER server);

SRV_RESULT srv_server_listen(SRV_SERVER server);

