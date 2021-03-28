
#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"



char* Hamming_To_Str(char* hamArr);
char* Hamming(char* arr);
int InitClientSocket(SOCKET* client_socket);
int DeinitializeSocket(SOCKET* socket);
long int Get_File_Size(char* input_file);
char* InitData(char* bit_arr, char* input_file);




int InitClientSocket(SOCKET* client_socket) {
	// Initialize Winsock
	WSADATA wsa_data;
	unsigned long address;
	SOCKADDR_IN myclient;
	int status;

	status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (status != NO_ERROR) {
		printf("WSAStartup failed: %d\n", status);
		return STATUS_CODE_FAILURE;
	}
	// Create a socket 
	*client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (*client_socket == INVALID_SOCKET) {
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		DeinitializeSocket(NULL);
		return STATUS_CODE_FAILURE;
	}

	address = inet_addr(CLIENT_ADDRESS); // 
	if (address == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			CLIENT_ADDRESS);
		DeinitializeSocket(client_socket);
		return STATUS_CODE_FAILURE;
	}

	myclient.sin_family = AF_INET;
	myclient.sin_addr.s_addr = address;
	myclient.sin_port = htons(CLIENT_PORT_NUM);

	//bind - bind client socket to port 8888
	status = bind(*client_socket, (SOCKADDR*)&myclient, sizeof(myclient));
	if (status == SOCKET_ERROR) {
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		DeinitializeSocket(client_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
int DeinitializeSocket(SOCKET* socket) {
	int result;

	if (*socket != INVALID_SOCKET) {
		if (closesocket(*socket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		return STATUS_CODE_FAILURE;
	}

	result = WSACleanup();
	if (result != 0) {
		printf("WSACleanup failed: %d\n", result);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
long int Get_File_Size(char* input_file)
{
	int size = 0;
	FILE* fp = fopen(input_file, "r");
	if (fp == NULL) {
		printf("Error- can't open file");
		return STATUS_CODE_FAILURE;
	}
	fseek(fp, 0L, SEEK_END);
	// calculating the size of the file
	long int res = ftell(fp);
	// closing the file
	fclose(fp);
	return res;
}
char* InitData(char* bit_arr, char* input_file)
{
	long int size_file = Get_File_Size(input_file); // size of bytes
	char* arr = malloc(size_file * 8 + 1);// "array" of 8*size elements. every elem is is one bit
	if (NULL == arr)
		return NULL;
	char* buffer = malloc(size_file + 1); // buffer of whole file
	FILE* fp = fopen(input_file, "rb");
	if (NULL == fp)
		return NULL;
	fread(buffer, 1, size_file, fp); // reading the whole file into buffer
	buffer[size_file] = '\0';
	int temp = 0;
	arr[0] = '\0';
	for (int i = 0; i < size_file; i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			temp = (buffer[i] & (1 << j));
			if (temp >= 1)
				strcat(arr, "1");
			else
				strcat(arr, "0");

		}

	}
	free(buffer);
	return Hamming(arr);
}
char* Hamming(char* arr)
{
	int size_hamArr = strlen(arr) + 4 * (((strlen(arr)) / 11)); // adding 4 bits to each block of 11 bits
	char* hamArr = malloc(size_hamArr + 1);
	if (NULL == hamArr)
		return NULL;
	char* hamArrReturned = hamArr;

	// Hamming calculation and insertion
	int num_of_iterations = strlen(arr) / 11;
	for (int i = 0; i < num_of_iterations; i++)
	{
		hamArr[2] = arr[0];
		hamArr[4] = arr[1];
		hamArr[5] = arr[2];
		hamArr[6] = arr[3];
		hamArr[8] = arr[4];
		hamArr[9] = arr[5];
		hamArr[10] = arr[6];
		hamArr[11] = arr[7];
		hamArr[12] = arr[8];
		hamArr[13] = arr[9];
		hamArr[14] = arr[10];
		/*Hamming codes*/
		hamArr[0] = arr[0] ^ arr[1] ^ arr[3] ^ arr[4] ^ arr[6] ^ arr[8] ^ arr[10]; // haming code of first place 
		hamArr[1] = arr[0] ^ arr[2] ^ arr[3] ^ arr[5] ^ arr[6] ^ arr[9] ^ arr[10];
		hamArr[3] = arr[1] ^ arr[2] ^ arr[3] ^ arr[7] ^ arr[8] ^ arr[9] ^ arr[10];
		hamArr[7] = arr[4] ^ arr[5] ^ arr[6] ^ arr[7] ^ arr[8] ^ arr[9] ^ arr[10];

		arr = arr + 11;
		hamArr = hamArr + 15;
	}
	hamArrReturned[size_hamArr] = '\0';
	return Hamming_To_Str(hamArrReturned);
}
char* Hamming_To_Str(char* hamArr)
{
	int size = strlen(hamArr) / 8 + 1;
	char* hammStr = malloc(size);
	if (NULL == hammStr)
		return NULL;
	int pos = 0;
	char c[2] = { '\0','\0' };
	hammStr[0] = '\0';
	for (int i = 0; i < size; i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			if (hamArr[j] == '1')
				c[0] = c[0] + pow(2, pos);
			pos++;
			//strcat(bufferString, "1");
		}
		pos = 0;
		strcat(hammStr, c);
		c[0] = '\0';
		hamArr = hamArr + 8;
	}
	//free(hamArr);
	return hammStr;

}
int SendData(SOCKET client_socket, char* strData, char* ip, int port)
{
	int sizeData = strlen(strData);
	int bytes_sent = 0;
	int bytes_left = sizeData;
	int addr_ip;
	int count = 0;
	int countRecv = 0;
	int num_setSoc = 0;
	char recvData[1500];
	fd_set readfds;
	SOCKADDR_IN serverAddr;
	addr_ip = inet_addr(ip);
	if (addr_ip == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			ip);
		DeinitializeSocket(client_socket);
		return STATUS_CODE_FAILURE;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = addr_ip;
	serverAddr.sin_port = htons(port);
	struct timeval tv = { 1, 0 };


	/*select() - checking if i got msg on client port(it was binded before) . if yes - break to recvfrom()*/

	while (TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(client_socket, &readfds);
		num_setSoc = select(0, &readfds, NULL, NULL, &tv);
		if (FD_ISSET(client_socket, &readfds))// we got msg on the client port == we recv data and exit
			break;
		if (bytes_left > 1) { // we have data to send
			count = sendto(client_socket, strData, 1500, 0, &serverAddr, sizeof(serverAddr));
			if (count == -1)
			{
				printf("********************ERROR : send data failed!*****************\n");
				return STATUS_CODE_FAILURE;
			}
			bytes_left = sizeData - count;			bytes_sent = bytes_sent + count;
		}

	}


	countRecv = recvfrom(client_socket, recvData, sizeof(recvData), 0, NULL, NULL); // NULL - should we replace it ? in udp these argument are optional
	if (countRecv == -1) {
		printf("error %ld\n", WSAGetLastError());
		printf("********************ERROR: recive data failed!************************\n");
		return STATUS_CODE_FAILURE;
	}
	fprintf(stderr, "%s", recvData);

	return STATUS_CODE_SUCCESS;

}