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

unsigned int sculll_quantum=400;
unsigned int sculll_qset=100;

unsigned int sculll_major=0;
unsigned int sculll_minor=0;
dev_t dev=0;
//struct cdev cdev;
struct class *sculll_class;

struct sculll_qset{
    void **data;
    struct sculll_qset *next;
};

struct sculll_dev {
    struct sculll_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};
struct sculll_dev *lll_dev;

struct sculll_qset *sculll_follow(struct sculll_dev *dev,int n)
{
    struct sculll_qset *qs = dev->data;

    if(!qs)
    {
        qs = dev->data = kmalloc(sizeof(struct sculll_qset),GFP_KERNEL);
        if(qs == NULL)
            return NULL;
        memset(qs,0,sizeof(struct sculll_qset));
    }
    while(n--)
    {
        if(!qs->next)
        {
            qs->next = kmalloc(sizeof(struct sculll_qset),GFP_KERNEL);
            if(qs->next == NULL)
                return NULL;
            memset(qs->next,0,sizeof(struct sculll_qset));
        }
        qs = qs->next;
        continue;
    }
    return qs;
}

int sculll_trim(struct sculll_dev *dev)
{
    struct sculll_qset *next,*dptr;
    int qset = dev->qset;
    int i;
    for(dptr = dev->data;dptr;dptr = next)
    {
        if(dptr->data)
        {
            for(i=0;i<qset;i++)
            {
                kfree(dptr->data[i]);
            }
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    dev->size = 0;
    dev->quantum = sculll_quantum;
    dev->qset = sculll_qset;
    dev->data = NULL;
    return 0;
}

ssize_t sculll_read(struct file *filp,char __user *buf,size_t count,loff_t *f_ops)
{
    struct sculll_dev *dev = filp->private_data;
    struct sculll_qset *dptr;
    int quantum = dev->quantum,qset = dev->qset;
    int itemsize = quantum * qset;
    int item,s_pos,q_pos,rest;
    ssize_t retval = 0;
    if(down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if(*f_ops >= dev->size)
        goto out;
    if(*f_ops + count > dev->size)
        count = dev->size - *f_ops;

    item = (long)*f_ops / itemsize;
    rest = (long)*f_ops % itemsize;

    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = sculll_follow(dev,item);
    if(dptr == NULL || !dptr->data || ! dptr->data[s_pos])
        goto out;

    if(count > quantum - q_pos)
        count = quantum - q_pos;

    if(copy_to_user(buf,dptr->data[s_pos] + q_pos,count))
    {
        retval = -EFAULT;
        goto out;
    }
    *f_ops += count;
    retval = count;
out:
    up(&dev->sem);
    return retval;
}


ssize_t scull_write(struct file * filp , const char __user *buf,size_t count,loff_t *f_ops)
{
    struct sculll_dev *dev = filp->private_data;
    struct sculll_qset *dptr;
    int quantum = dev->quantum,qset = dev->qset;
    int itemsize = quantum*qset;
    int item,s_pos,q_pos,rest;
    ssize_t retval = -ENOMEM;
    if(down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    item = (long)*f_ops / itemsize;
    rest = (long)*f_ops % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;
    dptr = sculll_follow(dev,item);
    if(dptr == NULL)
        goto out;
    if(!dptr->data)
    {
        dptr->data = kmalloc(qset * sizeof(char *),GFP_KERNEL);
        if(!dptr->data)
            goto out;
        memset(dptr->data ,0,qset * sizeof(char *));

        if(!dptr->data[s_pos])
        {
            dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
            if(!dptr->data[s_pos])
                goto out;
        }
        if(count > quantum - q_pos)
            count = quantum - q_pos;
        if(copy_from_user(dptr->data[s_pos]+q_pos,buf,count))
        {
            retval = -EFAULT;
            goto out;
        }
        *f_ops += count;
        retval = count;

        if(dev->size < *f_ops)
            dev->size = *f_ops;
out:
        up(&dev->sem);
        return retval;
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
    .open = sculll_open,
    .release = sculll_release,

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

    sculll_class = class_create(THIS_MODULE,"sys-lll_class");
    if(IS_ERR(sculll_class))
    {
        printk(KERN_ALERT"err: fial in create class");
        unregister_chrdev_region(dn,1);
        return -1;
    }
    else
    {
        printk(KERN_ALERT"success create class");
    }
    if(NULL == device_create(sculll_class,NULL,dn,NULL,"dev-sculll0"))
    {
        printk(KERN_ALERT"erro create device");
        class_destroy(sculll_class);
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
        device_destroy(sculll_class,dn);
        class_destroy(sculll_class);
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
    device_destroy(sculll_class,MKDEV(sculll_major,sculll_minor));
    class_destroy(sculll_class);
    unregister_chrdev_region(dn,1);
    printk(KERN_ERR"exit");
}


module_init(scull_init);
module_exit(scull_exit);
