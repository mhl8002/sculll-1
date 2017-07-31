#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
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

static struct file_operations sculll_fops={
    .owner = THIS_MODULE,

};

static int scull_init(void)
{
    dev_t dn;
    if(alloc_chrdev_region(&dev,sculll_minor,4,"scullhm"))
    {
        printk(KERN_ALERT "register chrdev region is fail");
    }
    else
    {
        sculll_major = MAJOR(dev);
        printk(KERN_ALERT "register chrdev region success,sculll-major= %d",sculll_major);
    }

    dn = MKDEV(sculll_major,sculll_minor);
    cdev_init(&cdev,&sculll_fops);
    cdev.owner = THIS_MODULE;
    cdev.ops = &sculll_fops;
    if(cdev_add(&cdev,dn,4))
    {
        printk(KERN_ALERT "cdev add error");
    }
    else
    {
        printk(KERN_ALERT "cdev add success");
    }

    printk(KERN_ALERT "run in cpu %d\n", get_cpu());

    printk(KERN_ALERT "PAGE_OFFSET : 0x%lx, TASK_SIZE : 0x%lx", PAGE_OFFSET, TASK_SIZE);

    return 0;
}



static void scull_exit(void)
{
    unregister_chrdev_region(dev,4);
    printk(KERN_ERR"exit");
}


module_init(scull_init);
module_exit(scull_exit);
