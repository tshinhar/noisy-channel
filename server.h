#ifndef SERVER_H
#define SERVER_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>


#define SERVER_ADDRESS "127.0.0.1"
#define SEND_STR_MAX_SIZE 
#define STATUS_CODE_FAILURE -1
#define STATUS_CODE_SUCCESS 0
#define STDIN 0
#define BUFF_SIZE 1500


DWORD WaitForConnection(LPVOID lpParam);
int InitServerSocket(SOCKET* server_socket, int port_num);
int DeinitializeSocket(SOCKET* socket);
int RecvData(SOCKET* server_socket, char* file_name, int port_num);

#endif // SERVER_H
#pragma once