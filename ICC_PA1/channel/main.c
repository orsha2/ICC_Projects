
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

#define BITS_IN_BYTE 8
static const char* SUMMARY_MSG = "sender: %s\nreceiver: %s\n%d bytes, flipped %d bits";

// function declarations ------------------------------------------------------

error_code_t transfer_messages(SOCKET channel_socket, char* receiver_ip, int receiver_port, char* sender_ip, int* p_sender_port, int* p_transferred_bytes, int bit_flip_probability, int* p_flipped_bits_num); 
int insert_noise(char* data, unsigned int data_length, unsigned int bit_flip_probability);
unsigned short get_random_short();


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

    srand(random_seed);

    SOCKET channel_socket = INVALID_SOCKET;

    int transferred_bytes = 0, flipped_bits_num = 0;

    char sender_ip[STR_IP_SIZE + 1] = "";
    int sender_port = 0;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_socket(&channel_socket);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    status = bind_to_port(channel_socket, channel_port);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    status = transfer_messages(channel_socket, receiver_ip, receiver_port, sender_ip, &sender_port, &transferred_bytes, bit_flip_probability, &flipped_bits_num);

    fprintf(stderr, SUMMARY_MSG, sender_ip, receiver_ip, transferred_bytes, flipped_bits_num);

channel_clean_up:

    if (channel_socket != INVALID_SOCKET)
        closesocket(channel_socket);

    deinitialize_winsock();
    return (int)status;
}

error_code_t transfer_messages(SOCKET channel_socket, char* receiver_ip, int receiver_port, char* sender_ip, int* p_sender_port, int* p_transferred_bytes, int bit_flip_probability, int* p_flipped_bits_num)
{

    error_code_t status = SUCCESS_CODE;
    char source_ip[STR_IP_SIZE] = "";
    int source_port = 0;

    char* received_msg_buffer = NULL;
    int msg_length = 0;

    while (true)
    {
        status = receive_message_from(channel_socket, &received_msg_buffer, &msg_length, source_ip, &source_port);

        if (status != SUCCESS_CODE)
            goto transfer_messages_exit;

        Sleep(UDP_SYNC_DELAY);

        // check if received message from receiver 
        if (strcmp(source_ip, receiver_ip) == 0 && source_port == receiver_port)
        {
            // transfer message to sender and return 
            status = send_message_to(channel_socket, received_msg_buffer, msg_length, sender_ip, *p_sender_port);
            goto transfer_messages_exit;
        }
        else
        {
            // message is from sender, store its address
            if (strlen(sender_ip) == 0 && (*p_sender_port) == 0)
            {
                strcpy_s(sender_ip, STR_IP_SIZE + 1, source_ip);
                *p_sender_port = source_port;
            }
        }

        (*p_flipped_bits_num) += insert_noise(received_msg_buffer, msg_length, bit_flip_probability);

        status = send_message_to(channel_socket, received_msg_buffer, msg_length, receiver_ip, receiver_port);

        (*p_transferred_bytes) += msg_length;

        if (status != SUCCESS_CODE)
            goto transfer_messages_exit;
    }

transfer_messages_exit:

    if (received_msg_buffer != NULL)
        free(received_msg_buffer);

    return status;
}

int insert_noise(char* data, unsigned int data_length,  unsigned int bit_flip_probability)
{
    unsigned int cell_index, bit;
    unsigned int random_short = 0;
    unsigned int flipped_bits_num = 0;

    for (cell_index = 0; cell_index < data_length; cell_index++)
    {
        for (bit = 0; bit < BITS_IN_BYTE; bit++)
        {
            random_short = get_random_short();

            if (random_short < bit_flip_probability)
            {
                data[cell_index] ^= 1 << bit;
                flipped_bits_num++;
            }
        }
    }

    return flipped_bits_num;
}

unsigned short get_random_short()
{
    unsigned short random_bit = rand() % 2;
    unsigned short random_15_bits = rand() % (1 << 15);

    return ((random_15_bits << 1) | random_bit); 
}





