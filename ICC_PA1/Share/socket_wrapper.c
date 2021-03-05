// include headers ------------------------------------------------------------

#include <stdlib.h>

#include "error_mgr.h"
#include "socket_wrapper.h"

// enum -----------------------------------------------------------------------

typedef enum _bind_status
{
	BIND_SUCCESS = 0,
	BIND_FAILED
} bind_status;

// constants ------------------------------------------------------------------

static const char* LOCAL_HOST = "127.0.0.1"; 

// function declarations ------------------------------------------------------

error_code_t bind_to_port(SOCKET socket, int socket_port); 
struct sockaddr_in initialize_sockaddr(const char* str_ip, int port);

error_code_t send_message_with_certain_length(SOCKET communication_socket, char* msg_buffer, int msg_size);
error_code_t receive_message_with_certain_length(SOCKET communication_socket, char* msg_segment_buffer, int bytes_to_recv);

error_code_t change_buffer_size(char** p_buffer, int new_size);

error_code_t check_socket_creation_result(SOCKET new_socket, const char* file, int line, const char* func_name);
error_code_t check_socket_result(int socket_result, error_code_t error_code, const char* file, int line, const char* func_name);
error_code_t check_winsock_result(int winsock_result, error_code_t error_code, const char* file, int line, const char* func_name); 

// function implementations ---------------------------------------------------


/// initialize_winsock
/// inputs:  - 
/// outputs: error code
/// summary: Auxiliary function - configures WINSOCK ( initialize winsock ) 
/// 
error_code_t initialize_winsock()
{
	error_code_t status = SUCCESS_CODE;

	WSADATA wsa_data;

	int winsock_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	status = check_winsock_result(winsock_result, WINSOCK_INITIALIZATION_FAILED, __FILE__, __LINE__, __func__);

	return status;
}

/// deinitialize_winsock
/// inputs:  - 
/// outputs: error code
/// summary: Auxiliary function - configures WINSOCK ( deinitialize winsock ) 
/// 
error_code_t deinitialize_winsock()
{
	error_code_t status = SUCCESS_CODE;

	int winsock_result = WSACleanup();

	status = check_winsock_result(winsock_result, WINSOCK_DEINITIALIZATION_FAILED, __FILE__, __LINE__, __func__);

	return status;
}

/// initialize_client_socket
/// inputs:  socket , server_ip , server_port
/// outputs: error code
/// summary: If the socket exists - connects to the server
/// 
error_code_t initialize_socket(SOCKET* p_socket)
{
	error_code_t status = SUCCESS_CODE;

	SOCKET new_socket;

	new_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	status = check_socket_creation_result(new_socket, __FILE__, __LINE__, __func__);

	if (status != SUCCESS_CODE)
		return status;

	*p_socket = new_socket;

	return status;
}

/// set_socket_blocking_mode
/// inputs:  socket , blocking_mode
/// outputs: error code
/// summary: Auxiliary function - configures socket blocking mode 
/// 
error_code_t set_socket_blocking_mode(SOCKET my_socket, socket_blocking_mode blocking_mode)
{
	error_code_t status = SUCCESS_CODE;
	int socket_result;

	socket_result = ioctlsocket(my_socket, FIONBIO, &((u_long)blocking_mode));

	status = check_socket_result(socket_result, SOCKET_SET_BLOCKING_MODE_FAILED, __FILE__, __LINE__, __func__);

	return status;
}

/// set_socket_operation_timeout
/// inputs:  socket , operation_type, timeout
/// outputs: error code
/// summary: Auxiliary function - configures socket operation timeout 
/// 
error_code_t set_socket_operation_timeout(SOCKET my_socket, int operation_type, int timeout) 
{
	error_code_t status = SUCCESS_CODE;
	int socket_result;
	
	status = set_socket_blocking_mode(my_socket, BLOCKING_MODE);

	socket_result = setsockopt(my_socket, SOL_SOCKET, operation_type, (char*)&timeout, sizeof(timeout));

	status = check_socket_result(socket_result, SOCKET_SET_OP_TIMEOUT_FAILED, __FILE__, __LINE__, __func__);

	return status;
}

/// bind_to_port
/// inputs:  socket , socket_port
/// outputs: error code
/// summary: Performs a bind on the socket.
///          Checks whether it was successful or not and returns a message accordingly
///
error_code_t bind_to_port(SOCKET my_socket, int socket_port)
{
	error_code_t status = SUCCESS_CODE; 

	struct sockaddr_in service;

	service = initialize_sockaddr(LOCAL_HOST, socket_port);

	int bind_status = bind(my_socket, (struct sockaddr*)&service, sizeof(service));

	status = check_socket_result(bind_status, SOCKET_BIND_FAILED, __FILE__, __LINE__, __func__);

	return status;
}


/// initialize_sockaddr
/// inputs:  str_ip , port
/// outputs: struct sockaddr_in
/// summary: Auxiliary function - configures sockaddr
///
struct sockaddr_in initialize_sockaddr(const char* str_ip, int port)
{
	struct sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(str_ip);
	service.sin_port = htons(port);

	return service;
}

/// send_message
/// inputs:  communication_socket , msg_buffer, msg_size
/// outputs: error_code 
/// summary: Sends a message and checks if it was successful 
///
error_code_t send_message_to(SOCKET communication_socket, char* msg_buffer, int msg_size, char* dest_ip, int dst_port)
{
	error_code_t status = SUCCESS_CODE;
	
	status = send_message_with_certain_length(communication_socket, (char*)(&msg_size), sizeof(msg_size));

	if (status != SUCCESS_CODE)
		return status;

	status = send_message_with_certain_length(communication_socket, msg_buffer, msg_size);

	return status;
}

