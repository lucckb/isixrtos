#include <isix/config.h>
#include <isix/types.h>
#include <isix/list.h>
#include <isix/device.h>
#include <isix/memory.h>
#include <isix/scheduler.h>

//Enable debuging
#define DEBUG

#ifdef DEBUG
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*------------------------------------------------------*/
//Device list in kernel
static list_entry_t devices;

/*------------------------------------------------------*/
//Dummy function
static int dummy_func(void)
{
    printk("DummyFunc: Warning !!! called\n");
    return -1;
}

/*------------------------------------------------------*/
//Create default structure
device_t* alloc_struct_device(void)
{
    device_t *dev = (device_t*)kmalloc(sizeof(device_t));
    if(!dev) return NULL;
    zero_memory(dev,sizeof(device_t));
    dev->init =    (void*)dummy_func;
    dev->destroy = (void*)dummy_func;
    dev->open =    (void*)dummy_func;
    dev->close =   (void*)dummy_func;
    dev->read =    (void*)dummy_func;
    dev->write =   (void*)dummy_func;
    dev->ioctl =   (void*)dummy_func;
    dev->flush =   (void*)dummy_func;
    return dev;
}
/*------------------------------------------------------*/
//Compare string
static int string_cmp(const char *s1,const char *s2)
{
    while(*s1 && *s2)
    {
        if(*s1++ != *s2++) return *s1;
    }
    return 0;
}

/*------------------------------------------------------*/
//Register device in system
int register_device(device_t *dev)
{
    sched_lock();
    device_t *d;
    if(devices.head.next == NULL) list_init(&devices);
    list_for_each_entry(&devices,d,inode)
    {
        if(string_cmp(dev->name,d->name))
        {
            printk("RegisterDevice: Device %s already registered\n",dev->name);
            sched_unlock();
            return -1;
        }
    }
    list_insert_end(&devices,&dev->inode);
    int res = dev->init(dev);
    sched_unlock();
    printk("RegisterDevice: Device %s initialized with result %d\n",dev->name,res);
    return res;
}

/*-------------------------------------------------------*/
//Unregister device
int unregister_device(device_t *dev)
{
    sched_lock();
    if(dev->used)
    {
        sched_unlock();
        printk("UnregisterDevice: Can`t unregister device dev %s is used\n",dev->name);
        return -1;
    }
    int res = dev->destroy(dev);
    list_delete(&dev->inode);
    printk("UnregisterDevice: Unregister dev %s with result %d\n",dev->name,res);
    sched_unlock();
    return res;
}

/*-------------------------------------------------------*/
//Open device
void* open(const char *path,int flags)
{
    if(path[0] != '/') return NULL;
    char devname[DEVICE_NAME_LENGTH];
    const char *dpath=NULL;
    file_t *ret;
    for(int i=1;i<DEVICE_NAME_LENGTH+1;i++)
    {
        if(path[i]==0) break;
        if(path[i]=='/') { dpath = &path[i]; break; }
        devname[i-1] = path[i];
    }
    devname[DEVICE_NAME_LENGTH-1] = 0;
    printk("open: Try open device %s\n",devname);
    //Try find specific device
    device_t *dev;
    sched_lock();
    list_for_each_entry(&devices,dev,inode)
    {
        if(!string_cmp(dev->name,devname))
        {
            ret = dev->open(dpath,flags);
            printk("open: Open device with result %d\n",ret);
            if(!ret) {sched_unlock(); return NULL;}
            ret->dev = dev;
            sched_unlock();
            return ret;
        }
    }
    sched_unlock();
    printk("open: Device not found\n");
    return NULL;
}

