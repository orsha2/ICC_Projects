#pragma warning( disable:6258 )
#pragma warning( disable:28182 )

// include headers -----------------------------------------------------------

#include <stdio.h>
#include <windows.h>

#include "win_api_wrappers.h"
#include "error_mgr.h"

// functions declarations -----------------------------------------------------

error_code_t get_line_length(HANDLE file_handle, int* p_line_length);

error_code_t check_handle_creation_result(HANDLE obj_handle, error_code_t error_code, const char* file, int line, const char* func_name);
error_code_t check_win_func_result(int win_func_result, error_code_t error_code, const char* file, int line, const char* func_name);

// functions implementations  -------------------------------------------------

// ----------------------------------------------------------------------------
//                              files api   
// ----------------------------------------------------------------------------

/// open_file
/// inputs:  p_file_handle, file_name, desired_access, share_mode, creation_disposition,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: opens files according to file_name, 
///          with desired_access, share_mode and creation_disposition parameters and sets the file handle.
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t open_file(HANDLE* p_file_handle, const char* file_name, DWORD desired_access, DWORD share_mode, DWORD creation_disposition,
                       const char* source_file, int source_line, const char* source_func_name)
{
	error_code_t status = SUCCESS_CODE;

    HANDLE file = CreateFileA(file_name, desired_access, share_mode, NULL,
                              creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        print_error(FILE_OPEN_FAILED, source_file, source_line, source_func_name);
        status = FILE_OPEN_FAILED;
    }

    *p_file_handle = file;

    return status;
}

/// delete_file
/// inputs:  file_name, source_file, source_line, source_func_name
/// outputs: error_code
/// summary: deletes file in path file_name, 
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t delete_file(const char* file_name, const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL del_result;

    del_result = DeleteFileA(file_name);

    if (del_result == 0)
    {
        print_error(FILE_DELETION_FAILED, source_file, source_line, source_func_name);
        status = FILE_DELETION_FAILED;
    }

    return status;
}

/// set_file_pointer
/// inputs:  file_handle, offset, starting_position, 
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: changes the (file_handle) to the desired location in file 
///          according to the (starting_position) and the (offset)
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t set_file_pointer(HANDLE file_handle, int offset, int starting_position,
                              const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    DWORD dwPtr;

    dwPtr = SetFilePointer(file_handle, offset, NULL, starting_position);

    if (dwPtr == INVALID_SET_FILE_POINTER)
    {
        print_error(SET_FILE_POINTER_FAILED, source_file, source_line, source_func_name);
        status = SET_FILE_POINTER_FAILED;
    }

    return status;
}

/// read_file
/// inputs:  file_handle, line, bytes_to_read, p_bytes_read,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: tries to read (bytes_to_read) bytes from the file into the (line) buffer, 
///          sets the actual amount of bytes read with (p_bytes_read)
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t read_file(HANDLE file_handle, char *line, int bytes_to_read, DWORD* p_bytes_read,
                       const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL return_code;

    return_code = ReadFile(file_handle, line, bytes_to_read, p_bytes_read, NULL);

    status = check_win_func_result(return_code, FILE_READING_FAILED, source_file, source_line, source_func_name);

    return status;
}

