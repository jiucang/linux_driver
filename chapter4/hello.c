#include <linux/init.h>
#include <linux/module.h>

static char *book_name = " dissecting Linux Device Driver ";
static int num = 4000;

/**
 *  实际上__init & __exit都是宏定义：
 *  #define __init __attribute__ ((__setction__(".init.text"))
 *  
 *  #ifdef MODULE
 *  #define __exit  __attribute__ ((__setction__(".exit.text"))
 *  #else
 *  #define __exit __attribute_used_attribute__ ((__section__(".exit.text")))
 *  #endif
 *
 *  数据也可以被定义为__initdata & __exitdata
 *  #define __initdata __attribute__((__section__(".init.data")))
 *  #define __exitdata __attribute__((__section__(".exit.data")))
 * /

/**
 *1) __init: __init 函数在链接的是时候会被放置到.init.text这个区段内，此外，所有的__init函数在区段.initcall.init中还保留一份函数指针，在初始化的时候内核通过这些函数指针调用这些__init函数，并在初始化完成之后，释放init区段(包括.init.text/.initcall.init等)
 *2) return value:
 *模块的init函数，成功则返回0；如果失败则返回一个负值，一般是定义在<linux/errno.h>中的一个错误号
 *如：-ENODEV，-ENOMEM等，这样用户可以使用perror来输出对应的有意义的错误信息
 */
static int __init hello_init(void) {
    printk(KERN_INFO "Hello world enter\n");
    printk(KERN_INFO "Book name:%s\n", book_name);
    printk(KERN_INFO "Book num:%d\n", num);
    return 0;
}

/**
 * 1) 完成功能和加载函数相反，如果：
 * a/ 加载函数注册了xxx，卸载函数注销xxx
 * b/ 加载函数申请了内存，卸载函数释放内存
 * c/ 加载函数申请了硬件资源(interrupt/dma/io/io memory），卸载函数释放这些硬件资源
 * d/ 加载函数开启了硬件，卸载函数关闭硬件
 *
 * 2）无返回值
 */
static void __exit hello_exit(void) {
    printk(KERN_INFO " Hello World exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

/**
 * module_param(参数名，参数类型，参数读写权限)
 * 参数类型：byte, short, ushot, int, uint, long, ulong, charp(字符指针),bool,invbool(布尔的反)
 * 模块还可以有参数数组：
 * module_param_array(数组名，数组类型，数组长，参数读写权限)
 *
 *\sys\module 下有对应的sysfs文件节点可以查看，如果你使用的是0的权限，那么是没有对应的文件的
 */
module_param(num, int, S_IRUGO);
module_param(book_name, charp, S_IRUGO);

/**
 * Linux 2.6 "\proc\kallsysms" 文件对应着内核符号表，它记录着符号以及符号所在的内存地址
 * 模块可以使用如下宏调入到内核符号表中
 * EXPORT_SYMBOL(symbol)
 * EXPORT_SYMBOL_GPL(symbol)
 */
int add_integar(int a, int b) {
    return a+b;
}

int sub_integar(int a, int b) {
    return a-b;
}

EXPORT_SYMBOL(add_integar);
EXPORT_SYMBOL(sub_integar);

/**
 * 模块的引用计数：
 * 2.6之后内核函数：int try_module_get(struct module *module);
 *           void module_put(struct module *module);
 * 但是2.6内核之后一般为不同类型的设备定义了owner域；
 * struct module *owner；用来指向管理此设备的模块。
 * 开始使用某个设备时，内核自动调用：try_module_get(dev->owner)
 * 停止对该设备的调用时：module_put(dev->owner)
 *
 * 作用：设备在使用时，管理该设备的模块时不能被卸载的，设备驱动工程师一般也不需要亲自调用try_module_get/module_put函数。因为设备的owner模块的计数管理由更底层的代码，如总线驱动或此类设备共用的核心模块来实现，从而简化了设备驱动开发。
 */

MODULE_AUTHOR("Barry Song <21cnbao@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
