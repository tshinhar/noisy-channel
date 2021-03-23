#include "server.h"


char* RemoveParity(int num_of_extra, int ltot, char* bits_array, int* e_count, int* fix_count) {
	char static rem[15] = { 0 };
	int ss, sss, error = 0, pos_of_orisig = 0, pos_of_redsig = 0;

	// checking whether there are any errors
	for (int i = 0; i < num_of_extra; i++)
	{
		int count = 0, value = 0;
		int position = (int)pow(2, i);
		ss = position - 1;
		while (ss < ltot)
		{
			for (sss = ss; sss < ss + position; sss++)
			{
				if (bits_array[sss] == '1')
					count++;
			}
			ss = ss + 2 * position;
		}
		if (count % 2 != 0) { 
			error += position; 
			e_count++;
		}
	}

	// navigating to the error bits and correct them
	if (error != 0) {// since only one error is fixable, this is correct
		printf("Error detected and corrected!\n");
		if (bits_array[error - 1] == '1')// flip the bit with the error
			bits_array[error - 1] = '0'; 
		else  
			bits_array[error - 1] = '1'; 
		for (int i = 0; i < ltot; i++)
		{
			if (i == ((int)pow(2, pos_of_orisig) - 1))
			{
				pos_of_orisig++;
			}
			else
			{
				rem[pos_of_redsig] = bits_array[i];
				pos_of_redsig++;
			}
		}
		fix_count++;
	}
	// removing the extra parity bits from the sequences without the errors
	else {
		for (int i = 0; i < ltot; i++)
		{
			if (i == ((int)pow(2, pos_of_orisig) - 1))
			{
				pos_of_orisig++;
			}
			else
			{
				rem[pos_of_redsig] = bits_array[i];
				pos_of_redsig++;
			}
		}
	}
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
				strcat_s(binary, strlen(binary), "1");
			}
			else {
				strcat_s(binary, strlen(binary), "0");
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
			if (b[j] == "1")
				ch[0] += pow(2, bit_position);
			bit_position--;
			strcat_s(str, strlen(str), ch);
		}
	}
	return str;
}

// calls remove parity for every 15 bits of the input
char* DecodeBinary(char* b, int* e_count, int* fix_count) {
	if (b == NULL)
		return NULL; // no input string
	size_t len = strlen(b);
	char* str = malloc(len);
	if (str == NULL) {
		printf("Error- malloc failed");
		return NULL;
	}
	str[0] = '\0';
	char bin_array[15] = { 0 };
	int counter = 0;
	for (size_t i = 0; i < len; i += 8) {
		bin_array[counter] = b[i];
		counter++;
		if (i % 15 == 0) {//we completed 15 iterations, send to decode and reset counter
			char* decoded_array = RemoveParity(4, 15, bin_array, e_count, fix_count);
			strcat_s(str, strlen(str), decoded_array);
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
int RecvData(SOCKET server_socket, char* file_name, int port_num) {
	FILE* output_file = NULL;
	fd_set readfds;
	char recv_buffer[BUFF_SIZE];
	char* binary_array = NULL;
	char* output_data = NULL;
	int byte_counter = 0, write_counter = 0, error_counter = 0, fixed_counter = 0;
	SOCKADDR_IN client_addr;
	int client_struct_length = sizeof(client_addr);
	if (fopen_s(&output_file, file_name, "w") != 0) {
		printf("Error- can't open file");
		return STATUS_CODE_FAILURE;
	}
	while (TRUE) {
		FD_ZERO(&readfds);
		FD_SET(_fileno(stdin), &readfds);
		FD_SET(port_num, &readfds);
		select(port_num + 1, &readfds, NULL, NULL, NULL);
		if (FD_ISSET(_fileno(stdin), &readfds)) {
			char input[4];
			scanf_s("%s", input, 4);
			if (input[3] == '\0') {// since the input is string this is always true, but just in case
				if (strcmp(input, "End") == 0) {//End recived from user, clean up and return
					// make an array with the needed parameters and send it back
					int send_array[4] = { byte_counter, write_counter, error_counter, fixed_counter };
					sendto(server_socket, send_array, sizeof(send_array), 0, (struct sockaddr*)&client_addr, client_struct_length);
					printf("received: %d bytes\n", byte_counter);
					printf("wrote: %d bytes\n", write_counter);
					printf("detected %d errors\n", error_counter);
					printf("corrected %d errors\n", fixed_counter);
					free(binary_array);
					free(output_data);
					if (fclose(output_file) == 0) {
						printf("Error - can't close file\n");
						return STATUS_CODE_FAILURE;
					}
					return STATUS_CODE_SUCCESS;
				}
			}
		}
		if (FD_ISSET(port_num, &readfds)) {//if this is true we can call recv
			int count = recvfrom(server_socket, recv_buffer, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &client_struct_length);
			if (count < 0) {
				printf("Error receiving data\n");
			}
			byte_counter += count;//count is the number of bytes written to the buffer
			binary_array = StringToBinary(recv_buffer);//convert the buffer to binary array
			binary_array = DecodeBinary(binary_array, &error_counter, &fixed_counter);//fix errors and remove parity
			output_data = BinaryToString(binary_array);// convert back to bytes
			if (fwrite(output_data, 1, sizeof(output_data), output_file) == 0) {// write to file
				printf("Error writing to file\n");
				return STATUS_CODE_FAILURE;
			}
		}
	}

}