/// read_line
/// inputs:  file_handle, p_line_buffer, p_line_length
/// outputs: error_code
/// summary: reads one line from the file into the (*p_line_buffer) 
///          and sets the length of the line read (p_line_length)
///          sets the actual amount of byed read with (p_bytes_read)
///
error_code_t read_line(HANDLE file_handle, char** p_line_buffer, int* p_line_length)
{
    error_code_t status = SUCCESS_CODE;
    char* line_buffer = NULL;
    DWORD bytes_read = 0;
    int line_length = 0;

    status = get_line_length(file_handle, &line_length);

    if (status != SUCCESS_CODE)
        return status;

    line_buffer = (char*)realloc(*p_line_buffer, (line_length + 1) * sizeof(char));

    status = check_mem_alloc(line_buffer, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    status = read_file(file_handle, line_buffer, line_length, &bytes_read, __FILE__, __LINE__, __func__);

    line_buffer[line_length] = '\0';

    *p_line_buffer = line_buffer;
    *p_line_length = line_length;

    return status;
}

/// get_line_length
/// inputs:  file_handle, p_line_length
/// outputs: error_code
/// summary: counts the number of bytes in the current line in the file,
///          after counting, it retrieves the file_handle 
///          to the begginning of the line so we will be able to read it
///
error_code_t get_line_length(HANDLE file_handle, int* p_line_length)
{
    error_code_t status = SUCCESS_CODE;

    char char_buffer;
    int char_counter = 0;

    DWORD bytes_read;

    // read char until end of line 
    status = read_file(file_handle, &char_buffer, 1, &bytes_read, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    while (bytes_read != 0)
    {
        char_counter++;

        if (char_buffer == '\n' || char_buffer == '\0')
            break;

        status = read_file(file_handle, &char_buffer, 1, &bytes_read, __FILE__, __LINE__, __func__);

        if (status != SUCCESS_CODE)
            return status;
    }

    // go to the beginning of the line 
    status = set_file_pointer(file_handle, -(char_counter), FILE_CURRENT, __FILE__, __LINE__, __func__);

    *p_line_length = char_counter;

    return status;
}

/// write_file
/// inputs:  file_handle, line, bytes_to_write, p_bytes_written,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: tries to write (bytes_to_write) bytes into the file from the (line) buffer, 
///          sets the actual amount of bytes written with (p_bytes_written)
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t write_file(HANDLE file_handle, char* line, int bytes_to_write, DWORD* p_bytes_written,
                        const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL return_code;

    return_code = WriteFile(file_handle, line, bytes_to_write, p_bytes_written, NULL);

    status = check_win_func_result(return_code, FILE_WRITING_FAILED, source_file, source_line, source_func_name);

    return status;
}

/// write_line_to_file
/// inputs:  file_handle, line, starting_point
/// outputs: error_code
/// summary: writes the string in (line) to the appropriate location in the file (according to (starting_point)) 
/// 
error_code_t write_line_to_file(HANDLE file_handle, char* line, int starting_point)
{
    error_code_t status = SUCCESS_CODE;
    DWORD bytes_written;

    status = set_file_pointer(file_handle, 0, starting_point, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    status = write_file(file_handle, line, strlen(line) + 1, &bytes_written, __FILE__, __LINE__, __func__);

    return status;
}

// ----------------------------------------------------------------------------
//                                 threads api   
// ----------------------------------------------------------------------------

/// create_thread
/// inputs:  StartAddress , ParameterPtr , ThreadIdPtr , p_thread_handle
/// outputs: error_code  
/// summary: creates thread and updates handle pointer 
/// 
error_code_t create_thread(LPTHREAD_START_ROUTINE StartAddress, LPVOID ParameterPtr, LPDWORD ThreadIdPtr, HANDLE* p_thread_handle)
{
    error_code_t status = SUCCESS_CODE;

    HANDLE thread_handle = CreateThread(NULL, 0, StartAddress, ParameterPtr, 0, ThreadIdPtr);     

    status = check_handle_creation_result(thread_handle, THREAD_CREATION_FAILED, __FILE__, __LINE__, __func__);

    *p_thread_handle = thread_handle;

    return status;
}

/// get_exit_code_thread
/// inputs:  thread_handle, line, p_thread_exit_code, 
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: gets the thread's exit_code and sets the (p_thread_exit_code) 
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t get_exit_code_thread(HANDLE thread_handle, DWORD* p_thread_exit_code,
                                  const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL return_code;

    return_code = GetExitCodeThread(thread_handle, p_thread_exit_code);

    status = check_win_func_result(return_code, GET_THREAD_EXIT_CODE_FAILED, source_file, source_line, source_func_name);

    return status;
}

/// terminate_threads
/// inputs:  thread_handles, threads_num, brutal_termination_code 
/// outputs: error_code  
/// summary: for each thread in (thread_handles) that is still active - terminate it, 
///          if there are no active threads, return wait timeout error  
/// 
error_code_t terminate_threads(HANDLE* thread_handles, int threads_num, DWORD brutal_termination_code)          
{
    error_code_t status = SUCCESS_CODE;
   
    for (int thread_idx = 0; thread_idx < threads_num; thread_idx++)
    {
        status = terminate_thread(thread_handles[thread_idx], brutal_termination_code, __FILE__, __LINE__, __func__);
      
        if (status != SUCCESS_CODE)
            return status;
    }
    return THREADS_WAIT_TIMEOUT;
}

/// terminate_thread
/// inputs:  thread_handle, brutal_termination_code, 
///          source_file, source_line, source_func_name 
/// outputs: error_code  
/// summary: if thread is still active, tries to terminate the thread specified by (thread_handle).
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name) 
/// 
error_code_t terminate_thread(HANDLE thread_handle, DWORD brutal_termination_code,
                              const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    DWORD thread_exit_code;
    BOOL return_code;

    status = get_exit_code_thread(thread_handle, &thread_exit_code, __FILE__, __LINE__, __func__);

    if (status != SUCCESS_CODE)
        return status;

    if (thread_exit_code != STILL_ACTIVE)
        return status;

    return_code = TerminateThread(thread_handle, brutal_termination_code); 
    
    status = check_win_func_result(return_code, THREADS_TERMINATION_FAILED, source_file, source_line, source_func_name);

    return status;
}

/// check_threads_exit_code
/// inputs:  thread_handles,  threads_num
/// outputs: error_code  
/// summary: checks if threads exit codes were SUCCESS_CODE, if not --> return error 
/// 
error_code_t check_threads_exit_code(HANDLE* thread_handles, int threads_num, error_code_t current_status)
{
    error_code_t status = SUCCESS_CODE;

    DWORD thread_exit_code;
    int thread_index;

    for (thread_index = 0; thread_index < threads_num; thread_index++)
    {
        status = get_exit_code_thread(thread_handles[thread_index], &thread_exit_code, __FILE__, __LINE__, __func__);

        if (status != SUCCESS_CODE)
            return status;

        if (thread_exit_code != SUCCESS_CODE)
            return thread_exit_code;
    }
    return current_status;
}

// ----------------------------------------------------------------------------
//                           thread synchronization api   
// ----------------------------------------------------------------------------

/// create_semaphore
/// inputs:  p_semaphore_handle, initial_value, max_value, 
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: creates a semaphore with initial value and max 
///          specified by (initial_value) and (max_value). 
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t create_semaphore(HANDLE* p_semaphore_handle, int initial_value, int max_value, 
                              const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;

    *p_semaphore_handle = CreateSemaphore(NULL, initial_value, max_value, NULL);

    status = check_handle_creation_result(*p_semaphore_handle, SEMAPHORE_CREATION_FAILED, source_file, source_line, source_func_name);

    return status;
}

/// release_semaphore
/// inputs:  semaphore, up_amount
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: performs the "up" operation on the semaphore with (up_amount) value. 
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t release_semaphore(HANDLE semaphore, int up_amount,
                              const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL return_code;

    return_code = ReleaseSemaphore(semaphore, up_amount, NULL);

    status = check_win_func_result(return_code, SEMAPHORE_RELEASE_FAILED, source_file, source_line, source_func_name);

    return status; 
}

// ----------------------------------------------------------------------------
//                              general functions 
// ----------------------------------------------------------------------------

/// wait_for_multiple_object
/// inputs:  object_handles, objects_num, wait_for_all, wait_time
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: waits (wait_time) milliseconds for objects (only one or all of them according to (wait_for_all)) 
///          to be in signaled state,
///          in case of timeout --> returns the appropriate status. 
///                                 prints appropriate message specifying the function's caller
///                                 (according to source_file, source_line, source_func_name)
/// 
error_code_t wait_for_multiple_objects(const HANDLE* object_handles, const int objects_num, bool wait_for_all, DWORD wait_time,
                                      const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    DWORD wait_code;

    wait_code = WaitForMultipleObjects(objects_num, object_handles, wait_for_all, wait_time);

    if (wait_code != WAIT_OBJECT_0)
    {
        print_error(OBJECT_WAIT_TIMEOUT, source_file, source_line, source_func_name);
        status = OBJECT_WAIT_TIMEOUT;
    }

    return status;
}

/// wait_for_single_object
/// inputs:  object_handle, wait_time,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: waits (wait_time) milliseconds for object to be in signaled state,
///          in case of timeout --> returns the appropriate status. 
///                                 prints appropriate message specifying the function's caller
///                                 (according to source_file, source_line, source_func_name)
/// 
error_code_t wait_for_single_object(HANDLE object_handle, DWORD wait_time, bool to_print_error,
                                    const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    DWORD wait_code;

    wait_code = WaitForSingleObject(object_handle, wait_time);

    if (wait_code != WAIT_OBJECT_0)
    {
        if(to_print_error)
            print_error(OBJECT_WAIT_TIMEOUT, source_file, source_line, source_func_name);

        status = OBJECT_WAIT_TIMEOUT;
    }

    return status;
}

/// close_handle
/// inputs:  object_handle, invalid_handle_value,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: if (object_handle != invalid_handle_value) closes the handle to the object. 
///          in case of error --> prints appropriate message specifying the function's caller
///          (according to source_file, source_line, source_func_name)
/// 
error_code_t close_handle(HANDLE object_handle, int invalid_handle_value,
                          const char* source_file, int source_line, const char* source_func_name)
{
    error_code_t status = SUCCESS_CODE;
    BOOL return_code;

    if (object_handle != (HANDLE)invalid_handle_value)
    {
        return_code = CloseHandle(object_handle);

        status = check_win_func_result(return_code, CLOSE_HANDLE_FAILED, source_file, source_line, source_func_name);
    }
    return status;
}

/// check_handle_creation_result
/// inputs:  object_handle, error_code,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: if (object_handle != NULL) return SUCCESS_CODE. 
///          else print error and return error_code
/// 
error_code_t check_handle_creation_result(HANDLE obj_handle, error_code_t error_code, const char* file, int line, const char* func_name)
{
    if (obj_handle == NULL)
    {
        print_error(error_code, file, line, func_name);
        return error_code;
    }

    return SUCCESS_CODE;
}

/// check_win_func_result
/// inputs:  win_func_result, error_code,
///          source_file, source_line, source_func_name
/// outputs: error_code
/// summary: if (win_func_result == TRUE) return SUCCESS_CODE. 
///          else print error and return error_code
/// 
error_code_t check_win_func_result(int win_func_result, error_code_t error_code, const char* file, int line, const char* func_name)
{
    if (win_func_result == FALSE)
    {
        print_error(error_code, file, line, func_name);
        return error_code;
    }
    return SUCCESS_CODE;
}
