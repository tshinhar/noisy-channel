#ifndef CHANNEL_H
#define CHANNEL_H

#define _CRT_SECURE_NO_WARNINGS
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


#define CHANNEL_ADDRESS "127.0.0.1"
#define SEND_STR_MAX_SIZE 
#define STATUS_CODE_FAILURE -1
#define STATUS_CODE_SUCCESS 0
#define STDIN 0
#define BUFF_SIZE 1500


int InitChannelSocket(SOCKET* server_socket, int port_num);
int DeinitializeSocket(SOCKET* socket);
int TunnelData(SOCKET channel_socket, int port_num, char* server_ip, int server_port_num, int porb, int seed);

#endif // CHANNEL_H
#pragma once