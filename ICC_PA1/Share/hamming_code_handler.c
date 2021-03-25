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


// ----------------------- REMOVE THIS ---------------------------

void print_binary(unsigned int n, unsigned int bits_num); 

void RUN_HAMMING_TEST() 
{

	//					--- TESTS ---

	short b1, b2, b3;
	bool is_error; 
	b1 = 0b01001000001;
	printf("\nb1 ");
	print_binary(b1, 15); 

	b2 = encode_block(b1);  
	printf("\nb2 ");
	print_binary(b2, 15); 

	b2 ^= 1 << 7;
	printf("\nb2 ");
	print_binary(b2, 15);

	b3 = decode_block(b2);
	printf("\nb3 ");
	print_binary(b3, 15); printf("\n");

	printf("\nerrors num: %d", check_and_correct_errors(&b2)); 
	printf("\nb2 ");
	print_binary(b2, 15); 

	b3 = decode_block(b2);
	printf("\nb3 ");
	print_binary(b3, 15); printf("\n");

	printf("---\n");


	char* str1 = "Abc";
	char str2[5] = { 0 };
	char str3[100] = { 0 }; 

	b1 = get_block(str1, 3, 0, 9);
	/*
	printf("%06x\n", get_block(str1, 0, 8));
	printf("%06x\n", get_block(str1, 1, 8));
	printf("%06x\n", get_block(str1, 2, 8));

	printf("%06x\n", get_block(str1, 0, 11));
	*/
	for (size_t i = 0; i < strlen(str1); i++)
	{
		print_binary(get_block(str1, strlen(str1), i, 8), 8); printf(" ");
	}
	printf("\n---\n");

	/*
	print_binary(get_block(str1, 3, 0, 11)); printf("\n");

	print_binary(get_block(str1, 3, 1, 11)); printf("\n");

	print_binary(get_block(str1, 3, 2, 11)); printf("\n");

	print_binary(get_block(str1, 3, 0, 14)); printf("\n");

	print_binary(get_block(str1, 3, 1, 14)); printf("\n---\n");
	*/
	/*
	//set_block(str2, 3, 0, 8, get_block(str1, 3, 1, 8));
	set_block(str2, 3, 0, 12, get_block(str1, 3, 1, 8));
	set_block(str2, 3, 1, 12, get_block(str1, 3, 0, 8));

	print_binary(get_block(str2, 3, 0, 8)); printf("\n");

	print_binary(get_block(str2, 3, 1, 8)); printf("\n");

	print_binary(get_block(str2, 3, 2, 8)); printf("\n---\n");
	*/
	encode_data(str1, 3, str2, 5);	

	for (size_t i = 0; i < 5; i++)
	{
		print_binary(get_block(str2, 5, i, 8), 8); printf(" ");
	}
	printf("\n---\n");

	decode_data(str2, 5, str3, 100);

	for (size_t i = 0; i < 5; i++)
	{
		print_binary(get_block(str3, 5, i, 8), 8); printf(" ");
	} 

	printf("\n---\n");
	printf(str1);
	printf(" "); 
	printf(str2); 
	printf(" -- -- ");
	printf(str3);


	
	char str5[100] = "abc 213 ewf wef r rehg th\n";
	char str6[100] = { 0 };
	char str7[100] = { 0 };

	encode_data(str5, 100, str6, 100);	
	decode_data(str6, 100, str7, 100);
	printf("\n---\n");

	printf(str5);
	printf(str6);
	printf("\n");

	printf(str7);



	//					--- END TESTS ---
}

