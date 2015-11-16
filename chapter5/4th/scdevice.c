/**
 * for learn device
 */
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>

extern struct bus_type scbus_type;
extern struct device scbus;
extern struct class *scclass;

static char *Device_version = "revision 1.0, device";

static void scdevice_release(struct device *dev) {
    printk("scbus device release\n");
}

struct device scdevice = {
    .parent = &scbus,
    .init_name = "scdevice0",
    .release = scdevice_release,
    .bus = &scbus_type,
};
EXPORT_SYMBOL_GPL(scdevice);

static ssize_t show_device_version(struct device *dev, 
	struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%s\n", Device_version);
}
DEVICE_ATTR(version, S_IRUGO, show_device_version, NULL);

static int __init scdevice_init(void) {
    int ret;

    ret = device_register(&scdevice);
    if (ret)
        return ret;

    ret = device_create_file(&scdevice, &dev_attr_version);
    if (ret)
	    goto create_file_err;

    /*Add a device to class */
    device_create(scclass, NULL, 0, 0, "scdevice0");

    printk("Create a scdevice device\n");
    return ret;

create_file_err:
    device_unregister(&scdevice);
    return ret;
}

static void __exit scdevice_exit(void) {
    /*Delete a device from class */
    device_destroy(scclass, 0);
    device_remove_file(&scdevice, &dev_attr_version);
    device_unregister(&scdevice);
    printk("Remove a device\n");
}

module_init(scdevice_init);
module_exit(scdevice_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com>");
