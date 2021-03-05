
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


// function implementations ---------------------------------------------------

int main(int argc, char* argv[])
{
    error_code_t status = SUCCESS_CODE;

    status = check_args_num(argc, CHANNEL_ARGS_NUM);

    if (status != SUCCESS_CODE)
        return status;

    int channel_port = atoi(argv[CHANNEL_PORT_INDEX]);
    char* reciver_ip = argv[RECEIVER_IP_INDEX];
    int reciver_port = atoi(argv[RECEIVER_PORT_INDEX]);

    int bit_flip_probability= atoi(argv[BIT_FLIP_PROBABILITY_INDEX]);
    int random_seed = atoi(argv[RANDOM_SEED_INDEX]);


    SOCKET channel_socket = INVALID_SOCKET;

    int transferred_bytes, flipped_bits;

    status = initialize_winsock();

    if (status != SUCCESS_CODE)
        return status;

    status = initialize_socket(&channel_socket);

    if (status != SUCCESS_CODE)
        goto channel_clean_up;

    //status = transfer_file(channel_socket, channel_ip, channel_port, file_name);

    //if (status != SUCCESS_CODE)
    //    goto sender_clean_up;

    //status = recv_feedback(channel_socket, &received_bytes, &written_bytes, &detected_errors_num, &corrected_errors_num);

    //if (status != SUCCESS_CODE)
    //    goto sender_clean_up;

    fprintf(stderr, FEEDBACK_MSG, received_bytes, written_bytes, detected_errors_num, corrected_errors_num);

channel_clean_up:

    if (channel_socket != INVALID_SOCKET)
        closesocket(channel_socket);

    deinitialize_winsock();
    return (int)status;
}
