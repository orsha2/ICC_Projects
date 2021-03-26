
// include headers -----------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "error_mgr.h"

// constants ------------------------------------------------------------------

// ERR_MSG - A data structure that contains all the error messages

const char* ERR_MSG[] = {
		"Success!",
		"Args - Wrong amount of arguments.",
		"Args - Program received invalid argument.",
		"Mem  - Memory allocation error.",

		"File - open failed.",
		"File - reading failed.",
		"File - writing failed.",

		"String - parsing failed. "

		"Handle - CloseHandle failed",

		"Winsock - initialization falied.", 
		"Winsock - deinitialization falied.",
		"Socket - creation failed",
		"Socket - bind failed",
		"Socket - listen failed",
		"Socket - connect failed",
		"Socket - send failed",
		"Socket - recv failed",
		"Socket - recv timeout",
		"Socket - setting blocking mode failed.",
		"Socket - setting operation timeout failed.",
		"Socket - connection closed.",
		"Socket - connection reset."
};

// function implementations ---------------------------------------------------

/// print_error
/// inputs:  error code, file, line and function name where the error happended
/// outputs: -
/// summary: prints the appropriate error message
/// 
void print_error(error_code_t error_code, const char* file, int line, const char* func_name)
{
	fprintf(stderr, "ERROR: %s\n", ERR_MSG[error_code]);
	fprintf(stderr, "File: %s\n", file);
	fprintf(stderr, "Line: %d, Function: %s\n", line, func_name);
}

/// check_args_num
/// inputs:  argc 
/// outputs: error_code 
/// summary: checks if argc is the desired amount
/// 
error_code_t check_args_num(int argc, int anticipated_args_num)
{
	if (argc != anticipated_args_num)
	{
		print_error(ARGS_NUM_ERROR, __FILE__, __LINE__, __func__);
		return ARGS_NUM_ERROR;
	}
	return SUCCESS_CODE;
}

/// check_mem_alloc
/// inputs:  ptr, file, line and function name where the error happened
/// outputs: error_code
/// summary: check ptr allocation & return to appropriate status 
/// 
error_code_t check_mem_alloc(void* ptr, const char* file, int line, const char* func_name)
{
	if (ptr == NULL)
	{
		print_error(MEM_ALLOC_ERROR, file, line, func_name);
		return MEM_ALLOC_ERROR;
	}
	return SUCCESS_CODE;
}

/// check_file_opening
/// inputs:  file_ptr , file, line and function name where the error happened
/// outputs: error_code
/// summary: check ptr allocation & return to appropriate status 
/// 
error_code_t check_file_opening(void* file_ptr, const char* file, int line, const char* func_name)
{
	if (file_ptr == NULL)
	{
		print_error(FILE_OPEN_FAILED, file, line, func_name);
		return FILE_OPEN_FAILED;
	}
	return SUCCESS_CODE;
}