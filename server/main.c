#include "server.h"


int main(int argc, char* argv[])
{
	SOCKET server_socket = INVALID_SOCKET;

	if (argc < 3) {
		printf("Not enough arguments.\n");
		return STATUS_CODE_FAILURE;
	}

	if (InitServerSocket(&server_socket, atoi(argv[1])) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	printf("Type \"End\" when done\n");
	if (RecvData(server_socket, argv[2], atoi(argv[1])) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	DeinitializeSocket(&server_socket);

	return STATUS_CODE_SUCCESS;
}