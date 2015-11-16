#LINUX设备驱动模型

##SYSFS
sysfs是处于内存中的虚拟文件系统，为我们提供了kobject的层次结构试图.

**API List:**

- 在sysfs中创建属性文件
```
static inline int __must_check sysfs_create_file(struct kobject *kobj,
						 const struct attribute *attr);
```
- 在sysfs中删除属性文件
```
static inline void sysfs_remove_file(struct kobject *kobj,
				     const struct attribute *attr);
```
- sysfs中创建和删除二进制文件
```
int __must_check sysfs_create_bin_file(struct kobject *kobj,
				       const struct bin_attribute *attr);
void sysfs_remove_bin_file(struct kobject *kobj,
			   const struct bin_attribute *attr);
```

##kobject:

- linux中设备模型的基础，嵌入到kset和device,driver等设备中，用于构成对应拓扑结构
- kobject中存在引用计数
- kobject对应sysfs中的一个目录,attribute对应于sysfs中的一个文件


**Kobject在sysfs中对应一个目录：**
```
struct kobject {
        const char              *name;   //名称
        struct list_head        entry;   //链入所属的kset链表
        struct kobject          *parent;	//指向kobject的父对象
        struct kset             *kset;	//所属kset
        struct kobj_type        *ktype;	//所属ktype
        struct kernfs_node      *sd;	//sysfs中目录项
        struct kref             kref;	//引用计数
#ifdef CONFIG_DEBUG_KOBJECT_RELEASE
        struct delayed_work     release;
#endif
        unsigned int state_initialized:1;		//表明该kobject是否被初始化了
        unsigned int state_in_sysfs:1;			//是否已经加入sysfs中
        unsigned int state_add_uevent_sent:1;		//已经发出KOBJ_ADD uevent
        unsigned int state_remove_uevent_sent:1;	//已经发出KOBJ_REMOVE　uevent
        unsigned int uevent_suppress:1;				//禁止发送uevent
};
```

**attribute在sysfs中对应于一个文件：**
```
struct attribute {
        const char              *name;	//属性名称
        umode_t                 mode;	//属性的访问模式,3.x之后不建议对other赋予权限
#ifdef CONFIG_DEBUG_LOCK_ALLOC
        bool                    ignore_lockdep:1;
        struct lock_class_key   *key;
        struct lock_class_key   skey;
#endif
};
```
一般使用封装后的kobj_attribute，对于该结构体的赋值可以使用宏定义完成，__ATTR(name, mode, show, store)
```
struct kobj_attribute {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
	ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count);
};

```


**API List:**
/include/linux/kobject.h

- 初始化：
void kobject_init(struct kobject *kobj, struct kobj_type *ktype);

- 初始化和添加：
int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype, struct kobject *parent, const char *fmt, ...);

- 创建和添加：
 struct kobject * __must_check kobject_create(void);
 struct kobject * __must_check kobject_create_and_add(const char *name, struct kobject *parent);

- 删除：
void kobject_del(struct kobject *kobj);

- 引用计数加一：
struct kobject *kobject_get(struct kobject *kobj);

- 引用计数减一：
char kobject_put(struct kobject *kobj);

##kset:

- 类似于容器的概念
- 



**总结：**
kobject, kset, ktype

- kobject：是设备模型中的基本对象，包含了引用计数，父子关系，目录项等，通常会迁入到其他的数据结构中，使其也具有kobject的特性；

- ktype:　定义了一些kobject相关的默认特性，包括析构函数，sysfs操作以及默认值；

- kset: 实现两个功能；

１）　其中嵌入的kobject作为kobject组的基类
２）　kset将相关的kobject集合在一起

![Kboject&kset关系图]( /home/jacob/Pictures/0_13226558510SGZ.gif)


bus:

```
/**
 * struct subsys_private - structure to hold the private to the driver core portions of the bus_type/class structure.
 *
 * @subsys - the struct kset that defines this subsystem
 * @devices_kset - the subsystem's 'devices' directory
 * @interfaces - list of subsystem interfaces associated
 * @mutex - protect the devices, and interfaces lists.
 *
 * @drivers_kset - the list of drivers associated
 * @klist_devices - the klist to iterate over the @devices_kset
 * @klist_drivers - the klist to iterate over the @drivers_kset
 * @bus_notifier - the bus notifier list for anything that cares about things
 *                 on this bus.
 * @bus - pointer back to the struct bus_type that this structure is associated
 *        with.
 *
 * @glue_dirs - "glue" directory to put in-between the parent device to
 *              avoid namespace conflicts
 * @class - pointer back to the struct class that this structure is associated
 *          with.
 *
 * This structure is the one that is the actual kobject allowing struct
 * bus_type/class to be statically allocated safely.  Nothing outside of the
 * driver core should ever touch these fields.
 */
struct subsys_private {
	struct kset subsys;		//定义该subsystem，在sys中表现为bus中的目录
	struct kset *devices_kset;　//对应subsystem子系统的device目录
	struct list_head interfaces;	//
	struct mutex mutex;

	struct kset *drivers_kset;		//相关的drivers，对应于subsystem子系统的driver目录
	struct klist klist_devices;		//klist内部就是封装了一个list，用于遍历devices_kset
	struct klist klist_drivers;		//用于遍历drivers_kset
	struct blocking_notifier_head bus_notifier;		//
	unsigned int drivers_autoprobe:1;
	struct bus_type *bus;	//指向该subsys_private关联的bus_type结构体

	struct kset glue_dirs;
	struct class *class;　//指向该subsys_private关联的class
};
#define to_subsys_private(obj) container_of(obj, struct subsys_private, subsys.kobj)

```