// include headers ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "hamming_code_handler.h"


// constants ------------------------------------------------------------------

#define BITS_IN_BYTE 8
#define CHECK_BITS_NUM 4 
#define DATA_BITS_NUM_PER_CHECK_BIT (HAMMING_ENCODED_BLOCK_SIZE - 1) / 2

// array containing all the data bits offsets in the encoded block, we substract 1 because the offsets should be relative to 0 
static const int HAMMING_BLOCK_DATA_OFFSETS[HAMMING_DATA_BLOCK_SIZE] = 
  { 3 - 1, 5 - 1, 6 - 1, 7 - 1, 9 - 1, 10 - 1, 11 - 1, 12 - 1, 13 - 1, 14 - 1, 15 - 1 };

// array containing all the check bits offsets in the encoded block, we substract 1 because the offsets should be relative to 0 
static const int HAMMING_BLOCK_CHECK_BITS_OFFSETS[CHECK_BITS_NUM] = 
  { 1 - 1, 2 - 1, 4 - 1, 8 - 1 };

// array containing all the relevent data bits for each check bit, we substract 1 because the offsets should be relative to 0 
static const int DATA_BITS_FOR_CHECK_BIT[CHECK_BITS_NUM][DATA_BITS_NUM_PER_CHECK_BIT] =
  { {3 - 1, 5  - 1, 7  - 1, 9  - 1, 11 - 1, 13 - 1, 15 - 1},
    {3 - 1, 6  - 1, 7  - 1, 10 - 1, 11 - 1, 14 - 1, 15 - 1},
    {5 - 1, 6  - 1, 7  - 1, 12 - 1, 13 - 1, 14 - 1, 15 - 1},
    {9 - 1, 10 - 1, 11 - 1, 12 - 1, 13 - 1, 14 - 1, 15 - 1} };


// function declarations ------------------------------------------------------

short encode_block(short data_block);
short decode_block(short encoded_data_block);

bool check_and_correct_errors(short* p_encoded_data_block);

short get_block(char* buffer, unsigned int buffer_size, unsigned int block_index, unsigned int block_size);
void set_block(char* buffer, unsigned int buffer_size, unsigned int block_index, unsigned int block_size, short new_block);

unsigned char get_bit(char* buffer, unsigned int buffer_size, unsigned int bit_index);
void set_bit(char* buffer, unsigned int buffer_size, unsigned int bit_index, unsigned char new_value);

bool is_square(unsigned int n); 

// function implementations --------------------------------------------------- 

void encode_data(char* data_buffer, unsigned int data_buffer_size, char* encoded_data_buffer, unsigned int encoded_data_buffer_size)
{
	unsigned int data_blocks_num = (int)(ceil((float)data_buffer_size * BITS_IN_BYTE / HAMMING_DATA_BLOCK_SIZE));

	short current_data_block = 0; 
	short current_encoded_block = 0;

	for (unsigned int block_index = 0; block_index < data_blocks_num; block_index++)
	{
		current_data_block = get_block(data_buffer, data_buffer_size, block_index, HAMMING_DATA_BLOCK_SIZE);

		current_encoded_block = encode_block(current_data_block);

		set_block(encoded_data_buffer, encoded_data_buffer_size, block_index, HAMMING_ENCODED_BLOCK_SIZE, current_encoded_block);
	}
}


unsigned int decode_data(char* encoded_data_buffer, unsigned int encoded_data_buffer_size, char* data_buffer, unsigned int data_buffer_size)
{
	unsigned int errors_num = 0;

	unsigned int encoded_blocks_num = (int)(ceil((float)encoded_data_buffer_size * BITS_IN_BYTE / HAMMING_ENCODED_BLOCK_SIZE));

	short current_encoded_block = 0;
	short current_data_block = 0;

	for (unsigned int block_index = 0; block_index < encoded_blocks_num; block_index++)
	{
		current_encoded_block = get_block(encoded_data_buffer, encoded_data_buffer_size, block_index, HAMMING_ENCODED_BLOCK_SIZE);

		errors_num += check_and_correct_errors(&current_encoded_block);
			
		current_data_block = decode_block(current_encoded_block);

		set_block(data_buffer, data_buffer_size, block_index, HAMMING_DATA_BLOCK_SIZE, current_data_block);
	}

	return errors_num;
}


