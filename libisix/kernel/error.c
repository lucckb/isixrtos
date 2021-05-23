#include <isix/config.h>
#include <isix/error.h>


static const char * error_string_table[] = 
{
    "No error", //0
    "Unknown error code", //1
    "Cannot allocate memory", //2
    "Syscall timeout", //3
    "Invalid argument",  //4
    "Fifo is full",  //5
    "Resource already used", //6
    "Invalid system priority", //7
    "Operation not supported", //8
    "IPC object has been reset", //9
    "IPC object has been destroyed", //10
    "Invalid state", //11
    "Permission denied", //12
    "Unknown error code", //13
    "Object not locked", //14
    "Task is not referenced", //15
    "Fifo queue is empty",    //16
    "Invalid address",  //17
    "Invalid data"     //18
};

const char* isix_strerror(int error)
{
    if( error>0 ) {
        error = 1;
    } else {
        error = -error;
    }
    if( error >= -_ISIX_ERROR_EOF ) {
        error = 1;
    }
    return error_string_table[error];
}