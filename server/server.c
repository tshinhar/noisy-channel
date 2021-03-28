#include "server.h"


int fixed_counter = 0;

char* RemoveParity(char* bits_array) {
	char static rem[12] = { 0 };
	rem[11] = '\0';
	char c, c1, c2, c3, c4;
	c1 = bits_array[0] ^ bits_array[2] ^ bits_array[4] ^ bits_array[6] ^ bits_array[8] ^ bits_array[10] ^ bits_array[12] ^ bits_array[14]; // haming code of first place 
	c2 = bits_array[1] ^ bits_array[2] ^ bits_array[5] ^ bits_array[6] ^ bits_array[9] ^ bits_array[10] ^ bits_array[13] ^ bits_array[14];
	c3 = bits_array[3] ^ bits_array[4] ^ bits_array[5] ^ bits_array[6] ^ bits_array[11] ^ bits_array[12] ^ bits_array[13] ^ bits_array[14];
	c4 = bits_array[7] ^ bits_array[8] ^ bits_array[9] ^ bits_array[10] ^ bits_array[11] ^ bits_array[12] ^ bits_array[13] ^ bits_array[14];

	c = c1 + 2*c2 + 4*c3 + 8*c4;
	if (c != 0) {//error found, fixing it
		if (bits_array[c - 1] == '1')
			bits_array[c - 1] = '0';
		else
			bits_array[c - 1] = '1';
		fixed_counter++;
	}
	//remove all parity bits 
	rem[0] = bits_array[2];
	rem[1] = bits_array[4];
	rem[2] = bits_array[5];
	rem[3] = bits_array[6];
	rem[4] = bits_array[8];
	rem[5] = bits_array[9];
	rem[6] = bits_array[10];
	rem[7] = bits_array[11];
	rem[8] = bits_array[12];
	rem[9] = bits_array[13];
	rem[10] = bits_array[14];
	return rem;
}



char* StringToBinary(char* s) {
	if (s == NULL) 
		return NULL; // no input string
	size_t len = strlen(s);
	char* binary = malloc(len * 8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
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
	for (size_t i = 0; i < len; i+=8) {
		char ch[2] = {0, '\0'};
		int bit_position = 7;
		for (size_t j = i; j < i+8; j++) {
			if (b[j] == '1')
				ch[0] += (char)pow(2, bit_position);
			bit_position--;
		}
		strcat(str, ch);
	}
	return str;
}

// calls remove parity for every 15 bits of the input
char* DecodeBinary(char* b) {
	if (b == NULL)
		return NULL; // no input string
	size_t len = strlen(b);
	char* str = malloc(len);
	int err_count = 0, fixed_count = 0;
	if (str == NULL) {
		printf("Error- malloc failed");
		return NULL;
	}
	str[0] = '\0';
	char bin_array[15] = { 0 };
	int counter = 0;
	for (size_t i = 0; i < len; i++) {
		bin_array[counter] = b[i];
		counter++;
		if ((i+1) % 15 == 0) {//we completed 15 iterations, send to decode and reset counter
			char* decoded_array = RemoveParity(bin_array);
			strcat(str, decoded_array);
			counter = 0;
		}
	}
	return str;
}


int InitServerSocket(SOCKET* server_socket, int port_num) {
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


	address = inet_addr(SERVER_ADDRESS);
	if (address == INADDR_NONE) {
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS);
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
		if (closesocket(*socket) == SOCKET_ERROR) {
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
			return STATUS_CODE_FAILURE;
		}
	}

	result = WSACleanup();
	if (result != 0) {
		printf("WSACleanup failed: %d\n", result);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}


//wait for incoming data
int RecvData(SOCKET server_socket, char* file_name, int port_num) {
	FILE* output_file = NULL;
	fd_set readfds;
	char recv_buffer[BUFF_SIZE];
	char* binary_array = NULL;
	char* output_data = NULL;
	int byte_counter = 0, write_counter = 0;
	SOCKADDR_IN client_addr;
	int client_struct_length = sizeof(client_addr);
	struct timeval tv = { 5, 0 };
	output_file = fopen(file_name, "wb");
	if (output_file == NULL) {
		printf("Error- can't open file");
		return STATUS_CODE_FAILURE;
	}
	while (TRUE) {
		FD_ZERO(&readfds);
		FD_SET(server_socket, &readfds);
		int status = select(port_num + 1, &readfds, NULL, NULL, &tv);
		if (kbhit()) {
			char input[4];
			scanf_s("%s", input, 4);
			if (input[3] == '\0') {// since the input is string this is always true, but just in case
				if (strcmp(input, "End") == 0) {//End recived from user, clean up and return
					// make an array with the needed parameters and send it back
					char send_array[BUFF_SIZE];
					sprintf(send_array, "received: %d bytes\nwritten: %d bytes\ndetected & corrected: %d\n", byte_counter, write_counter, fixed_counter);
					sendto(server_socket, send_array, sizeof(send_array), 0, (struct sockaddr*)&client_addr, client_struct_length);
					fprintf(stderr, "received: %d bytes\n", byte_counter);
					fprintf(stderr, "wrote: %d bytes\n", write_counter);
					fprintf(stderr, "detected & corrected: %d errors\n", fixed_counter);
					free(binary_array);
					free(output_data);
					if (output_file != NULL) {
						if (fclose(output_file) != 0) {
							printf("Error - can't close file\n");
							return STATUS_CODE_FAILURE;
						}
					}
					return STATUS_CODE_SUCCESS;
				}
			}
		}
		if (FD_ISSET(server_socket, &readfds)) {//if this is true we can call recv
			int count = recvfrom(server_socket, recv_buffer, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &client_struct_length);
			if (count < 0) {
				printf("Error receiving data\n");
			}
			byte_counter += count;//count is the number of bytes written to the buffer
			binary_array = StringToBinary(recv_buffer);//convert the buffer to binary array
			binary_array = DecodeBinary(binary_array);//fix errors and remove parity
			output_data = BinaryToString(binary_array);// convert back to bytes
			if (fwrite(output_data, sizeof(char), strlen(output_data), output_file) != strlen(output_data)){// write to file
				printf("Error writing to file\n");
				return STATUS_CODE_FAILURE;
			}
			write_counter += strlen(output_data);
		}
	}

}