short encode_block(short data_block)
{
	short encoded_data_block = 0; 
	unsigned char check_bit_value = 0;

	// place each data bit in the correct place in the block 
	for (int data_bit = 0; data_bit < HAMMING_DATA_BLOCK_SIZE; data_bit++)
	{
		encoded_data_block |= (1 & (data_block >> data_bit)) << HAMMING_BLOCK_DATA_OFFSETS[data_bit];
	}

	// calculate the check bits 
	for (int check_bit = 0; check_bit < CHECK_BITS_NUM; check_bit++)
	{
		check_bit_value = 0; 
		for (int data_bit = 0; data_bit < DATA_BITS_NUM_PER_CHECK_BIT; data_bit++)
		{
			check_bit_value ^= (1 & (encoded_data_block >> DATA_BITS_FOR_CHECK_BIT[check_bit][data_bit]));
		}

	   encoded_data_block |= check_bit_value << HAMMING_BLOCK_CHECK_BITS_OFFSETS[check_bit];
	}

	return encoded_data_block;
}

short decode_block(short encoded_data_block)
{
	short data_block = 0;

	// place each data bit in the correct place in the block 
	for (int data_bit = 0; data_bit < HAMMING_DATA_BLOCK_SIZE; data_bit++)
	{
		data_block |= (1 & (encoded_data_block >> HAMMING_BLOCK_DATA_OFFSETS[data_bit])) << data_bit;
	}

	return data_block;
}


bool check_and_correct_errors(short* p_encoded_data_block)
{
	bool is_error = false;

	unsigned char error_pos = 0;

	short encoded_data_block = *p_encoded_data_block;
	unsigned char check_bit_value = 0;

	// make sure the check bits are correct 
	for (int check_bit = 0; check_bit < CHECK_BITS_NUM; check_bit++)
	{
		check_bit_value = 1 & (encoded_data_block >> HAMMING_BLOCK_CHECK_BITS_OFFSETS[check_bit]);

		// XOR the check bit with the relevent data bits, if correct --> the XOR will be 0 
		for (int data_bit = 0; data_bit < DATA_BITS_NUM_PER_CHECK_BIT; data_bit++)
		{
			check_bit_value ^= (1 & (encoded_data_block >> DATA_BITS_FOR_CHECK_BIT[check_bit][data_bit]));
		}

		// check if error occured, if there is an error --> add to the error position calculation 
		if (check_bit_value != 0)
			error_pos |= 1 << check_bit;
		
	}

	is_error = (error_pos != 0); // && (is_square(error_pos) == false); 

	// flip bit that caused the error 
	if (error_pos != 0)
		encoded_data_block ^= (1 << (error_pos - 1));

	*p_encoded_data_block = encoded_data_block;

	return is_error;
}


short get_block(char* buffer, unsigned int buffer_size, unsigned int block_index, unsigned int block_size)
{
	short block_value = 0;

	for (unsigned int block_offset = 0; block_offset < block_size; block_offset++)
		block_value |= get_bit(buffer, buffer_size, block_index * block_size + block_offset) << block_offset;

	return block_value;
}

void set_block(char* buffer, unsigned int buffer_size, unsigned int block_index, unsigned int block_size, short new_block)
{
	for (unsigned int block_offset = 0; block_offset < block_size; block_offset++)
		set_bit(buffer, buffer_size, block_index * block_size + block_offset, 1 & (new_block >> block_offset));
}


unsigned char get_bit(char* buffer, unsigned int buffer_size, unsigned int bit_index)
{
	unsigned int cell_index = bit_index / (BITS_IN_BYTE * sizeof(char));
	unsigned int offset_in_cell = bit_index % (BITS_IN_BYTE * sizeof(char));

	if (cell_index >= buffer_size)
		return 0; 

	unsigned char bit_value = 1 & (buffer[cell_index] >> offset_in_cell);

	return bit_value;
}

void set_bit(char* buffer, unsigned int buffer_size, unsigned int bit_index, unsigned char new_value)
{
	unsigned int cell_to_modify = bit_index / (BITS_IN_BYTE * sizeof(char));
	unsigned int offset_in_cell = bit_index % (BITS_IN_BYTE * sizeof(char));

	if (cell_to_modify >= buffer_size)
		return; 

	if (new_value == 0)
		buffer[cell_to_modify] &= ~(1 << offset_in_cell);
	else
		buffer[cell_to_modify] |= 1 << offset_in_cell;
}

bool is_square(unsigned int n)
{
	return (n != 0) && ((n & (n - 1)) == 0); 
}


