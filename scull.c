#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
//#include <linux/list.h>
//#include <linux/mm.h>
//#include <linux/mm_types.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("mhl ");
MODULE_DESCRIPTION("test ");

unsigned int sculll_major=0;
unsigned int sculll_minor=0;
dev_t dev=0;
struct cdev cdev;
struct class *my_class;

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

    cdev_init(&cdev,&sculll_fops);
    cdev.owner = THIS_MODULE;
    cdev.ops = &sculll_fops;
    if(cdev_add(&cdev,dn,1))
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
    cdev_del(&cdev);
    device_destroy(my_class,MKDEV(sculll_major,sculll_minor));
    class_destroy(my_class);
    unregister_chrdev_region(dn,1);
    printk(KERN_ERR"exit");
}


module_init(scull_init);
module_exit(scull_exit);
