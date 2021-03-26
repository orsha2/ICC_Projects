// include headers ------------------------------------------------------------
#include <conio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>

#include "error_mgr.h"
#include "socket_wrapper.h" 
#include "hamming_code_handler.h"

// enum -----------------------------------------------------------------------

typedef enum _receiver_args_index {
    RECEIVER_PORT_INDEX = 1,
    FILE_NAME_INDEX,
    RECEIVER_ARGS_NUM
}receiver_args_index;

// constants ------------------------------------------------------------------

#define CONSOLE_COMMAND_SIZE 100 
#define FEEDBACK_MSG_MAX_SIZE 50 
#define PRINTABLE_CHARS_THRESHOLD 128 

static const char* RECEIVER_END_MSG = "Type 'End' when done\n";
static const char* EXIT_COMMAND = "End"; 

static const char* SUMMARY_MSG = "\nreceived: %d bytes\nwrote: %d bytes\ndetected & corrected %d error\n";

static const int RECV_TIMEOUT = 100; // receive message timeout, in milliseconds 

// function declarations ------------------------------------------------------

error_code_t init_receiver_socket(SOCKET *p_receiver_socket, int recv_timeout, int receiver_port);

error_code_t recv_file(char* file_name, SOCKET receiver_socket, char* channel_ip, int* p_channel_port, int* p_total_bytes_received, int* p_total_bytes_written, int* p_detected_and_corrected_errors_num); 

error_code_t write_bytes_to_file(FILE** p_p_file, char* data_buffer, int bytes_to_write, int* p_bytes_written);

error_code_t send_feedback(SOCKET receiver_socket, char* channel_ip, int channel_port, int bytes_received, int bytes_written, int detected_and_corrected_errors_num);

error_code_t recv_and_write_bytes(SOCKET receiver_socket, char* channel_ip, int* p_channel_port,
                                  FILE* p_file, char** p_received_msg_buffer, int* msg_length,
                                  char** p_data_buffer, int* p_data_buffer_size, int* p_bytes_written,  int* p_detected_and_corrected_errors_num);

// function implementations ---------------------------------------------------


int main(int argc, char* argv[])
{
    error_code_t status = SUCCESS_CODE;

    status = check_args_num(argc, RECEIVER_ARGS_NUM);

    if (status != SUCCESS_CODE)
        return status;

    int receiver_port = atoi(argv[RECEIVER_PORT_INDEX]);
    char* file_name = argv[FILE_NAME_INDEX];

    SOCKET receiver_socket = INVALID_SOCKET;

    char channel_ip[STR_IP_SIZE + 1];
    int channel_port;

    int bytes_received = 0, bytes_written = 0;
    int detected_and_corrected_errors_num = 0;

    printf(RECEIVER_END_MSG);

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = init_receiver_socket(&receiver_socket, RECV_TIMEOUT, receiver_port);

    if (status != SUCCESS_CODE)
        return status;

    // receive and write file from sender 
    status = recv_file(file_name, receiver_socket, channel_ip, &channel_port, &bytes_received, &bytes_written, &detected_and_corrected_errors_num);

    if (status != SUCCESS_CODE)
        return status;

    // send feedback back to the sender 
    status = send_feedback(receiver_socket, channel_ip, channel_port, bytes_received, bytes_written, detected_and_corrected_errors_num);

    if (status != SUCCESS_CODE)
        goto receiver_clean_up;

    // print the summary message 
    fprintf(stderr, SUMMARY_MSG, bytes_received, bytes_written, detected_and_corrected_errors_num);

receiver_clean_up:

    if (receiver_socket != INVALID_SOCKET)
        closesocket(receiver_socket);

    deinitialize_winsock();

    return (int)status;
}

/// init_receiver_socket
/// inputs:  p_receiver_socket, recv_timeout, receiver_port
/// outputs: error_code_t
/// summary: initializes the receiver's socket, binds it to port (receiver_port) 
///          and sets its recv operation timeout 
/// 
error_code_t init_receiver_socket(SOCKET *p_receiver_socket, int recv_timeout, int receiver_port)
{
    error_code_t status = SUCCESS_CODE;

    status = initialize_socket(p_receiver_socket);

    if (status != SUCCESS_CODE)
        return status;

    // set timeout for recv operation 
    status = set_socket_operation_timeout(*p_receiver_socket, SO_RCVTIMEO, recv_timeout);

    if (status != SUCCESS_CODE)
        return status;

    // bind the socket to the receiver's port 
    status = bind_to_port(*p_receiver_socket, receiver_port);

    return status;
}

