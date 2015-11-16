/**
 * for learn class
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

static char *Version = "revision 1.0, scclass";

struct class *scclass;
EXPORT_SYMBOL_GPL(scclass);

/*export class attribute */
static ssize_t class_show_version(struct class *class, 
    struct class_attribute *attr, char *buf) {
    return sprintf(buf, "%s\n", Version);
}
CLASS_ATTR(version, 0660, class_show_version, NULL);

static int __init scclass_init(void) {
    int err;

    /*Create a class named scclass */
    scclass = class_create(THIS_MODULE, "scclass");
    err = PTR_ERR(scclass);
    if (IS_ERR(scclass))
        return err;

    /*Create class attribute: version*/
    err = class_create_file(scclass, &class_attr_version);
    if (err)
        goto err_class_create;
 
    printk("class create\n");
    return 0;

err_class_create:
    class_destroy(scclass);
    return err;
}

static void __exit scclass_exit(void) {
    class_remove_file(scclass, &class_attr_version);
    class_destroy(scclass);
    printk("class destroy\n");
}

module_init(scclass_init);
module_exit(scclass_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com>");
