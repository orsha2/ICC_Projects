
// include headers ------------------------------------------------------------
#include <conio.h> 
#include <stdlib.h> 
#include <stdbool.h>

#include "error_mgr.h"
#include "socket_wrapper.h" 

// enum -----------------------------------------------------------------------

typedef enum _receiver_args_index {
    RECEIVER_PORT_INDEX = 1,
    FILE_NAME_INDEX,
    RECEIVER_ARGS_NUM
}receiver_args_index;


// constants ------------------------------------------------------------------

static const char* RECEIVER_END_MSG = "Type 'End' when done\n";
static const char* SUMMARY_MSG = "\nreceived: %d bytes\nwrote: %d bytes\ndetected & corrected %d error";

// function declarations ------------------------------------------------------

void write_bytes_to_file(FILE** p_p_file, char* file_buffer, int bytes_counter); 


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

    char* received_msg_buffer = NULL;
    int msg_length = 0;

    char channel_ip[STR_IP_SIZE + 1];
    int channel_port;

    int received_bytes, written_bytes;
    int detected_errors_num, corrected_errors_num;

    printf(RECEIVER_END_MSG);

    FILE* p_file_write; 

    fopen_s(&p_file_write, file_name, "wb");

    status = check_file_opening(p_file_write, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_socket(&receiver_socket);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    status = bind_to_port(receiver_socket, receiver_port);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    // char console_command[20];

    while (true) 
    {
        /*
        printf("1");
        if (_kbhit() != 0)
        {
            scanf_s(" %s", console_command, 20);
            if (strcmp(console_command, "End") == 0)
                break;
        }
        //else Sleep(KEYBOARD_POLLING_TIME);
        */

        status = receive_message_from(receiver_socket, &received_msg_buffer, &msg_length, channel_ip, &channel_port);

        if (status != SUCCESS_CODE)
            goto channel_clean_up;

        if (strcmp(received_msg_buffer, "exit") == 0)
            break;

        // decode_bits(); 

        //------------------------------------------------
      //  printf("%s", received_msg_buffer);
        //------------------------------------------------

        write_bytes_to_file(&p_file_write, received_msg_buffer, msg_length);
    }

    //------------------------------------------------
    printf("\n%s\n%d\n", channel_ip, channel_port);
    //------------------------------------------------

    //status = recv_feedback(channel_socket, &received_bytes, &written_bytes, &detected_errors_num, &corrected_errors_num);

    //if (status != SUCCESS_CODE)
    //    goto sender_clean_up;

    // fprintf(stderr, FEEDBACK_MSG, received_bytes, written_bytes, detected_errors_num, corrected_errors_num); 

channel_clean_up:

    if (receiver_socket != INVALID_SOCKET)
        closesocket(receiver_socket);

    if (received_msg_buffer != NULL)
        free(received_msg_buffer);

    if (p_file_write != NULL)
        fclose(p_file_write);

    deinitialize_winsock();
    return (int)status;
}

//---------------------------------


//---------------------------------

void write_bytes_to_file(FILE** p_p_file, char* file_buffer, int bytes_counter)
{
    fwrite(file_buffer, sizeof(char), bytes_counter, *p_p_file);
}
//write_bytes_to_file(&p_file_write, file_buffer, bytes_counter); 
