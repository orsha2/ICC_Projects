#ifndef HAMMING_CODE_HANDLER_H
#define HAMMING_CODE_HANDLER_H

// constants ------------------------------------------------------------------

#define HAMMING_ENCODED_BLOCK_SIZE 15
#define HAMMING_DATA_BLOCK_SIZE 11

#define ENCODED_DATA_BUFFER_SIZE HAMMING_ENCODED_BLOCK_SIZE * 100
#define DATA_BUFFER_SIZE		 HAMMING_DATA_BLOCK_SIZE * 100

// function declarations ------------------------------------------------------

void encode_data(char* data_buffer, unsigned int data_buffer_size, char* encoded_data_buffer, unsigned int encoded_data_buffer_size);

unsigned int decode_data(char* encoded_data_buffer, unsigned int encoded_data_buffer_size, char* data_buffer, unsigned int data_buffer_size);

#endif // HAMMING_CODE_HANDLER_H