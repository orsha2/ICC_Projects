#ifndef HAMMING_H
#define HAMMING_H

#include "Defines.h"

/***********************************************************************************/
/*
	Function: Function to encode a 26 bit data block using Hamming code.
	Input:	unsigned int data_block - An unsinged 26 bit integer.
	Output: An unsigned 31 bit integer encoded using Hamming.
*/
/***********************************************************************************/
uint32 Encode_data(uint32 data_block);

/***********************************************************************************/
/*
	Function: Function to decode a 31 bit block encoded using Hamming and
			  retrieve it's data. If a fix is made function increases counter by 1.
	Input:	unsigned int coded_data_block - An unsinged 31 bit integer.
			int *corrections - Pointer to corrections counter.
	Output: An unsigned 26 bit integer decoded using Hamming.
*/
/***********************************************************************************/
uint32 Decode_data(uint32 coded_data_block, int *corrections);

#endif