void print_binary(unsigned int n, unsigned int bits_num)
{
	int l = bits_num;
	for (int i = l - 1; i >= 0; i--) {
		printf("%x", (n & (1 << i)) >> i);
		if (i % 8 == 0)
			printf(" ");
	}
}
// ----------------------- REMOVE THIS ---------------------------


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

























	//for (int i = 0; i < 80; i++)
	//{

	//}

	/*
	i = 0 -- > 1100
	{
		j = 0 -- > 8{
		bitfield arr[i * 8 + j] = data_buffer[i] & 1;
		}
	}
	
	//--------------------------
	for (i = 0; i < data_buffer_size; i += 2)
	{
		short data_block_buffer;

		bitfield parity = 0;
		for i ->16
			i & (i-1) ==1 
				data_block_buffer (bit) = 0
			else
				data_block_buffer (bit) = arr[i].b 

		for i ->16
			i & (i-1) ==1 
				check_bits(data_block_buffer)

		c c   c       c           
		1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16

	

		arr = arr + 11;

	}

	bitfield *		: 1 0 1 1 0 1 0 1 0
	(char*)bitfield


	11111101 010         10101 011100      10 1101111
	int i, j = 0, k;
	uint32 code_word = 0, temp;
	Checkbit check_bits_array[PARITY_SIZE];

	for(i = 0; i < PARITY_SIZE; i++)
    {
        check_bits_array[i].b = 0;
    }
	//STEP-1: Fill all data bits in codeword
	for(i = 3; i <= DATA_SIZE_DECODED; i++)
    {
		//Condition of Power of 2
        if(i & (i-1))
		{ 
			k = Power(2,j++);
			temp = data_block & k;
            code_word |= (temp<<(i-j));
        }
    }
	//STEP-2: calculat all checkbits (parity bits)
	for(i = 3; i <= DATA_SIZE_DECODED; i++)
    {
		//condition for Power of 2
        if(i & (i-1))
		{ 
			for(j = 0; j < PARITY_SIZE; j++)
			{
                if(i & Power(2,j))
				{
					check_bits_array[j].b ^= Bit_pos(code_word, i);
                }
            }
        }
    }
	//STEP-3: Fill check bits in code word and return it
	for(i = 1, j = 0; i <= DATA_SIZE_DECODED; i++)
    {
		//Condition for only power of 2
        if(!(i & (i-1)))
        {
            code_word |= (check_bits_array[j++].b << (Power(2, j) - 1));
        }
    }
	return code_word;
}*/

//

//// Function to calculte the a number with a given power
//int Power(int base, int p)
//{
//    int i = 0, result = 1;
//
//    for(i = 1; i <= p; i++)
//        result *= base;
//    return result;
//}
//
//// Function to get the value of a bit in a certain index
//unsigned char Bit_pos(int num,int pos)
//{
//    return (num & Power(2,pos-1))>>(pos-1);
//}
//
//uint32 Decode_data(uint32 coded_data_block, int *corrections)
//{
//	Checkbit check_bits_array[PARITY_SIZE];
//	int i, j;
//	uint32 temp = 0, ciphered_data = 0;
//
//	// Initialize the parity bits array
//	for(i = 0; i < PARITY_SIZE; i++)
//    {
//        check_bits_array[i].b = 0;
//    }
//
//	// Get all parity bits according ti given coded word
//    for(i = 1; i <= DATA_SIZE_DECODED; i++)
//    {
//        for(j = 0; j < PARITY_SIZE; j++)
//		{
//            if(i & Power(2,j))
//			{
//                check_bits_array[j].b ^= Bit_pos(coded_data_block,i);
//            }
//        }
//    }
//
//	// Check if data is correct, if not get the index of wrong bit
//	for(i = 0; i < PARITY_SIZE; i++)
//    {
//        temp |= (check_bits_array[i].b << i);
//    }
//
//	// If coded data is not correct fix the wrong bit and increase the corrections counter
//	if(temp)
//	{
//        if(Bit_pos(coded_data_block, temp))
//			// Fix 1 to 0
//            coded_data_block -= Power(2, temp-1);
//        else
//			// Fix 0 to 1
//            coded_data_block += Power(2, temp-1);
//		// increase correction counter
//		(*corrections)++;
//     }
//	// Create the ciphered 26 bit data block and return it
//    for(i = 1, j = 0; i <= DATA_SIZE_DECODED; i++)
//	{
//        if(i & (i-1))
//		{
//            ciphered_data |= (Bit_pos(coded_data_block, i) << j++);
//        }
//    }
//
//	return ciphered_data;
//}