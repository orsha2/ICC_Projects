
#include <stdio.h>
#include <string.h>
#include "hamming_code_handler.h"


typedef struct _bit
{
	// Define an integer that it's size is 1 bit
     unsigned char b:1;
}bit;


void encode_data(char* data_buffer, int data_buffer_size, char* encoded_data_buffer)
{
	int i;
	char* arr = "asdasdssss";
	char arr1[150] ={ 0 };

	bit bit_field[100] = { 0 };
	for (int i = 0; i < strlen(arr); i++)
	{
		for (int j = 0; j < 8; j++)
		{
			bit_field[i * 8 + j].b = (arr[i] >> j) & 1;
		}
	}


	for (int i = 0; i < 100/8; i++)
	{
		printf("%d ", bit_field[i]);

		for (int j = 0; j < 8; j++)
		{
			arr1[i] = (arr1[i] << j) | (bit_field[i * 8 + j].b);
			
			//bit_field[i * 8 + j].b = (arr[i] >> j) & 1;
		}
	}

	bit_field[0].b =4;
	printf("%d\n", sizeof(bit_field));

	printf("%s\n", arr);

	printf("%s", arr1);
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