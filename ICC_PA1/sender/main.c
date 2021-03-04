// include headers ------------------------------------------------------------

#include <stdlib.h> 
#include <stdbool.h>

#include "error_mgr.h"
#include "socket_wrapper.h" 
#include "server_client_messages_handler.h"
#include "client_game_handler.h"

// enum -----------------------------------------------------------------------

typedef enum _client_args_index {
    SERVER_IP_INDEX = 1,
    SERVER_PORT_INDEX,
    FILE_NAME_INDEX,
    SENDER_ARGS_NUM
}client_args_index;

// constants ------------------------------------------------------------------

static const char* FEEDBACK_MSG = "received: %d bytes\nwritten: %d bytes\ndetected & corrected %d errors";

// global variables -----------------------------------------------------------

static char client_name[MAX_NAME_SIZE + 1] = { 0 };

// function declarations ------------------------------------------------------

error_code_t handle_client_communication(SOCKET client_socket);
// error_code_t set_socket_timeout_parameter(SOCKET client_socket, message* p_client_message);


// function implementations ---------------------------------------------------

int main(int argc, char* argv[])
{
    error_code_t status = SUCCESS_CODE;

    status = check_args_num(argc, SENDER_ARGS_NUM);

    if (status != SUCCESS_CODE)
        return status;

    char* channel_ip = argv[SERVER_IP_INDEX];
    int channel_port = atoi(argv[SERVER_PORT_INDEX]);
    char* file_name = argv[FILE_NAME_INDEX];
   
    SOCKET sender_socket = INVALID_SOCKET;

    int received_bytes, written_bytes;
    int detected_errors_num, corrected_errors_num;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_socket(&sender_socket);

    if (status != SUCCESS_CODE)
         goto sender_clean_up;

    status = transfer_file(sender_socket, channel_ip, channel_port, file_name);

    if (status != SUCCESS_CODE)
        goto sender_clean_up;

    status = recv_feedback(sender_socket, &received_bytes, &written_bytes, &detected_errors_num, &corrected_errors_num);

    if (status != SUCCESS_CODE)
        goto sender_clean_up;

    fprintf(stderr, FEEDBACK_MSG, received_bytes, written_bytes, detected_errors_num, corrected_errors_num); 

sender_clean_up:

    if (sender_socket != INVALID_SOCKET)
        closesocket(sender_socket);

    deinitialize_winsock();
    return (int)status;
}

error_code_t transfer_file(SOCKET sender_socket, char* dest_ip, int dest_port, char* file_name)
{
    error_code_t status = SUCCESS_CODE;

    FILE* p_file = fopen(file_name, "rb");

    status = check_file_opening(p_file, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    while (end_of_file(p_file) == false) 
    {
        read_bits_from_file();

        encode_bits();

        send_bits();
    }

    if (p_file != NULL)
        fclose(p_file);

    return status;
}

error_code_t  recv_feedback(SOCKET sender_socket, int * p_received_bytes, int* p_written_bytes, int* p_detected_errors_num, int* p_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE;
    char* recv_buffer = NULL;

    status = recv_message(sender_socket, &recv_buffer);

    if (status != SUCCESS_CODE)
        return status;

    parse_feedback(recv_buffer, p_received_bytes, p_written_bytes, p_detected_errors_num, p_corrected_errors_num); 
    // sscanf 

    return status;
}


/*
do {


    status = set_socket_operation_timeout(client_socket, SO_RCVTIMEO, DEFAULT_WAIT_TIME);

    if (status != SUCCESS_CODE)
        goto client_clean_up;

    status = handle_client_communication(client_socket);

    if (status != SOCKET_RECV_TIMEOUT && status != SOCKET_SEND_FAILED
        && status != SOCKET_CONNECTION_RESET && status != CLIENT_CONNECTION_DENIED)
        goto client_clean_up;

    shutdown_and_close_connection(client_socket);

    if (status == CLIENT_CONNECTION_DENIED)
        msg_to_client = MSG_CONNECTION_TO_SERVER_DENIED;

    else msg_to_client = MSG_CONNECTION_TO_SERVER_FAILED;

    printf(msg_to_client, server_ip, server_port);
    scanf_s(" %d", &current_client_choice);

} while (current_client_choice == CONNECT_TO_SERVER);
*/

/*
/// handle_client_communication
/// inputs:  client_socket 
/// outputs: error_code
/// summary:  Performs the sending of messages for the client. 
///           sends a message and receives a message.
///           Depending on the type of message we will know whether to send twice or once
/// 
error_code_t handle_client_communication(SOCKET client_socket)
{
    error_code_t status = SUCCESS_CODE;

    message message_from_server = { 0 };
    message client_message = { 0 };

    char client_message_string[MAX_MSG_SIZE + 1];

    char* received_msg = NULL;
    int received_msg_length;

    status = send_client_request(client_socket, client_message_string, &client_message, client_name);

    while (status != SOCKET_CONNECTION_CLOSED && client_message.current_msg_type != CLIENT_DISCONNECT)
    {
        status = receive_message(client_socket, &received_msg, &received_msg_length);

        if (status != SUCCESS_CODE)
            goto client_communication_clean_up;

        // handle server message and get response for server
        status = handle_server_message(received_msg, client_message_string, &message_from_server, &client_message);

        if (status != SUCCESS_CODE)
            goto client_communication_clean_up;

        status = set_socket_timeout_parameter(client_socket, &client_message);

        if (status != SUCCESS_CODE)
            goto client_communication_clean_up;

        if (server_sends_additional_message(&message_from_server))
            continue;

        status = send_message(client_socket, client_message_string, strlen(client_message_string));

        if (status != SUCCESS_CODE)
            goto client_communication_clean_up;
    }

client_communication_clean_up:

    if (received_msg != NULL)
        free(received_msg);

    return status;
}
*/