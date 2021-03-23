
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

#define CONSOLE_COMMAND_SIZE 20 
#define FEEDBACK_MSG_MAX_SIZE 50 
#define PRINTABLE_CHARS_TRESHOLD 127 

static const char* RECEIVER_END_MSG = "Type 'End' when done\n";
static const char* EXIT_COMMAND = "End"; 

static const char* SUMMARY_MSG = "\nreceived: %d bytes\nwrote: %d bytes\ndetected & corrected %d error\n";

static const int RECV_TIMEOUT = 100;

// function declarations ------------------------------------------------------

error_code_t recv_file(char* file_name, SOCKET receiver_socket, char* channel_ip, int* p_channel_port, int* p_detected_and_corrected_errors_num); 

void get_user_input(char* console_command, unsigned int console_command_size);
error_code_t write_bytes_to_file(FILE** p_p_file, char* file_buffer, int bytes_to_write, int* p_bytes_written);
error_code_t send_feedback(SOCKET receiver_socket, char* channel_ip, int channel_port, int bytes_received, int bytes_written, int detected_and_corrected_errors_num);




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

    status = initialize_socket(&receiver_socket);

    if (status != SUCCESS_CODE)
        goto receiver_clean_up;

    // set timeout for recv operation 
    status = set_socket_operation_timeout(receiver_socket, SO_RCVTIMEO, RECV_TIMEOUT);

    if (status != SUCCESS_CODE)
        goto receiver_clean_up;

    status = bind_to_port(receiver_socket, receiver_port);

    if (status != SUCCESS_CODE)
        goto receiver_clean_up;

    // receive and write file from sender 
    status = recv_file(file_name, receiver_socket, channel_ip, &channel_port, &bytes_received, &bytes_written, &detected_and_corrected_errors_num);

        if (status != SUCCESS_CODE)
            return status;

    // send feedback back to the sender 
    status = send_feedback(receiver_socket, channel_ip, channel_port, bytes_received, bytes_written, detected_and_corrected_errors_num);

    if (status != SUCCESS_CODE)
        goto receiver_clean_up;

    fprintf(stderr, SUMMARY_MSG, bytes_received, bytes_written, detected_and_corrected_errors_num);

receiver_clean_up:

    if (receiver_socket != INVALID_SOCKET)
        closesocket(receiver_socket);

    deinitialize_winsock();
    return (int)status;
}

//---------------------------------


//---------------------------------

error_code_t recv_file(char* file_name, SOCKET receiver_socket, char* channel_ip, int* p_channel_port, int* p_total_bytes_received, int* p_total_bytes_written, int* p_detected_and_corrected_errors_num)
{

    error_code_t status = SUCCESS_CODE;

    char* data_buffer = NULL;
    int data_buffer_size = 0;

    char* received_msg_buffer = NULL;
    int msg_length = 0;

    int bytes_written = 0;

    bool is_txt_file = true;

    FILE* p_file_write;

    fopen_s(&p_file_write, file_name, "wb");

    status = check_file_opening(p_file_write, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    char console_command[CONSOLE_COMMAND_SIZE+1] = { 0 };

    while (true)
    {
        get_user_input(console_command, CONSOLE_COMMAND_SIZE);

        if (strcmp(console_command, EXIT_COMMAND) == 0)
            break;

        //else Sleep(KEYBOARD_POLLING_TIME);

        status = receive_message_from(receiver_socket, &received_msg_buffer, &msg_length, channel_ip, p_channel_port);

        if (status == SOCKET_RECV_TIMEOUT)
            continue;

        if (status != SUCCESS_CODE)
            goto recv_file_clean_up;

        data_buffer_size = (int)(floor((float)msg_length * HAMMING_DATA_BLOCK_SIZE / HAMMING_ENCODED_BLOCK_SIZE));

        status = change_buffer_size(&data_buffer, data_buffer_size);

        if (status != SUCCESS_CODE)
            goto recv_file_clean_up;

        (*p_detected_and_corrected_errors_num) += decode_data(received_msg_buffer, msg_length, data_buffer, data_buffer_size);

        //------------------------------------------------
      //  printf("%s", received_msg_buffer);
        //------------------------------------------------

        // CHECK FILE TYPE 

        is_txt_file = check_if_txt_file(data_buffer, data_buffer_size); 

        status = write_bytes_to_file(&p_file_write, data_buffer, data_buffer_size, &bytes_written);

        if (status != SUCCESS_CODE)
            goto recv_file_clean_up;

       (*p_total_bytes_received) += msg_length;
       (*p_total_bytes_written) += bytes_written;

    }

    status = rename_file(p_file_write, is_txt_file);

recv_file_clean_up: 

    if (received_msg_buffer != NULL)
        free(received_msg_buffer);

    if (data_buffer != NULL)
        free(data_buffer);

    if (p_file_write != NULL)
        fclose(p_file_write);

    if (status == SOCKET_RECV_TIMEOUT)
        status = SUCCESS_CODE;

    return status; 

}

void get_user_input(char* console_command, unsigned int console_command_size)
{
    if (_kbhit() != 0)
    {
        scanf_s(" %s", console_command, console_command_size);
        /*
        char c = getchar();

        // if there is enough space in the buffer, concatenate the char to the end of it 
        if (strlen(console_command) < console_command_size)
            sprintf_s(console_command, console_command_size, "%s%c", console_command, c);

        // if char is new line, user finished command --> free buffer 
        if (c == '\n')
            *console_command = '\0';
        */ 
    }
}

bool check_if_txt_file(char* data_buffer, int data_buffer_size)
{
    for (int i = 0; i < data_buffer_size; i++)
    {
        if (data_buffer[i] > PRINTABLE_CHARS_TRESHOLD)
            return false;
    }
    return true; 
}

error_code_t write_bytes_to_file(FILE** p_p_file, char* file_buffer, int bytes_to_write, int* p_bytes_written)
{
    *p_bytes_written = fwrite(file_buffer, sizeof(char), bytes_to_write, *p_p_file);

    if (*p_bytes_written < bytes_to_write)
    {
        print_error(FILE_WRITING_FAILED, __FILE__, __LINE__, __func__);
        return FILE_WRITING_FAILED;
    }

    return SUCCESS_CODE;
}

error_code_t send_feedback(SOCKET receiver_socket, char* channel_ip, int channel_port, int bytes_received, int bytes_written, int detected_and_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE; 

    char msg_buffer[FEEDBACK_MSG_MAX_SIZE + 1] = { 0 };

    sprintf(msg_buffer, "%d %d %d", bytes_received, bytes_written, detected_and_corrected_errors_num);

    status = send_message_to(receiver_socket, msg_buffer, strlen(msg_buffer) + 1, channel_ip, channel_port);

    return status;
}

//write_bytes_to_file(&p_file_write, file_buffer, bytes_counter); 
