
// include headers ------------------------------------------------------------

#include <stdlib.h> 
#include <stdbool.h>

#include "error_mgr.h"
#include "socket_wrapper.h" 

// enum -----------------------------------------------------------------------

typedef enum _channel_args_index {
    CHANNEL_PORT_INDEX = 1,
    RECEIVER_IP_INDEX,
    RECEIVER_PORT_INDEX,
    BIT_FLIP_PROBABILITY_INDEX,
    RANDOM_SEED_INDEX,
    CHANNEL_ARGS_NUM
}channel_args_index;

// constants ------------------------------------------------------------------

static const char* FEEDBACK_MSG = "sender: %s\nreceiver: %s\n%d bytes, flipped %d bits";

// function declarations ------------------------------------------------------
error_code_t transfer_messages(SOCKET channel_socket, char* receiver_ip, int receiver_port, char* sender_ip, int* p_sender_port); 

// function implementations ---------------------------------------------------

int main(int argc, char* argv[])
{
    error_code_t status = SUCCESS_CODE;

    status = check_args_num(argc, CHANNEL_ARGS_NUM);

    if (status != SUCCESS_CODE)
        return status;

    int channel_port = atoi(argv[CHANNEL_PORT_INDEX]);
    char* receiver_ip = argv[RECEIVER_IP_INDEX];
    int receiver_port = atoi(argv[RECEIVER_PORT_INDEX]);

    int bit_flip_probability = atoi(argv[BIT_FLIP_PROBABILITY_INDEX]);
    int random_seed = atoi(argv[RANDOM_SEED_INDEX]);

    SOCKET channel_socket = INVALID_SOCKET;

    int transferred_bytes = 0, flipped_bits = 0;

    char sender_ip[STR_IP_SIZE + 1];
    int sender_port;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_socket(&channel_socket);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    status = bind_to_port(channel_socket, channel_port);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    status = transfer_messages(channel_socket, receiver_ip,  receiver_port, sender_ip, &sender_port);

        if (status != SUCCESS_CODE)
            goto channel_clean_up;


    printf("\n%s\n%d\n", sender_ip, sender_port);

    //status = transfer_file(channel_socket, channel_ip, channel_port, file_name);

    //if (status != SUCCESS_CODE)
    //    goto sender_clean_up;

    //status = recv_feedback(channel_socket, &received_bytes, &written_bytes, &detected_errors_num, &corrected_errors_num);

    //if (status != SUCCESS_CODE)
    //    goto sender_clean_up;

   // fprintf(stderr, FEEDBACK_MSG, sender_ip, receiver_ip, transferred_bytes, flipped_bits);

channel_clean_up:

    if (channel_socket != INVALID_SOCKET)
        closesocket(channel_socket);


    deinitialize_winsock();
    return (int)status;
}

error_code_t transfer_messages(SOCKET channel_socket, char* receiver_ip, int receiver_port, char* sender_ip, int* p_sender_port)
{

    error_code_t status = SUCCESS_CODE; 
    char* received_msg_buffer = NULL;
    int msg_length = 0;

    while (true)
    {
        status = receive_message_from(channel_socket, &received_msg_buffer, &msg_length, sender_ip, p_sender_port);

        if (status != SUCCESS_CODE)
            break;

        Sleep(30);

        // flip_bits(received_msg_buffer, bit_flip_probability, random_seed); 

        //-----------------
        if (strcmp(received_msg_buffer, "exit") == 0)
            break;
        //-----------------
        status = send_message_to(channel_socket, received_msg_buffer, msg_length, receiver_ip, receiver_port);


        if (status != SUCCESS_CODE)
            break;

        //  printf("%s", received_msg_buffer);
    }

    if (received_msg_buffer != NULL)
        free(received_msg_buffer);

    return status; 
}

/*

0 <= num <= 2^16-1
num < n --> flip bit

--------------------------

get 16  (2 bytes) --> and(16 (2 bytes)) 
do it n times --> or --> bit is 1 with probability n/(2^16)

*/
/*
void insert_noise(int *data, double probability, int *flipped)
{
    int mask = 0x1, i;
    float rand_num = 0;
    
    for (i = 0; i < BYTE_SIZE; i++)
    {
        rand_num = (float)rand()/RAND_MAX;
        if (rand_num < probability)
        {
            (*data) ^= mask;
            (*flipped)++;
        }
        mask <<= 1;
    }
}
*/







