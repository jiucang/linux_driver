/**
 * for learn driver
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/string.h>

extern struct bus_type scbus_type;

static char *Version = "revision 1.0, scdriver";

static int sc_probe(struct device *dev) {
    printk("driver found device\n");
    return 0;
}

static int sc_remove(struct device *dev) {
    printk("device remove\n");
    return 0;
}

struct device_driver scdriver = {
    .name = "scdevice0", //驱动名称，用来匹配支持的设备
    .bus = &scbus_type, //依附的总线类型
    .probe = sc_probe,  //如果这里总线没有对应的probe函数，driver的probe函数就会被调用
    .remove = sc_remove, //设备被移除的时候会被调用
};

ssize_t scdriver_version_show(struct device_driver *driver, char *buf) {
	return sprintf(buf, "%s\n", Version);
}
DRIVER_ATTR(version, S_IRUGO, scdriver_version_show, NULL);

static int __init scdriver_init(void) {
    int ret;

    ret = driver_register(&scdriver);
    if (ret)
        return ret;

    ret = driver_create_file(&scdriver, &driver_attr_version);
    if (ret)
        goto create_file_error;

    printk(KERN_INFO "scdriver init...");

    return ret;

create_file_error:
    driver_unregister(&scdriver);
    return ret;
}

static void __exit scdriver_exit(void) {
    driver_remove_file(&scdriver, &driver_attr_version);
    driver_unregister(&scdriver);
    printk(KERN_INFO "scdriver exit...");
}

module_init(scdriver_init);
module_exit(scdriver_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com>");