/// recv_file
/// inputs:  file_name, receiver_socket, channel_ip, p_channel_port, p_total_bytes_received,
///          p_total_bytes_written,p_total_detected_and_corrected_errors_num
/// outputs: error_code_t
/// summary: Opens the file for writing. 
///          Receives information from the channel, decodes it and writes to the file.
///          If an interrupt is received - "END" from the keyboard - stops the file transfer and returns 
/// 
error_code_t recv_file(char* file_name, SOCKET receiver_socket, char* channel_ip, int* p_channel_port, int* p_total_bytes_received, int* p_total_bytes_written, int* p_total_detected_and_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE;

    char* data_buffer = NULL;
    int data_buffer_size = 0;

    char* received_msg_buffer = NULL;
    int msg_length = 0;

    int bytes_written = 0;
    int detected_and_corrected_errors_num = 0; 

    FILE* p_file; 

    fopen_s(&p_file, file_name, "wb");

    status = check_file_opening(p_file, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    char console_command[CONSOLE_COMMAND_SIZE+1] = { 0 };
    
    while (true)
    {
        // checks for user input 
        if (_kbhit() != 0) 
        {
            scanf_s(" %s", console_command, CONSOLE_COMMAND_SIZE);
            
            // if user input is the "END" command --> exit the loop 
            if (_stricmp(console_command, EXIT_COMMAND) == 0)
                break; 
        }

        // receive bytes from channel, decode them and write them to file 
        status = recv_and_write_bytes(receiver_socket, channel_ip, p_channel_port, p_file, &received_msg_buffer, &msg_length, &data_buffer, &data_buffer_size, &bytes_written, &detected_and_corrected_errors_num);

        // if recv timeout --> check again for user input and for messages from the channel 
        if (status == SOCKET_RECV_TIMEOUT)
            continue;

        if (status != SUCCESS_CODE)
            goto recv_file_clean_up;

        // update the bytes received, bytes written and errors counters s
        (*p_total_detected_and_corrected_errors_num) += detected_and_corrected_errors_num;
        (*p_total_bytes_received) += msg_length;
        (*p_total_bytes_written) += bytes_written; 
    }

recv_file_clean_up: 

    if (received_msg_buffer != NULL)
        free(received_msg_buffer);

    if (data_buffer != NULL)
        free(data_buffer);

    if (p_file != NULL)
        fclose(p_file);

    if (status == SOCKET_RECV_TIMEOUT)
        status = SUCCESS_CODE;

    return status; 
}

/// recv_and_write_bytes
/// inputs:  receiver_socket, channel_ip, p_channel_port,
///          p_file,p_received_msg_buffer, p_msg_length,
///          p_data_buffer,p_data_buffer_size, p_bytes_written,
///          p_detected_and_corrected_errors_num
/// outputs: error_code_t
/// summary: Receives information from the channel and decodes it according to Hammimg Code.
///          After decoding, writes the information to the file.
/// 
error_code_t recv_and_write_bytes(SOCKET receiver_socket, char* channel_ip, int* p_channel_port, FILE* p_file, char** p_received_msg_buffer, int* p_msg_length, char** p_data_buffer, int* p_data_buffer_size, int* p_bytes_written, int* p_detected_and_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE;
    
    // receive message from channel
    status = receive_message_from(receiver_socket, p_received_msg_buffer, p_msg_length, channel_ip, p_channel_port);

    if (status != SUCCESS_CODE)
        return status;

    // change the data buffer size according to the message received 
    (*p_data_buffer_size) = (int)(floor((float)(*p_msg_length) * HAMMING_DATA_BLOCK_SIZE / HAMMING_ENCODED_BLOCK_SIZE));

    status = change_buffer_size(p_data_buffer, *p_data_buffer_size);

    if (status != SUCCESS_CODE)
        return status;

    // decodes the message (according to Hamming Code)
    (*p_detected_and_corrected_errors_num) = decode_data(*p_received_msg_buffer, *p_msg_length, *p_data_buffer, *p_data_buffer_size);

    // writes the decoded bytes to file 
    status = write_bytes_to_file(&p_file, *p_data_buffer, *p_data_buffer_size, p_bytes_written);

    if (status != SUCCESS_CODE)
        return status; 
}

/// write_bytes_to_file
/// inputs:  p_p_file, data_buffer, bytes_to_write, p_bytes_written
/// outputs: error_code_t 
/// summary: Writes data_buffer to *p_p_file
///          If didn't write all the bytes (bytes_written is smaller than bytes_to_write) - return error 
/// 
error_code_t write_bytes_to_file(FILE** p_p_file, char* data_buffer, int bytes_to_write, int* p_bytes_written)
{
    *p_bytes_written = fwrite(data_buffer, sizeof(char), bytes_to_write, *p_p_file);

    if (*p_bytes_written < bytes_to_write)
    {
        print_error(FILE_WRITING_FAILED, __FILE__, __LINE__, __func__);
        return FILE_WRITING_FAILED;
    }

    return SUCCESS_CODE;
}

/// send_feedback
/// inputs:  receiver_socket, channel_ip, channel_port, bytes_received,
///          bytes_written, detected_and_corrected_errors_num)
/// outputs: error_code_t
/// summary: Sends the feedback from receiver to the sender (through the channel) 
/// 
error_code_t send_feedback(SOCKET receiver_socket, char* channel_ip, int channel_port, int bytes_received, int bytes_written, int detected_and_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE; 

    char msg_buffer[FEEDBACK_MSG_MAX_SIZE + 1] = { 0 };

    snprintf(msg_buffer, FEEDBACK_MSG_MAX_SIZE, "%d %d %d", bytes_received, bytes_written, detected_and_corrected_errors_num);

    status = send_message_to(receiver_socket, msg_buffer, strlen(msg_buffer) + 1, channel_ip, channel_port);

    return status;
}
