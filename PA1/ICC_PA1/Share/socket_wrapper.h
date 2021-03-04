#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H 

// include headers ------------------------------------------------------------

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#include <string.h>
#include "error_mgr.h"

// enum -----------------------------------------------------------------------

typedef enum _socket_blocking_mode {
	BLOCKING_MODE,
	NON_BLOCKING_MODE
} socket_blocking_mode;

typedef enum _client_choice {
	CONNECT_TO_SERVER = 1,
	EXIT_SERVER
}client_choice;

// constants ------------------------------------------------------------------

extern const char* MSG_CONNECTION_TO_SERVER_FAILED; 

extern const int PLAYER_DECISION_WAIT_TIME;
extern const int GAME_START_WAIT_TIME;
extern const int DEFAULT_WAIT_TIME;

// function declarations ------------------------------------------------------

error_code_t initialize_winsock();
error_code_t deinitialize_winsock();

error_code_t initialize_socket(SOCKET* p_socket); 

error_code_t send_message_to(SOCKET communication_socket, char* msg_buffer, int msg_size, char* dest_ip, int dest_port); 
error_code_t receive_message_from(SOCKET communication_socket, char** p_received_msg_buffer, int* p_received_msg_length);

error_code_t set_socket_blocking_mode(SOCKET my_socket, socket_blocking_mode blocking_mode); 
error_code_t set_socket_operation_timeout(SOCKET my_socket, int operation_type, int timeout); 

void shutdown_and_close_connection(SOCKET my_socket);


#endif // SOCKET_WRAPPER_H 
