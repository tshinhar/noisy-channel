#ifndef CLIENT_H
#define CLIENT_H

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


#define SEND_STR_MAX_SIZE 
#define STATUS_CODE_FAILURE -1
#define STATUS_CODE_SUCCESS 0
#define STDIN 0
#define BUFF_SIZE 1500
#define CLIENT_ADDRESS "127.0.0.1"
#define CLIENT_PORT_NUM 8888

/*INPUTS: ptr to client socket
OUTPUT : exit status & openning client socket
In this function we bind() the socket to port "8888"*/
int InitClientSocket(SOCKET* client_socket);

/*Inputs: ptr socket
free socket*/
int DeinitializeSocket(SOCKET* socket);

/*INPUTS : ptr to str , path inputs file
OUTPUTS: this function will return pointer to string that will represent the data+hamming */
char* InitData(char* strData, char* input_file);

/*Inputs : socket of client , string of data after hamming , port number of channel*/
/*outputs : send data and recv data from server*/
int SendData(SOCKET client_socket, char* strData, char* ip, int port);

#endif // CLIENT
#pragma once
