#ifndef HAMMING_CODE_HANDLER_H
#define HAMMING_CODE_HANDLER_H

#define HAMMING_ENCODED_BLOCK 15
#define HAMMING_DATA_BLOCK 11
#define DATA_MASK 0x07FF // 11 bit mask 

#define ENCODED_DATA_BUFFER_SIZE HAMMING_ENCODED_BLOCK * 100
#define DATA_BUFFER_SIZE		 HAMMING_DATA_BLOCK * 100

///***********************************************************************************/
///*
//	Function: Function to encode a 26 bit data block using Hamming code.
//	Input:	unsigned int data_block - An unsinged 26 bit integer.
//	Output: An unsigned 31 bit integer encoded using Hamming.
//*/
///***********************************************************************************/


void encode_data(char* data_buffer, int data_buffer_size, char* encoded_data_buffer); 



///***********************************************************************************/
///*
//	Function: Function to decode a 31 bit block encoded using Hamming and
//			  retrieve it's data. If a fix is made function increases counter by 1.
//	Input:	unsigned int coded_data_block - An unsinged 31 bit integer.
//			int *corrections - Pointer to corrections counter.
//	Output: An unsigned 26 bit integer decoded using Hamming.
//*/
///***********************************************************************************/

int decode_data(int coded_data_block, int *corrections);


#endif // HAMMING_CODE_HANDLER_H