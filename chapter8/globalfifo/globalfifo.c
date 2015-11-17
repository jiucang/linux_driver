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

#include <asm/wait.h>

#define GLOBALINFO_SIZE 0x1000
#define MEM_CLEAR 0x1
#define GLOBALINFO_MAJOR 250

static int globalinfo_major = GLOBALINFO_MAJOR;

struct globalfifo_dev {
    struct cdev cdev;
    unsigned int current_len;
    unsigned char mem[GLOBALINFO_SIZE];
	struct mutex mutex;	/*并发控制使用的信号量*/
	wait_queue_head_t r_wait; /*阻塞读用的等待队列头*/
	wait_queue_head_t w_wait; /*阻塞写用的等待队列头*/
};

struct globalfifo_dev *globalfifo_devp;

int globalinfo_open(struct inode *inode, struct file *filp) {
    filp->private_data = globalfifo_devp;
    return 0;
}

int globalinfo_release(struct inode *inode, struct file *filp) {
    return 0;
}

long globalinfo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct globalfifo_dev *dev = filp->private_data;

    switch(cmd) {
    case MEM_CLEAR:
        memset(dev->mem, 0, GLOBALINFO_SIZE);
        printk(KERN_INFO "globalinfo is set to zero\n");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

loff_t globalinfo_llseek(struct file *filp, loff_t offset, int method) {
    loff_t ret = 0;
    switch (method) {
    case 0: //SEEK_BEGIN
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        if ((unsigned int)offset > GLOBALINFO_SIZE) {
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
        if (offset + filp->f_pos > GLOBALINFO_SIZE) {
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
        if (offset + filp->f_pos > GLOBALINFO_SIZE) {
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

ssize_t globalinfo_read(struct file *filp, char __user *user, size_t size, loff_t *offset) {
    int ret;
    struct globalfifo_dev *devp;

	DECLARE_WAITQUEUE(rwait, current);		/*在当前进程创建一等待队列rwait*/

    devp = filp->private_data;

	mutex_lock_interruptible(&devp->mutex);	/*FIFO对象，如果有多个进程同时读，导致混乱*/
	add_wait_queue(&devp->r_wait, &rwait);

	while (devp->current_len <= 0) {
		if (filp->f_flags & O_NONBLOCK) {		/*检查是否是非阻塞*/
			ret = -EAGAIN;
			goto out;
		}
		__set_current_state(TASK_INTERRUPTIBLE);	/*改变进程状态为可中断的休眠态*/
		mutex_unlock(&devp->mutex);				/*进程休眠之前需要释放互斥锁*/

		schedule();
		if (signal_pending(current)) {			/*Wakeup cause of signal, but by condition*/
			ret = -ERESTARTSYS;
			goto out2;
		}
		/*Wakup cause condition scatified */
		mutex_lock_interruptible(&devp->mutex);
	}

    if (copy_to_user(user, (void *)(devp->mem), size)) {
        return -EFAULT;
    } else {
        devp->current_len = devp->current_len + size;
        ret = devp->current_len;
        printk(KERN_INFO "read %u byte(s), current_len=%lu\n", (unsigned int)size, (long unsigned int)*ret);
    }

	wake_up_interruptible(&devp->w_wait);

out:
	mutex_unlock(&devp->mutex);
out2:
	remove_wait_queue(&devp->r_wait, rwait);
	set_current_state(TASK_RUNNING);
    return ret;
}

ssize_t globalinfo_write(struct file *filp, const char __user *user, size_t size, loff_t *offset) {
    int ret;
    struct globalfifo_dev *devp;

	DECLARE_WAITQUEUE(&wwait, current);

    devp = filp->private_data;

	add_wait_queue(&devp->w_wait, &wwait);
	mutex_lock_interruptible(&devp->mutex);

	while (devp->current_len >= GLOBALINFO_SIZE) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}
		__set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&devp->mutex);

		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto out2;
		}
		mutex_lock_interruptible(&devp->mutex);
	}

    if (copy_from_user((char *)(devp->mem + devp->current_len), user, size)) {
        ret = -EFAULT;
    } else {
		devp->current_len = devp->current_len + size;
        printk(KERN_INFO "written %u byte(s),  current_len=%lu\n", (unsigned int)size,  (long unsigned int)devp->current_len);
    }
	wake_up_interruptible(&devp->r_wait);

out:
	mutex_unlock(&devp->mutex);
out2:
	remove_wait_queue(&devp->w_wait, &wwait);
    return ret;
}

static const struct file_operations globalinfo_fops = {
    .owner = THIS_MODULE,
    .open = globalinfo_open,
    .read = globalinfo_read,
    .write = globalinfo_write,
    .llseek = globalinfo_llseek,
    .unlocked_ioctl = globalinfo_ioctl,
    .release = globalinfo_release,
};

static void globalinfo_setup_chrdev(struct globalfifo_dev *devp, int minor) {
    int err, devno = MKDEV(globalinfo_major, minor);

    cdev_init(&devp->cdev, &globalinfo_fops);
    devp->cdev.owner = THIS_MODULE;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d add globalinfo device, minor %d\n", err, minor);
}

int __init globalinfo_init(void) {
    int ret;
    int devno = MKDEV(globalinfo_major, 0);

    //Assigned the dev major youself
    if (globalinfo_major)
        ret = register_chrdev_region(devno, 1, "globalinfo");
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, "globalinfo");
        globalinfo_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

    globalinfo_setup_chrdev(globalfifo_devp, 0);

	globalinfo_devp->current_len = 0;
	mutex_init(&globalinfo_devp->mutex);	/*初始化互斥体*/
	init_wait_queue_head(&globalinfo_devp->r_wait);
	init_wait_queue_head(&globalinfo_devp->w_wait);		/*初始化读写等待队列*/
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 0);
    return 0;
}

void __exit globalinfo_exit(void) {
    cdev_del(&globalfifo_devp->cdev);
    kfree(globalfifo_devp);
    unregister_chrdev_region(MKDEV(globalinfo_major, 0), 0);
}

module_param(globalinfo_major, int, S_IRUGO);
module_init(globalinfo_init);
module_exit(globalinfo_exit)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Barry Song <21 cnbao@gmail.com>");
