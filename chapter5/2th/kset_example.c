/**
 * For learn kset, kobject, ktype
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>

struct foo_obj {
    struct kobject kobj;
    int xoo;
    int yoo;
    int zoo;
};
#define to_foo_obj(x) container_of(x, struct foo_obj, kobj);

struct foo_attribute {
    struct attribute attr;
    ssize_t(*show)(struct foo_obj *foo, struct foo_attribute *attr, char *buf);
    ssize_t(*store)(struct foo_obj *foo, struct foo_attribute *attr, char *buf, ssize_t count);
};
#define to_foo_attr(x) container_of(x, struct foo_attribute, attr);

/**
 * ktype Public attribute related operations
 */
static ssize_t foo_attr_show(struct kobject *kobj, struct attribute *attr, char *buf) {
    struct foo_attribute *attribute;
    struct foo_obj *foo;

    attribute = to_foo_obj(attr);
    foo = to_foo_obj(kobj);

    if (!attribute->show)
        return -EIO;

    return attribute->show(foo, attribute, buf);
}

static ssize_t foo_attr_store(struct kobject *kobj, struct attribute *attr, char *buf, ssize_t count) {
    struct foo_attribute *attribute;
    struct foo_obj *foo;

    attribute = to_foo_attr(attr);
    foo = to_foo_obj(kobj);

    if (!attribute->store)
        return -EIO;

    return attribute->store(foo, attribute, buf, count);
}

static const struct sysfs_ops foo_sysfs_ops = {
    .show = foo_attr_show,
    .store = foo_attr_store,
};

static void foo_release(struct kobject *kobj) {
    struct foo_obj *foo;
    foo = to_foo_obj(kobj);
    kfree(foo);
}

static ssize_t xoo_show(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", foo_obj->xoo);
}

static ssize_t xoo_store(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf, ssize_t count) {
    sscanf(buf, "%du", &foo_obj->xoo);
    return count;
}

static ssize_t yoo_show(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", foo_obj->yoo);
}

static ssize_t yoo_store(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf, ssize_t count) {
    sscanf(buf, "%du", &foo_obj->yoo);
    return count;
}

static ssize_t zoo_show(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", foo_obj->zoo);
}

static ssize_t zoo_store(struct foo_obj *foo_obj, struct foo_attribute *attr, char *buf, ssize_t count) {
    sscanf(buf, "%du", &foo_obj->zoo);
    return count;
}

static struct foo_attribute xoo_attribute =
    __ATTR(xoo, 0660, xoo_show, xoo_store);

static struct foo_attribute yoo_attribute =
    __ATTR(yoo, 0660, yoo_show, yoo_store);

static struct foo_attribute zoo_attribute =
    __ATTR(zoo, 0660, zoo_show, zoo_store);

static struct attribute *foo_default_attrs[] = {
    &xoo_attribute.attr,
    &yoo_attribute.attr,
    NULL,
};

static struct kobj_type foo_ktype = {
    .sysfs_ops = &foo_sysfs_ops,
    .release = foo_release,
    .default_attrs = foo_default_attrs,
};

static struct kset *kset_example;
static struct foo_obj *aaa_obj;
static struct foo_obj *bbb_obj;

static struct foo_obj *create_foo_obj(const char *name) {
    struct foo_obj *foo;
    int ret;

    foo = kzalloc(sizeof(*foo), GFP_KERNEL);
    if (!foo)
        return NULL;

    foo->kobj.kset = kset_example;
    foo->xoo = 6;
    foo->yoo = 7;

    ret = kobject_init_and_add(&foo->kobj, &foo_ktype, NULL, "%s", name);
    if (ret) {
        kobject_put(&foo->kobj);
        return NULL;
    }
    kobject_uevent(&foo->kobj, KOBJ_ADD);

    return foo;
}

static void destroy_foo_obj(struct foo_obj *foo) {
    kobject_put(&foo->kobj);
}

static int __init example_init(void) {
    //kset name, kset_uevent_ops, kobject
    kset_example = kset_create_and_add("kset_example", NULL, NULL);
    if (!kset_example)
        return -ENOMEM;

    aaa_obj = create_foo_obj("aaa");
    if (!aaa_obj)
        goto aaa_error;

    bbb_obj = create_foo_obj("bbb");
    if (!bbb_obj)
        goto bbb_error;

    sysfs_create_file(&aaa_obj->kobj, &zoo_attribute.attr);

    return 0;

bbb_error:
    destroy_foo_obj(aaa_obj);
aaa_error:
    kset_unregister(kset_example);

    return -EINVAL;
}

static void __exit example_exit(void) {
    sysfs_remove_file(&aaa_obj->kobj, &zoo_attribute.attr);
    destroy_foo_obj(aaa_obj);
    destroy_foo_obj(aaa_obj);
    kset_unregister(kset_example);
}

module_init(example_init);
module_exit(example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CJOK<cjok.liao@gmail.com");