/// send_message_with_certain_length
/// inputs:  communication_socket , msg_buffer, msg_size
/// outputs: error_code 
/// summary: Sends a message of a certain size - each time sends a part of it. 
///          Checks whether it was successful 
///
error_code_t send_message_with_certain_length(SOCKET communication_socket, char* msg_buffer, int msg_size)
{
	error_code_t status = SUCCESS_CODE;
	int total_bytes_sent = 0;
	int bytes_sent = 0;

	while (total_bytes_sent < msg_size)
	{
		bytes_sent = send(communication_socket, msg_buffer, msg_size - total_bytes_sent, 0);

		if (bytes_sent == SOCKET_ERROR)
			return SOCKET_SEND_FAILED;

		total_bytes_sent += bytes_sent;
		msg_buffer += bytes_sent;
	}
	return status;
}

/// receive_message
/// inputs:  communication_socket ,  received_msg_buffer, p_received_msg_length
/// outputs: error_code 
/// summary: receive a message and checks if it was successful 
///
error_code_t receive_message_from(SOCKET communication_socket, char** p_received_msg_buffer, int* p_received_msg_length)
{
	error_code_t status = SUCCESS_CODE;

	char* received_msg_buffer = *p_received_msg_buffer;
	int msg_length = 0;

	status = receive_message_with_certain_length(communication_socket, (char*)&msg_length, sizeof(msg_length));

	if (status != SUCCESS_CODE)
		goto recv_msg_end;

	status = change_buffer_size(&received_msg_buffer, msg_length + 1); 

	if (status != SUCCESS_CODE)
		goto recv_msg_end;

	status = receive_message_with_certain_length(communication_socket, received_msg_buffer, msg_length);

	if (status != SUCCESS_CODE)
		goto recv_msg_end;

	received_msg_buffer[msg_length] = '\0';
	
recv_msg_end:
	*p_received_msg_buffer = received_msg_buffer;
	*p_received_msg_length = msg_length;

	return status; 
}

/// send_message_with_certain_length
/// inputs:  communication_socket , msg_buffer, msg_size
/// outputs: error_code 
/// summary: receive a message of a certain size - each time sends a part of it. 
///          Checks whether it was successful 
///
error_code_t receive_message_with_certain_length(SOCKET communication_socket, char* msg_segment_buffer, int bytes_to_recv)
{
	error_code_t status = SUCCESS_CODE;
	int total_bytes_recv = 0;
	int bytes_recv = 0;
	int recv_error = 0;

	while (total_bytes_recv < bytes_to_recv)
	{
		bytes_recv = recv(communication_socket, msg_segment_buffer, bytes_to_recv - total_bytes_recv, 0);

		if (bytes_recv == SOCKET_ERROR)
		{
			recv_error = WSAGetLastError();

			if (recv_error == WSAETIMEDOUT)
				return SOCKET_RECV_TIMEOUT;

			if (recv_error == WSAECONNRESET || recv_error == WSAENOTCONN ||
				recv_error == WSAENOTSOCK   || recv_error != WSAECONNABORTED)
				return SOCKET_CONNECTION_RESET;

			print_error(SOCKET_RECV_FAILED, __FILE__, __LINE__, __func__);
			return SOCKET_RECV_FAILED;
		}

		if (bytes_recv == 0) 
			return SOCKET_CONNECTION_CLOSED;

		total_bytes_recv += bytes_recv;
		msg_segment_buffer += bytes_recv;
	}
	return status;
}

/// change_buffer_size
/// inputs:  p_buffer , new_size
/// outputs: error_code 
/// summary: Auxiliary function - changes the size of a buffer by using realloc
///
error_code_t change_buffer_size(char** p_buffer, int new_size)
{
	error_code_t status = SUCCESS_CODE;
	char* buffer; 

	buffer = (char*)realloc(*p_buffer, new_size);

	status = check_mem_alloc(buffer, __FILE__, __LINE__, __func__);

	if (status == SUCCESS_CODE)
		*p_buffer = buffer;
	
	return status;
}

/// check_socket_creation_result
/// inputs:  new_socket , source_file, source_line, source_func_name
/// outputs: error_code 
/// summary: Checking whether new_socket creation was successful - If not print an error message
///
error_code_t check_socket_creation_result(SOCKET new_socket, const char* file, int line, const char* func_name)
{
	if (new_socket == INVALID_SOCKET)
	{
		print_error(SOCKET_CREATION_FAILED, file, line, func_name);
		return SOCKET_CREATION_FAILED;
	}
	return SUCCESS_CODE;
}

/// check_socket_result
/// inputs:  socket_result , error_code,  source_file, source_line, source_func_name
/// outputs: error_code 
/// summary: Checking socket result creation was successful - If not print an error message
///
error_code_t check_socket_result(int socket_result, error_code_t error_code, const char* file, int line, const char* func_name)
{
	if (socket_result == SOCKET_ERROR)
	{
		print_error(error_code, file, line, func_name);
		return error_code;
	}
	return SUCCESS_CODE;
}

/// check_winsock_result
/// inputs:  winsock_result , error_code,  source_file, source_line, source_func_name
/// outputs: error_code 
/// summary: Checking winsock result creation was successful - If not print an error message
///
error_code_t check_winsock_result(int winsock_result, error_code_t error_code, const char* file, int line, const char* func_name)
{
	if (winsock_result != 0)
	{
		print_error(error_code, file, line, func_name);
		return error_code;
	}
	return SUCCESS_CODE;
}
