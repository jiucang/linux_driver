#include <linux/module.h>
//module_init, module_exit
#include <linux/types.h>
//u32, ssize_t and so on
#include <linux/fs.h>
//
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/slab.h>
//kmalloc
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x1
#define GLOBALMEM_MAJOR 250

static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;

int globalmem_open(struct inode *inode, struct file *filp) {
    filp->private_data = globalmem_devp;
    return 0;
}

int globalmem_release(struct inode *inode, struct file *filp) {
    return 0;
}

long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct globalmem_dev *dev = filp->private_data;

    switch(cmd) {
    case MEM_CLEAR:
        memset(dev->mem, 0, GLOBALMEM_SIZE);
        printk(KERN_INFO "globalmem is set to zero\n");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

loff_t globalmem_llseek(struct file *filp, loff_t offset, int method) {
    loff_t ret = 0;
    switch (method) {
    case 0: //SEEK_BEGIN
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        if ((unsigned int)offset > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset;
        ret = filp->f_pos;
    case 1: //SEEK_CURRENT
        if (offset + filp->f_pos < 0) {
            ret = -EINVAL;
            break;
        }
        if (offset + filp->f_pos > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset + filp->f_pos;
        ret = filp->f_pos;
    case 2: //SEEK_TAIL
        if (offset + filp->f_pos < 0) {
            ret = -EINVAL;
            break;
        }
        if (offset + filp->f_pos > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int)offset + filp->f_pos;
        ret = filp->f_pos;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

ssize_t globalmem_read(struct file *filp, char __user *user, size_t size, loff_t *offset) {
    int ret;
    struct globalmem_dev *devp;

    devp = filp->private_data;

    if (*offset >= GLOBALMEM_SIZE)
        return 0;
    if (size + *offset > GLOBALMEM_SIZE)
        size = GLOBALMEM_SIZE - *offset;

    if (copy_to_user(user, (void *)(devp->mem + *offset), size)) {
        return -EFAULT;
    } else {
        *offset = *offset + size;
        ret = size;
        printk(KERN_INFO "read %u bytes from %lu\n", (unsigned int)size, (long unsigned int)*offset);
    }
    return ret;
}

ssize_t globalmem_write(struct file *filp, const char __user *user, size_t size, loff_t *offset) {
    int ret;
    struct globalmem_dev *devp;

    devp = filp->private_data;

    if (*offset >= GLOBALMEM_SIZE)
        return 0;
    if (size + *offset > GLOBALMEM_SIZE) 
        size = GLOBALMEM_SIZE - *offset;

    if (copy_from_user((char *)(devp->mem + *offset), user, size)) {
        ret = -EFAULT;
    } else {
        *offset = *offset + size;
        ret = *offset;
        printk(KERN_INFO "written %u bytes from %lu\n", (unsigned int)size,  (long unsigned int)*offset);
    }
    return ret;
}

static const struct file_operations globalmem_fops = {
    .owner = THIS_MODULE,
    .open = globalmem_open,
    .read = globalmem_read,
    .write = globalmem_write,
    .llseek = globalmem_llseek,
    .unlocked_ioctl = globalmem_ioctl,
    .release = globalmem_release,
};

static void globalmem_setup_chrdev(struct globalmem_dev *devp, int minor) {
    int err, devno = MKDEV(globalmem_major, minor);

    cdev_init(&devp->cdev, &globalmem_fops);
    devp->cdev.owner = THIS_MODULE;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d add globalmem device, minor %d\n", err, minor);
}

int __init globalmem_init(void) {
    int ret;
    int devno = MKDEV(globalmem_major, 0);

    //Assigned the dev major youself
    if (globalmem_major)
        ret = register_chrdev_region(devno, 1, "globalmem");
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
        globalmem_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

    globalmem_setup_chrdev(globalmem_devp, 0);
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 0);
    return 0;
}

void __exit globalmem_exit(void) {
    cdev_del(&globalmem_devp->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 0);
}

module_param(globalmem_major, int, S_IRUGO);
module_init(globalmem_init);
module_exit(globalmem_exit)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Barry Song <21 cnbao@gmail.com>");
