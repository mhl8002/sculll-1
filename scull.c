#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
//#include <linux/list.h>
//#include <linux/mm.h>
//#include <linux/mm_types.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("mhl ");
MODULE_DESCRIPTION("test ");

unsigned int sculll_major=0;
unsigned int sculll_minor=0;
dev_t dev=0;
//struct cdev cdev;
struct class *my_class;

struct sculll_dev {
    struct scull_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};
struct sculll_dev *lll_dev;

int sculll_trim(struct sculll_dev *dev)
{
    if(dev)
    {
        if(dev->data)
        {
            kfree(dev->data);
        }
        dev->data = NULL;
        dev->size = 0;
    }
}

int sculll_open(struct inode *inode,struct file *filp)
{
    struct sculll_dev *dev;
    dev = container_of(inode->i_cdev,struct sculll_dev,cdev);
    filp->private_data = dev;
    if((filp->f_flags & O_ACCMODE)==O_WRONLY)
    {
        sculll_trim(dev);
    }
    return 0;
}

int sculll_release(void)
{
    return 0;
}

static struct file_operations sculll_fops={
    .owner = THIS_MODULE,

};

static int scull_init(void)
{
    dev_t dn;
    if(alloc_chrdev_region(&dev,sculll_minor,1,"proc-scull"))
    {
        printk(KERN_ALERT "register chrdev region is fail");
    }
    else
    {
        sculll_major = MAJOR(dev);
        printk(KERN_ALERT "register chrdev region success,sculll-major= %d",sculll_major);
    }

    dn = MKDEV(sculll_major,sculll_minor);

    my_class = class_create(THIS_MODULE,"sys-my_class");
    if(IS_ERR(my_class))
    {
        printk(KERN_ALERT"err: fial in create class");
        unregister_chrdev_region(dn,1);
        return -1;
    }
    else
    {
        printk(KERN_ALERT"success create class");
    }
    if(NULL == device_create(my_class,NULL,dn,NULL,"dev-sculll0"))
    {
        printk(KERN_ALERT"erro create device");
        class_destroy(my_class);
        unregister_chrdev_region(dn,1);
        return -1;

    }
    else
    {
        printk(KERN_ALERT"success create device");
    }

    cdev_init(&lll_dev->cdev,&sculll_fops);
    lll_dev->cdev.owner = THIS_MODULE;
    lll_dev->cdev.ops = &sculll_fops;
    if(cdev_add(&lll_dev->cdev,dn,1))
    {
        device_destroy(my_class,dn);
        class_destroy(my_class);
        unregister_chrdev_region(dn,1);
        printk(KERN_ALERT "cdev add error");
    }
    else
    {
        printk(KERN_ALERT "cdev add success");
    }

    printk(KERN_ALERT "register char dev ");

    return 0;
}

static void scull_exit(void)
{
    dev_t dn = MKDEV(sculll_major,sculll_minor);
    cdev_del(&lll_dev->cdev);
    device_destroy(my_class,MKDEV(sculll_major,sculll_minor));
    class_destroy(my_class);
    unregister_chrdev_region(dn,1);
    printk(KERN_ERR"exit");
}


module_init(scull_init);
module_exit(scull_exit);
