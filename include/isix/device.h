#ifndef __ISIX_DEVICE_H
#define __ISIX_DEVICE_H

#include <isix/types.h>
#include <isix/list.h>

//typeof devices
#define CHAR_DEVICE 'C'


//Lenght of device name
#define DEVICE_NAME_LENGTH 4

//Device data structure
typedef struct dev_data_struct
{
    struct device_struct *dev;
    char prv_data[];
}
file_t;


//Device operation structure
typedef struct device_struct
{
    //Device name
    char name[DEVICE_NAME_LENGTH];
    //Device type
    char devtype;
    //Initializing device
    int (*init)(void);
    //Open device
    void* (*open)(const char *path,int flags);
    //Close device
    int (*close)(void *fd);
    //Read device
    int (*read)(void *fd,void *buf,size_t count);
    //Write device
    int (*write)(void *fd,void *buf,size_t count);
    //IOCTL specific Device fun
    int (*ioctl)(void *fd,int request,...);
    //Flush device
    int (*flush)(void *fd);
    //IOCTL device specific function
    int (*ioctl)(void *fd,int request,...);
    //Specific file operations
    void *fileop;
    //Used count
    int used;
    //Inode of device
    list_t inode;
}
device_t;


//Register device in system
int register_device(device_t *device);

//Unregister device
int unregister_device(device_t *device);

//Open device
file_t* open(const char *path,int flags);

//Close device
static inline int close(file_t *fd)
{
    return ((file_t*)fd)->dev->close(((file_t*)fd)->prv_data);
}

//Read device
static inline int read(file_t *fd,void *buf,size_t count)
{
   return ((file_t*)fd)->dev->read(((file_t*)fd)->prv_data,buf,count);
}

//Write device
static inline int write(file_t *fd,void *buf,size_t count)
{
   return ((file_t*)fd)->dev->write(((file_t*)fd)->prv_data,buf,count);
}


//IOCTL specific Device fun
#define ioctl(a,b,...)((file_t*)a)->dev->ioctl(((file_t*)a)->prv_data,,b,__VA_ARGS__)

//Flush device
static inline int flush(file_t *fd)
{
   return ((file_t*)fd)->dev->flush(((file_t*)fd)->prv_data);
}


#endif
