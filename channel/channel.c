#include "channel.h"


int flip_count = 0;

char* StringToBinary(char* s) {
	if (s == NULL)
		return NULL; // no input string
	size_t len = strlen(s);
	char* binary = (char *)malloc(len * 8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
	if (binary == NULL) {
		printf("Error- malloc failed");
		return NULL;
	}
	binary[0] = '\0';
	for (size_t i = 0; i < len; ++i) {
		char ch = s[i];
		for (int j = 7; j >= 0; --j) {
			if (ch & (1 << j)) {
				strcat(binary, "1");
			}
			else {
				strcat(binary, "0");
			}
		}
	}
	return binary;
}


char* BinaryToString(char* b) {
	if (b == NULL)
		return NULL; // no input string
	size_t len = strlen(b);
	char* str = malloc(len / 8 + 1); // this size is always correct since b was created from s string
	if (str == NULL) {
		printf("Error- malloc failed");
		return NULL;
	}
	str[0] = '\0';
	for (size_t i = 0; i < len; i += 8) {
		char ch[2] = { 0, '\0' };
		int bit_position = 7;
		for (size_t j = i; j < i + 8; j++) {
			if (b[j] == '1')
				ch[0] += pow(2, bit_position);
			bit_position--;
		}
		strcat(str, ch);
	}
	return str;
}

// calls remove parity for every 15 bits of the input
int FlipBinary(char* b, int prob, int seed) {
	if (b == NULL)
		return -1; // no input string
	srand((unsigned)seed);
	double rnd = 0;
	double p = prob / pow(2, 16);
	size_t len = strlen(b);
	for (size_t i = 0; i < len; i++) {
		rnd = (double)rand() / RAND_MAX;
		if(rnd < p){ //this is true with prob p 
			if (b[i] == '1')
				b[i] = '0';
			else
				b[i] = '1';
			flip_count++;
		}
	}
	return 0;
}


int InitChannelSocket(SOCKET* server_socket, int port_num) {
	// Initialize Winsock
	WSADATA wsa_data;
	unsigned long address;
	SOCKADDR_IN service;
	int status;

	status = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (status != NO_ERROR) {
		printf("WSAStartup failed: %d\n", status);
		return STATUS_CODE_FAILURE;
	}
	// Create a socket 
	*server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (*server_socket == INVALID_SOCKET) {
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		DeinitializeSocket(NULL);
		return STATUS_CODE_FAILURE;
	}


	address = inet_addr(CHANNEL_ADDRESS);
	if (address == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			CHANNEL_ADDRESS);
		DeinitializeSocket(server_socket);
		return STATUS_CODE_FAILURE;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = address;
	service.sin_port = htons(port_num);

	//bind
	status = bind(*server_socket, (SOCKADDR*)&service, sizeof(service));
	if (status == SOCKET_ERROR) {
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		DeinitializeSocket(server_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}


//closes the server socket
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


//wait for incoming data
int TunnelData(SOCKET channel_socket, int port_num, char* server_ip, int server_port_num, int porb, int seed) {
	fd_set readfds;
	char recv_buffer[BUFF_SIZE];
	char* binary_array = NULL;
	char* output_data = NULL;
	int transfer_count = 0;
	SOCKADDR_IN server_addr;
	unsigned long server_address;
	int server_struct_length = sizeof(server_addr);
	SOCKADDR_IN client_addr = {0};
	SOCKADDR_IN sender_addr;
	int sender_struct_length = sizeof(sender_addr);
	//create server address
	server_address = inet_addr(CHANNEL_ADDRESS);
	if (server_address == INADDR_NONE) {
		printf("Error converting string to ip\n");
		return STATUS_CODE_FAILURE;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_address;
	server_addr.sin_port = htons(server_port_num);

	while (TRUE){ 
		//wait for incoming data
		int count = recvfrom(channel_socket, recv_buffer, BUFF_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_struct_length);
		if (count < 0) {
			printf("Error receiving data\n");
		}
		//check if the sender is the server
		if (sender_addr.sin_port == htons(server_port_num)) {//message from server - end
			sendto(channel_socket, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
			fprintf(stderr, "sender: %s\n", inet_ntoa(client_addr.sin_addr));
			fprintf(stderr, "reciver: %s\n", server_ip);
			fprintf(stderr, "%d bytes, %d flipped bits\n", transfer_count, flip_count);
			free(binary_array);
			free(output_data);
			return STATUS_CODE_SUCCESS;
		}
		//if we get here the message is from the client
		//save client addr
		transfer_count += count;
		client_addr.sin_family = AF_INET;
		client_addr.sin_addr.s_addr = sender_addr.sin_addr.s_addr;
		client_addr.sin_port = sender_addr.sin_port;
		binary_array = StringToBinary(recv_buffer);//convert the buffer to binary array
		FlipBinary(binary_array, porb, seed);//filp with given prob and seed 
		output_data = BinaryToString(binary_array);// convert back to bytes
		sendto(channel_socket, output_data, BUFF_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)); //send to server
	}

}
