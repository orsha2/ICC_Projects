// include headers ------------------------------------------------------------

#include <stdlib.h> 
#include <stdbool.h>
#include <math.h> 

#include "error_mgr.h"
#include "socket_wrapper.h" 
#include "hamming_code_handler.h"

// enum -----------------------------------------------------------------------

typedef enum _sender_args_index {
    CHANNEL_IP_INDEX = 1,
    CHANNEL_PORT_INDEX,
    FILE_NAME_INDEX,
    SENDER_ARGS_NUM
}sender_args_index;

// constants ------------------------------------------------------------------

static const char* SUMMARY_MSG = "received: %d bytes\nwritten: %d bytes\ndetected & corrected %d errors\n";

// function declarations ------------------------------------------------------

error_code_t transfer_file(SOCKET sender_socket, char* dest_ip, int dest_port, char* file_name);
bool read_bytes_from_file(FILE** p_p_file, char* data_buffer, int bytes_to_read, int* p_bytes_counter); 
error_code_t recv_feedback(SOCKET sender_socket, int* p_received_bytes, int* p_written_bytes, int* p_detected_and_corrected_errors_num); 

// function implementations ---------------------------------------------------

int main(int argc, char* argv[])
{
    error_code_t status = SUCCESS_CODE;
    status = check_args_num(argc, SENDER_ARGS_NUM);

    if (status != SUCCESS_CODE)
        return status;

    char* channel_ip = argv[CHANNEL_IP_INDEX];
    int channel_port = atoi(argv[CHANNEL_PORT_INDEX]);
    char* file_name = argv[FILE_NAME_INDEX];
   
    SOCKET sender_socket = INVALID_SOCKET;

    int received_bytes, written_bytes;
    int detected_and_corrected_errors_num;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;
    
    status = initialize_socket(&sender_socket);

    if (status != SUCCESS_CODE)
         goto sender_clean_up;

    status = transfer_file(sender_socket, channel_ip, channel_port, file_name);

    if (status != SUCCESS_CODE)
        goto sender_clean_up;

   // printf("done\n"); 

    status = recv_feedback(sender_socket, &received_bytes, &written_bytes, &detected_and_corrected_errors_num);

    if (status != SUCCESS_CODE)
        goto sender_clean_up;

     fprintf(stderr, SUMMARY_MSG, received_bytes, written_bytes, detected_and_corrected_errors_num);

sender_clean_up:

    if (sender_socket != INVALID_SOCKET)
        closesocket(sender_socket);

    deinitialize_winsock();
    return (int)status;
}

error_code_t transfer_file(SOCKET sender_socket, char* dest_ip, int dest_port, char* file_name)
{
    error_code_t status = SUCCESS_CODE;

    FILE* p_file; 
    fopen_s(&p_file, file_name, "rb");

    status = check_file_opening(p_file, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    char data_buffer[DATA_BUFFER_SIZE];
    char encoded_data_buffer[ENCODED_DATA_BUFFER_SIZE];

    int bytes_counter = 0; 
    int encoded_data_buffer_size; 
    bool is_end_of_file = false;

    while (is_end_of_file == false)
    {
        is_end_of_file = read_bytes_from_file(&p_file, data_buffer, DATA_BUFFER_SIZE, &bytes_counter);

        encoded_data_buffer_size = (int)(ceil((float)bytes_counter * HAMMING_ENCODED_BLOCK_SIZE / HAMMING_DATA_BLOCK_SIZE));

        encode_data(data_buffer, bytes_counter, encoded_data_buffer, encoded_data_buffer_size);
        
        Sleep(UDP_SYNC_DELAY * 2);

        status = send_message_to(sender_socket, encoded_data_buffer, encoded_data_buffer_size, dest_ip, dest_port);

        if (status != SUCCESS_CODE)
            goto transfer_file_clean_up;

        bytes_counter = 0;
    }

transfer_file_clean_up:
    if (p_file != NULL)
        fclose(p_file);
    
    return status;
}

error_code_t recv_feedback(SOCKET sender_socket, int* p_received_bytes, int* p_written_bytes, int* p_detected_and_corrected_errors_num)
{
    error_code_t status = SUCCESS_CODE;
    char* received_msg_buffer = NULL;
    int msg_length = 0;
    int fields_converted = 0; 

    status = receive_message_from(sender_socket, &received_msg_buffer, &msg_length, NULL, NULL); 

    if (status == SUCCESS_CODE)
    {
        fields_converted = sscanf_s(received_msg_buffer, " %d %d %d", p_received_bytes, p_written_bytes, p_detected_and_corrected_errors_num);

        if (fields_converted == EOF || fields_converted != 3)
        {
            print_error(STRING_PARSING_FAILED, __FILE__, __LINE__, __func__);
            return STRING_PARSING_FAILED;
       }
    }

    if (received_msg_buffer != NULL)
        free(received_msg_buffer); 

    return status;
}

bool read_bytes_from_file(FILE** p_p_file, char* data_buffer, int bytes_to_read, int* p_bytes_counter)
{
    *p_bytes_counter = fread(data_buffer, sizeof(char), bytes_to_read, *p_p_file);

    if (*p_bytes_counter == bytes_to_read)
        return false;

    return true;
}
