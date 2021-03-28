#include "channel.h"


int main(int argc, char* argv[])
{
	SOCKET channel_socket = INVALID_SOCKET;
	if (argc < 6) {
		printf("Not enough arguments.\n");
		return STATUS_CODE_FAILURE;
	}
	char* server_ip = argv[2];
	int server_port_num = atoi(argv[3]);
	int err_prob = atoi(argv[4]);
	int seed = atoi(argv[5]);
	if (InitChannelSocket(&channel_socket, atoi(argv[1])) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	if (TunnelData(channel_socket, atoi(argv[1]), server_ip, server_port_num, err_prob, seed) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	DeinitializeSocket(&channel_socket);

	return STATUS_CODE_SUCCESS;
}