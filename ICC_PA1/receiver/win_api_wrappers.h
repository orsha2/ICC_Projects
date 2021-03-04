
#ifndef WIN_API_WRAPPERS
#define WIN_API_WRAPPERS

// include headers ------------------------------------------------------------

#include <stdbool.h>
#include <windows.h>
#include "error_mgr.h"

// functions declarations -----------------------------------------------------

// file api 

error_code_t open_file(HANDLE* p_file_handle, const char* file_name, DWORD desired_access, DWORD share_mode, DWORD creation_disposition, const char* source_file, int source_line, const char* source_func_name);

error_code_t delete_file(const char* file_name, const char* source_file, int source_line, const char* source_func_name); 

error_code_t set_file_pointer(HANDLE file_handle, int offset, int starting_position, const char* source_file, int source_line, const char* source_func_name);

error_code_t read_file(HANDLE file_handle, char* line, int bytes_to_read, DWORD* p_bytes_read, const char* source_file, int source_line, const char* source_func_name);

error_code_t read_line(HANDLE file_handle, char** p_line_buffer, int* p_line_length);

error_code_t write_file(HANDLE file_handle, char* line, int bytes_to_write, DWORD* p_bytes_written, const char* source_file, int source_line, const char* source_func_name);

error_code_t write_line_to_file(HANDLE file_handle, char* line, int starting_point); 

// threads api 

error_code_t create_thread(LPTHREAD_START_ROUTINE StartAddress, LPVOID ParameterPtr, LPDWORD ThreadIdPtr, HANDLE* p_thread_handle);

error_code_t get_exit_code_thread(HANDLE thread_handle, DWORD* p_thread_exit_code, const char* source_file, int source_line, const char* source_func_name);

error_code_t terminate_threads(HANDLE* thread_handles, int threads_num, DWORD brutal_termination_code); 

error_code_t terminate_thread(HANDLE thread_handle, DWORD brutal_termination_code, const char* source_file, int source_line, const char* source_func_name); 

error_code_t check_threads_exit_code(HANDLE* thread_handles, int threads_num, error_code_t current_status); 

// threads synchronization  

error_code_t create_semaphore(HANDLE* p_semaphore_handle, int initial_value, int max_value, const char* source_file, int source_line, const char* source_func_name);

error_code_t release_semaphore(HANDLE semaphore, int up_amount, const char* source_file, int source_line, const char* source_func_name);

// general functions 

error_code_t wait_for_multiple_objects(const HANDLE* object_handles, const int objects_num, bool wait_for_all, DWORD wait_time, const char* source_file, int source_line, const char* source_func_name); 

error_code_t wait_for_single_object(HANDLE object_handle, DWORD wait_time, bool to_print_error, const char* source_file, int source_line, const char* source_func_name);

error_code_t close_handle(HANDLE object_handle, int invalid_handle_value, const char* source_file, int source_line, const char* source_func_name);

#endif // WIN_API_WRAPPERS 