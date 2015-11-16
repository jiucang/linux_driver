##Content：
-**1. Linux 文件系统目录结构**
-**2. Linux文件系统与设备驱动**
-**3. devfs设备文件系统**
-**4. udev设备文件系统**

 - udev和devfs区别
 
 - sysfs文件系统
 
-**5. Nand flash & SD card file system demo**

##1. Linux 文件系统目录结构
```
➜  /  tree -L 1 
.
├── bin
├── boot
├── cdrom
├── dev
├── etc
├── home
├── initrd.img -> boot/initrd.img-3.19.0-25-generic
├── lib
├── lib32
├── lib64
├── libx32
├── lost+found
├── media
├── mnt
├── opt
├── proc	//内存中的虚拟文件系统
├── root
├── run
├── sbin
├── srv
├── sys		//sysfs文件系统被映射到该目录，linux设备驱动模型层的总线，驱动，设备都可以在sysfs中有对应节点。当内核检测到在系统中出现了新设备后，内核会在sysfs文件系统中对应为该设备生成一项新的记录。
├── tmp
├── usr
├── var
└── vmlinuz -> boot/vmlinuz-3.19.0-25-generic
23 directories, 2 files
```

##2. Linux文件系统与设备驱动
###文件系统和设备驱动
![文件系统和设备驱动](http://img.blog.csdn.net/20151107211408152)

 - 应用程序和VFS之间通过文件操作系统调用来实现交互
 
 - VFS 虚拟文件系统存在的价值是可以兼容多种文件系统
 
 - VFS和特殊文件/磁盘文件(Ramdisk/flash/rom/sd card/u flash等文件系统)/设备文件 之间交互通过file_operations函数的调用
 
 - 磁盘文件系统中实现了对应的file_operations操作，磁盘驱动中调用上层的接口即可
 
 - 设备文件中，如字符设备则直接在设备驱动中定义了file_operations操作

###设备驱动开发中需要涉及的结构体：file & inode
 - file: 一个打开的文件在内存中就对应了一个file结构体，可以理解为是一个用户层定义的结构体
包括：用户需要的打开方式，文件读写位置，文件属于的uid/gid等；并包含了文件操作struct file_operations *f_op；指针
在内核和驱动代码中，struct file的指针通常被命名为file 和　filp（file pointer）

	```
struct file
{
	union {
		struct file_head fu_list;
        struct rou_head fu_rcuhead;
	} f_u;
    struct dentry *f_dentry;	/*与文件关联的目录入口(dentry)结构*/
    struct vfsmount *f_vfsmount;
    struct file_operations *f_op;	/*和文件关联的操作*/
    automic_t f_count;
    unsigned int f_flags;	/*文件标志，如Ｏ_RDONLY, O_NONBLOCK, O_SYNC*/
    mode_t f_mode;	/*文件读写模式，FMODE_READ & FMODE_WRITE*/
    loff_t f_pos;	/*当前读写位置*/
    struct fown_struct f_owner;
    unsigned int f_uid, f_gid;
    struct file_ra_state f_ra;
    
    unsigned long f_version;
    void *f_security;
    
    /*tty驱动需要，其他的也许需要*/
    void *private_data;　／*文件私有数据*/
    ...
    struct address_space *f_mapping;
};
```
**private_data: **在设备驱动中使用广泛，大多被指向设备驱动自定义用于描述设备的结构体。
测试文件读写方式：
```
if (file->f_mode & FMODE_WRITE)	用户是否可写
if (file->f_mode & FMODE_READ)　用户是否可读
```
测试文件打开方式：
```
if (file->f_flags & O_NONBLOCK)		/*NONE BLOCK*/
	pr_debug("open: non-blocking\n");
else 								  /*BLOCK*/
	pr_debug("open: blocking\n");
```
		
- inode: vfs inode　包括文件相关的元数据，包括文件访问权限，数组，组，大小，生成时间，访问时间，最后修改时间等信息。是linux中管理文件系统的最基本单位

  ```
struct inode {
	...
    umode_t i_mode;	/*inode的权限*/
    uid_t i_uid;	/*inode拥有者的id*/
    gid_t i_gid;	/*inode所属的群组id*/
    dev_t i_rdev;	/*如果是设备文件，该字段记录设备的设备号*/
    loff_t i_size;	／*inode所代表的文件大小*/
    
    //inode最近一次的存取时间，修改时间，生成时间
    struct timespec i_atime;
    struct timespec i_mtime;
    struct timespec i_ctime;
    
    unsigned long i_blksize;	/*inode在做IO时的区块大小*/
    unsigned long i_blocks;		/*inode所使用的block数，一个block为512byte*/
    
    struct block_device *i_bdev;	/*若是快设备,为其对应block_device结构体指针*/
    struct cdev *i_cdev;			/*如是字符设备，为其对应的cdev结构体指针*/
}
```
对于设备文件的inode结构，dev_t i_rdev;该字段包含了设备号，linux2.6设备编号分为主次设备号，前者为dev_t的高１２位，后者位后２０位。
对应操作函数：
**unsigned int iminor(struct inode *inode);**
**unsigned int imajor(struct inode *inode);**
主设备号对应的是一类设备，而次设备号则是对应于此类设备下的具体设备。一个驱动可以支持多种设备，使用次设备号来区分使用该驱动的设备。

##３.devfs设备文件系统
devfs有些过时了，不做细说
xxx_module_init(void) {
	register_chrdev()	/*在内核中注册设备*/
    devfs_register()	/*创建设备文件*/
}

xxx_module_exit(void) {
	devfs_unregister()	/*撤销设备文件*/
    unregister_chrdev()　/*注销设备*/
}

##4. Udev 设备文件系统
**特点：**
- 是一种用户空间的策略机制，符合linux中的机制和策略分离思路
- 在设备热插拔的时候加载／卸载内核模块，并生成对应的设备文件，在/sys/中的sysfs文件系统中有体现。
- 可以在用户态自定义创建设备文件创建规则，包括设备文件命名策略，权限控制，时间处理等

###udev和devfs区别
个人认为和devfs的最大区别是驱动模块加载时机不一样，对于一个不存在的/dev节点被打开,devfs能自动加载对应驱动，而udev不这么做。**UDEV的设计者认为应该在设备被发现的时候加载驱动模块，而不是在它被访问的时候。**

###sysfs文件系统
sysf是2.6引入的一个虚拟文件系统，和proc,devfs,devpty等为同等级的文件系统。和提供进程和状态信息的proc文件系统类似。展示了设备驱动模型中各个组件的层次关系。顶级目录包括：block, device, bus, drivers, class, power, firmware.加入设备模型一开始就是为了解决电源管理和热插拔的问题。

/sys/bus/: 下面存在drivers和devices的目录，下面都对应于/sys/drivers, /sys/drivers下的链接
/sys/class: 下面包含了/sys/devices下的文件的链接

**驱动加载匹配流程：**主要通过总线的match函数
１．系统启动，bus对应的驱动被加载，下面提供了它所管理的设备和驱动
２．驱动加载
３．设备加载，主动寻找对应的bus上是否有match的驱动程序，如果有，则加载之。

```
struct bus_type {
    const char      *name;		//总线名
    struct bus_attribute    *bus_attrs;		//总线属性，每一个attribute对应于sysfs中的一个文件
    struct device_attribute *dev_attrs;	　　//设备属性
    struct driver_attribute *drv_attrs;		//驱动属性
    int (*match)(struct device *dev, struct device_driver *drv);	//设备和驱动的匹配函数，主要就是查看name是否一致
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);	//底层的时间，如热插拔等，会一直返回给上层
    int (*probe)(struct device *dev);			//当设备和驱动配置时调用，用于初始化设备
    int (*remove)(struct device *dev);			//设备移除时调用
    void (*shutdown)(struct device *dev);		//shutdown的时候使用
    int (*suspend)(struct device *dev, pm_message_t state);		//以下都是电源管理函数
    int (*resume)(struct device *dev);
    const struct dev_pm_ops *pm;
    struct subsys_private *p;		//总线的私有数据指针
};
```

```
struct device_driver {
	const char		*name;		//驱动名称
	struct bus_type		*bus;		//所属总线
	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */
	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
	const struct of_device_id	*of_match_table;
	int (*probe) (struct device *dev);		//设备驱动匹配总线时调用
	int (*remove) (struct device *dev);		//设备移除时调用
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, pm_message_t state);	//设备驱动的电源管理操作
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;
	const struct dev_pm_ops *pm;
	struct driver_private *p;		//设备驱动的私有数据
};

```

```
struct device {
	struct device		*parent;		//父设备，常为某种总线或者宿主设备
	struct device_private	*p;
	struct kobject kobj;
	const char		*init_name; /* initial name of the device */
	struct device_type	*type;
	struct mutex		mutex;	/* mutex to synchronize calls to
					 * its driver.
					 */
	struct bus_type	*bus;		/* type of bus device is on 设备所在的总线类型*/
	struct device_driver *driver;	/* which driver has allocated this device 设备用到的驱动*/
	void		*platform_data;	/* Platform specific data, device core doesn't touch it */
	struct dev_pm_info	power;
	struct dev_power_domain	*pwr_domain;
    dev_t devt;					/*dev_t，创建sysfs "dev"　*/
	...
};
```














