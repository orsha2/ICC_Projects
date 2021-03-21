// include headers ------------------------------------------------------------

#include <stdlib.h>
#include <stdbool.h>


#include "bitfield.h"
#include "error_mgr.h"

// enum -----------------------------------------------------------------------




// constants ------------------------------------------------------------------

#define BITS_IN_BYTE 8

// global variables -----------------------------------------------------------


// function declarations ------------------------------------------------------


// function implementations ---------------------------------------------------

error_code_t create_bitfield(bitfield* p_bitfield, unsigned int size)
{
    error_code_t status = SUCCESS_CODE;

    int* p_bf_array = (int*)calloc(size, sizeof(int));

    status = check_mem_alloc(p_bf_array, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    p_bitfield->p_bitfield_array = p_bf_array;

    return status;
}


void set_bit(bitfield* p_bitfield, unsigned int bit_index, int new_value)
{
    unsigned int cell_to_modify = bit_index / (BITS_IN_BYTE * sizeof(*(p_bitfield->p_bitfield_array)));
    unsigned int offset_in_cell = bit_index % (BITS_IN_BYTE * sizeof(*(p_bitfield->p_bitfield_array)));

    if (new_value == 0)
        p_bitfield->p_bitfield_array[cell_to_modify] &= ~(1 << offset_in_cell); 
    else
        p_bitfield->p_bitfield_array[cell_to_modify] |= 1 << offset_in_cell;
}



int get_bit(bitfield* p_bitfield, unsigned int bit_index)
{
    unsigned int cell_index     = bit_index / (BITS_IN_BYTE * sizeof(*(p_bitfield->p_bitfield_array)));
    unsigned int offset_in_cell = bit_index % (BITS_IN_BYTE * sizeof(*(p_bitfield->p_bitfield_array)));

    unsigned int bit_value = 1 & (p_bitfield->p_bitfield_array[cell_index] >> offset_in_cell);
    
    return bit_value;
}


