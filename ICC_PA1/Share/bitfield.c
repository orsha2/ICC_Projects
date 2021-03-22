
#include <stdlib.h>
#include <stdbool.h>


#include "bitfield.h"
#include "error_mgr.h"


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



