#define _CRT_SECURE_NO_WARNINGS
#include "Client.h"


int main(int argc, char* argv[])
{
	SOCKET client_socket = INVALID_SOCKET;
	char* strData = NULL;

	if (argc < 4) {
		printf("Not enough arguments.\n");
		return STATUS_CODE_FAILURE;
	}

	strData = InitData(strData, argv[3]);
	if (NULL == strData) {
		printf("Error with memory allocations\n");
		return STATUS_CODE_FAILURE;
	}
	if (InitClientSocket(&client_socket) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}

	if (SendData(client_socket, strData, argv[1], atoi(argv[2])) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	DeinitializeSocket(client_socket);
	free(strData);

	return STATUS_CODE_SUCCESS;

}
