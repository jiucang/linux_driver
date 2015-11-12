/**
 *  for learn bus
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/device.h>

static char *Version = "revision 1.0, scbus";
static char *Device_version = "revision 1.0, device";

static int scbus_match(struct device *dev, struct device_driver *driver) {
    printk("\n%s,%s\n", dev_name(dev), driver->name);
    return !strncmp(dev_name(dev), driver->name, strlen(driver->name));
}

static void scbus_release(struct device *dev) {
    printk("scbus release\n");
}

static void scbus_device_release(struct device *dev) {
    printk("scbus device release\n");
}

static struct bus_type scbus_type = {
    .name = "scbus",
    .match = scbus_match,
};
EXPORT_SYMBOL_GPL(scbus_type);

static struct device scbus = {
    .init_name = "scbus0",
    .release = scbus_release,
};
EXPORT_SYMBOL_GPL(scbus);

static struct device scbus_device = {
    .parent = &scbus,
    .init_name = "scdevice0",
    .release = scbus_device_release,
    .bus = &scbus_type,
};
EXPORT_SYMBOL_GPL(scbus_device);

static ssize_t show_bus_version(struct bus_type *bus, char *buf) {
    return snprintf(buf, PAGE_SIZE, "%s\n", Version);
}
BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);

static ssize_t show_device_version(struct device *dev, 
	struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%s\n", Device_version);
}
DEVICE_ATTR(version, S_IRUGO, show_device_version, NULL);

static int __init scbus_init(void) {
    int ret;

    ret = bus_register(&scbus_type);
    if (ret)
        return ret;

    ret = bus_create_file(&scbus_type, &bus_attr_version);
    if (ret)
        goto create_error;

    ret = device_register(&scbus);
    if (ret)
	goto bus_device_error;

    ret = device_register(&scbus_device);
    if (ret)
        goto device_error;

    ret = device_create_file(&scbus_device, &dev_attr_version);
    if (ret)
	    goto create_file_err;
    printk("Create a scbus_device device\n");

    printk("Create a scbus\n");
    return 0;

create_file_err:
    device_unregister(&scbus_device);
bus_device_error:
device_error:
    bus_remove_file(&scbus_type, &bus_attr_version);
create_error:
    bus_unregister(&scbus_type);
    return ret;
}

static void __exit scbus_exit(void) {
    device_remove_file(&scbus_device, &dev_attr_version);
    device_unregister(&scbus_device);
    printk("Remove a device\n");

    device_unregister(&scbus);
    bus_remove_file(&scbus_type, &bus_attr_version);
    bus_unregister(&scbus_type);
    printk("Remove a scbus\n");
}

module_init(scbus_init);
module_exit(scbus_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com");
