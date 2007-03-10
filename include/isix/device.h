#ifndef __ISIX_DEVICE_H
#define __ISIX_DEVICE_H

#include <isix/types.h>
#include <isix/list.h>

//typeof devices
#define CHAR_DEVICE 'C'


//Lenght of device name
#define DEVICE_NAME_LENGTH 6

/*--------------------------------------------------------------*/
//Device data structure
typedef struct dev_data_struct
{
    struct device_struct *dev;
    char prv_data[];
}
file_t;


/*--------------------------------------------------------------*/
//Device operation structure
typedef struct device_struct
{
    //Device name
    char name[DEVICE_NAME_LENGTH];
    //Device type
    char devtype;
    //Used count
    unsigned short used;
    //Open device
    file_t* (*open)(const char *path,int flags);
    //Close device
    int (*close)(file_t *fd);
    //Read device
    int (*read)(file_t *fd,void *buf,size_t count);
    //Write device
    int (*write)(file_t *fd,void *buf,size_t count);
    //IOCTL specific Device fun
    int (*ioctl)(file_t *fd,int request,...);
    //Flush device
    int (*flush)(file_t *fd);
    //Specific file operations
    void *fileop;
    //Private data
    void *devprv;
    //Used count
    list_t inode;
}
device_t;

/*--------------------------------------------------------------*/
//Create default structure
device_t* alloc_struct_device(void);

/*--------------------------------------------------------------*/
//Register device in system
int register_device(device_t *dev);

/*--------------------------------------------------------------*/
//Unregister device
int unregister_device(device_t *dev);

/*--------------------------------------------------------------*/
//Open device
void* open(const char *path,int flags);

/*--------------------------------------------------------------*/
//Close device
static inline int close(void *fd)
{
   ((file_t*)fd)->dev->used--;
    return ((file_t*)fd)->dev->close((file_t*)fd);
}

/*--------------------------------------------------------------*/
//Read device
static inline int read(void *fd,void *buf,size_t count)
{
   return ((file_t*)fd)->dev->read((file_t*)fd,buf,count);
}

/*--------------------------------------------------------------*/
//Write device
static inline int write(void *fd,void *buf,size_t count)
{
   return ((file_t*)fd)->dev->write((file_t*)fd,buf,count);
}

/*--------------------------------------------------------------*/
//IOCTL specific Device fun
#define ioctl(a,b,...)((file_t*)a)->dev->ioctl((file_t*)a, b, __VA_ARGS__)


/*--------------------------------------------------------------*/
//Flush device
static inline int flush(void *fd)
{
   return ((file_t*)fd)->dev->flush((file_t*)fd);
}

/*--------------------------------------------------------------*/
#endif

