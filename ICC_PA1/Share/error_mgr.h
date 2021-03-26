#ifndef ERROR_MGR_H
#define ERROR_MGR_H

// include headers ------------------------------------------------------------

#include <stdio.h>

// enum -----------------------------------------------------------------------

typedef enum {
    SUCCESS_CODE,
    ARGS_NUM_ERROR,
    NOT_VALID_ARGS,
    MEM_ALLOC_ERROR,

    FILE_OPEN_FAILED,
    FILE_READING_FAILED,
    FILE_WRITING_FAILED,

    STRING_PARSING_FAILED,

    CLOSE_HANDLE_FAILED,

    WINSOCK_INITIALIZATION_FAILED,
    WINSOCK_DEINITIALIZATION_FAILED,
	SOCKET_CREATION_FAILED,
    SOCKET_BIND_FAILED,
	SOCKET_LISTEN_FAILED,
    SOCKET_CONNECT_FAILED,
	SOCKET_SEND_FAILED,
	SOCKET_RECV_FAILED,
    SOCKET_RECV_TIMEOUT,
    SOCKET_SET_BLOCKING_MODE_FAILED,
    SOCKET_SET_OP_TIMEOUT_FAILED,
	SOCKET_CONNECTION_CLOSED,
    SOCKET_CONNECTION_RESET

} error_code_t;

// function declarations ------------------------------------------------------

void print_error(error_code_t error_code, const char* file, int line, const char* func_name);

error_code_t check_args_num(int argc, int anticipated_args_num); 

error_code_t check_mem_alloc(void* ptr, const char* file, int line, const char* func_name);

error_code_t check_file_opening(void* file_ptr, const char* file, int line, const char* func_name);


#endif // ERROR_MGR_H
