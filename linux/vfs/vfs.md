# 虚拟文件系统
虚拟文件系统（Virtual Filesystem, VFS）是内核的一个组件，用于处理与文件和文件系统相关的所有系统调用。VFS是用户与特定文件系统之间的通用接口。这种抽象简化了文件系统的实现，并提供了更容易的多文件系统集成这样，文件系统的实现是通过使用VFS提供的API完成的，通用硬件和I/O子系统通信部分由VFS处理。

VFS的基本思想是提供一个可以表示来自任何文件系统的文件的文件模型。文件系统驱动程序负责底层操作。 这样，内核可以创建包含整个系统的单个目录结构。将有一个文件系统作为根，其余文件系统将安装在其各种目录中。

## 通用文件系统模型
通用文件系统模型用`super_block`、`inode`、`dentry`和`file`四个对象来表征任何文件系统。

### `super_block`
`super_block`代表一个具体某个已经挂载的文件系统（每个物理的磁盘、硬盘都有一个文件控制块FCB，super_block相当于FCB的内存映像）。标识一个文件系统的信息，比如：
- 依附的物理硬件
- 索引结点`inode`和数据块`block`的位置
- `block`的大小（字节）
- 文件系统类型
- 最长文件名
- 最大文件大小
- 根目录的`inode`位置
- 支持的操作

```
/// @file include/linux/fs.h
1182 struct super_block {
1183     struct list_head                 s_list;     
1184     dev_t                            s_dev;              // 设备标识符
1185     unsigned char                    s_blocksize_bits;   // blocksize的位数
1186     unsigned long                    s_blocksize;        // 块大小（字节）
1187     loff_t                           s_maxbytes;         // 最大文件大小（字节）
1188     struct file_system_type         *s_type;             // 文件系统类型
1189     const struct super_operations   *s_op;               // 超级块方法（函数操作）
/// ...
1193     unsigned long                    s_flags;            // 挂载标志
1194     unsigned long                    s_iflags;           // 
1195     unsigned long                    s_magic;            // 文件系统的魔术字
1196     struct dentry                   *s_root;             // 挂载点
1197     struct rw_semaphore              s_umount;           // 超级块信号量
1198     int                              s_count;            // 超级块引用计数
1199     atomic_t                         s_active;           // 活动引用计数
/// ...
1205     struct list_head                 s_inodes;           // inode链表
/// ...
1215 
1216     char                             s_id[32];           // 名字
1217     u8                               s_uuid[16];         // UUID
1219     void                            *s_fs_info;          // 文件系统特殊信息
1220     unsigned int                     s_max_links;
1221     fmode_t                          s_mode;             // 安装权限
/// ...
```

`struct super_operations`中定义了超级块支持的操作，是一组函数指针
```
/// @file include/linux/fs.h
1555 struct super_operations {
1556     struct inode *(*alloc_inode)(struct super_block *sb);
1557     void (*destroy_inode)(struct inode *);
1558 
1559     void (*dirty_inode) (struct inode *, int flags);
1560     int (*write_inode) (struct inode *, struct writeback_control *wbc);
1561     int (*drop_inode) (struct inode *);
1562     void (*evict_inode) (struct inode *);
1563     void (*put_super) (struct super_block *);
1564     int (*sync_fs)(struct super_block *sb, int wait);
1565     int (*freeze_fs) (struct super_block *);
1566     int (*unfreeze_fs) (struct super_block *);
1567     int (*statfs) (struct dentry *, struct kstatfs *);
1568     int (*remount_fs) (struct super_block *, int *, char *);
1569     void (*umount_begin) (struct super_block *);
1570 
1571     int (*show_options)(struct seq_file *, struct dentry *);
1572     int (*show_devname)(struct seq_file *, struct dentry *);
1573     int (*show_path)(struct seq_file *, struct dentry *);
1574     int (*show_stats)(struct seq_file *, struct dentry *);
1575 #ifdef CONFIG_QUOTA
1576     ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
1577     ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
1578 #endif
1579     int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
1580     long (*nr_cached_objects)(struct super_block *, int);
1581     long (*free_cached_objects)(struct super_block *, long, int);
1582 };
```
比较重要的操作
- `write_inode()`, `alloc_inode()`, `destroy_inode()`：写、分配和释放`indoe`
- `put_super()`：卸载文件系统时由VFS系统调用，用于释放`super_block`
- `sync_fs()`：文件内容同步
- `remount_fs()`：当指定新的标识重新挂载文件系统时，VFS调用此函数
- `statfs()`：获取文件系统状态，返回文件信息

### `inode`
索引结点`inode`包含了内核在操作文件或目录时需要的全部信息。对于UNIX风格的文件系统，这些信息可以根据需要从磁盘索引结点直接读入或者写会磁盘。磁盘上的一个索引结点代表一个文件，内核中一个`inode`代表打开的一个文件。
- 文件类型
- 文件大小
- 访问权限
- 访问或修改时间
- 文件位置（指向磁盘数据块）

```
/// @file include/linux/fs.h
506 struct inode {
507     umode_t                          i_mode;       // 访问权限
508     unsigned short                   i_opflags;
509     kuid_t                           i_uid;        // 使用者ID
510     kgid_t                           i_gid;        // 使用组ID
511     unsigned int                     i_flags;      // 文件系统标志
518     const struct inode_operations   *i_op;         // 索引结点操作
519     struct super_block              *i_sb;         // 本inode所属超级块
520     struct address_space            *i_mapping;    // 地址映射相关
527     unsigned long                    i_ino;        // 索引结点号
535     union {
536         const unsigned int           i_nlink;      // 连接到本inode的dentry的数目
537         unsigned int               __i_nlink;
538     };
539     dev_t                            i_rdev;       // 挂载目录的设备标识符
540     loff_t                           i_size;       // 文件大小（字节）
541     struct timespec                  i_atime;      // 最后访问时间
542     struct timespec                  i_mtime;      // 最后修改时间
543     struct timespec                  i_ctime;      // 最后改变时间
544     spinlock_t                       i_lock;       // 自旋锁
545     unsigned short                   i_bytes;      // 使用的字节数
546     unsigned int                     i_blkbits;    // blocksize的位数
547     blkcnt_t                         i_blocks;     // 占用块数
/// ...
554     unsigned long                    i_state;      // 状文件态
/// ...
559     struct hlist_node                i_hash;
560     struct list_head                 i_wb_list;
561     struct list_head                 i_lru;        //
562     struct list_head                 i_sb_list;    // 超级块链表
563     union {
564         struct hlist_head            i_dentry;
565         struct rcu_head              i_rcu;
566     };
567     u64                              i_version;
568     atomic_t                         i_count;      // 引用计数
569     atomic_t                         i_dio_count;
570     atomic_t                         i_writecount; // 写者引用计数
571 #ifdef CONFIG_IMA
572     atomic_t                         i_readcount;  // 读者引用计数
573 #endif
581     union {
582         struct pipe_inode_info      *i_pipe;       // 具名管道文件
583         struct block_device         *i_bdev;       // 块设备文件
584         struct cdev                 *i_cdev;       // 字符设备文件
585     };
/// ...
595 };
```
`inode`相关的操作
```
/// @file include/linux/fs.h
1507 struct inode_operations {
1508     struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
1509     void * (*follow_link) (struct dentry *, struct nameidata *);
1510     int (*permission) (struct inode *, int);
1511     struct posix_acl * (*get_acl)(struct inode *, int);
1512 
1513     int (*readlink) (struct dentry *, char __user *,int);
1514     void (*put_link) (struct dentry *, struct nameidata *, void *);
1515 
1516     int (*create) (struct inode *,struct dentry *, umode_t, bool);
1517     int (*link) (struct dentry *,struct inode *,struct dentry *);
1518     int (*unlink) (struct inode *,struct dentry *);
1519     int (*symlink) (struct inode *,struct dentry *,const char *);
1520     int (*mkdir) (struct inode *,struct dentry *,umode_t);
1521     int (*rmdir) (struct inode *,struct dentry *);
1522     int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
1523     int (*rename) (struct inode *, struct dentry *,
1524             struct inode *, struct dentry *);
1525     int (*rename2) (struct inode *, struct dentry *,
1526             struct inode *, struct dentry *, unsigned int);
1527     int (*setattr) (struct dentry *, struct iattr *);
1528     int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
1529     int (*setxattr) (struct dentry *, const char *,const void *,size_t,int);
1530     ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
1531     ssize_t (*listxattr) (struct dentry *, char *, size_t);
1532     int (*removexattr) (struct dentry *, const char *);
1533     int (*fiemap)(struct inode *, struct fiemap_extent_info *, u64 start,
1534               u64 len);
1535     int (*update_time)(struct inode *, struct timespec *, int);
1536     int (*atomic_open)(struct inode *, struct dentry *,
1537                struct file *, unsigned open_flag,
1538                umode_t create_mode, int *opened);
1539     int (*tmpfile) (struct inode *, struct dentry *, umode_t);
1540     int (*set_acl)(struct inode *, struct posix_acl *, int);
1541 } ____cacheline_aligned;
```
比较重要的操作
- `lookup()`：用于在指定目录中根据名字查找一个`inode`，该索引结点要对应于指定的文件名
- `permission`：访问权限
- `create()`：为传入`dentry`对象创建一个新的索引结点
- `link()`、`unlink()`：建立连接和删除连接
- `mkdir()`、`rmdir`：建立目录和删除目录
- `mknod()`：创建一个设备文件，FIFO或者套接字文件
- `rename()`：重命名

### `dentry`
VFS把目录当作文件对待，但是没有一个具体的磁盘结构与之对应。
```
/// @file include/linux/dcache.h
108 struct dentry {
110     unsigned int                    d_flags;                   // 目录项表示
111     seqcount_t                      d_seq;
112     struct hlist_bl_node            d_hash;
113     struct dentry                  *d_parent;                  // 父目录
114     struct qstr                     d_name;                    // 目录项名字
115     struct inode                   *d_inode;                   // 关联的inode
117     unsigned char                   d_iname[DNAME_INLINE_LEN]; // 短文件名
120     struct lockref                  d_lockref;                 // 引用计数（带锁）
121     const struct dentry_operations *d_op;                      // 目录项操作
122     struct super_block             *d_sb;                      // 所属超级块
123     unsigned long                   d_time;                    // 重置时间
124     void                           *d_fsdata;                  // 文件系统特有数据
126     struct list_head                d_lru;                     // LRU链表
127     struct list_head                d_child;                   // 父目录孩子
128     struct list_head                d_subdirs;                 // 子目录
132     union {
133         struct hlist_node           d_alias;
134         struct rcu_head             d_rcu;
135     } d_u;
136 };
```
`dentry`相关操作
```
/// @file include/linux/dcache.h
150 struct dentry_operations {
151     int (*d_revalidate)(struct dentry *, unsigned int);
152     int (*d_weak_revalidate)(struct dentry *, unsigned int);
153     int (*d_hash)(const struct dentry *, struct qstr *);
154     int (*d_compare)(const struct dentry *, const struct dentry *,
155             unsigned int, const char *, const struct qstr *);
156     int (*d_delete)(const struct dentry *);
157     void (*d_release)(struct dentry *);
158     void (*d_prune)(struct dentry *);
159     void (*d_iput)(struct dentry *, struct inode *);
160     char *(*d_dname)(struct dentry *, char *, int);
161     struct vfsmount *(*d_automount)(struct path *);
162     int (*d_manage)(struct dentry *, bool);
163 } ____cacheline_aligned;
```
重要的操作
- `d_compare()`：比较两个文件名
- `d_delete()`：删除目录项
- `d_release()`：释放目录项
- `d_prune()`：
- `d_iput()`：当一个目录项对象丢失了其相关联的`inode`结点时，调用此函数
- `d_dname()`：延迟目录项生成，在真正需要时生成

### `file`
从进程的角度，标识打开的文件。主要维持如下信息
- 文件读写的标记的位置
- 打开文件的权限
- 指向inode的指针

```
/// @file include/linux/fs.h
751 struct file {
752     union {
753         struct llist_node            fu_llist;
754         struct rcu_head              fu_rcuhead;
755     } f_u;
756     struct path                      f_path;        // 文件路径
757 #define f_dentry                     f_path.dentry
758     struct inode                    *f_inode;       // inode缓存
759     const struct file_operations    *f_op;          // 文件操作

765     spinlock_t                       f_lock;        // 自旋锁
766     atomic_long_t                    f_count;       // 引用计数
767     unsigned int                     f_flags;       // 打开文件指定的标志
768     fmode_t                          f_mode;        // 访问权限
769     struct mutex                     f_pos_lock;    // 文件指针锁
770     loff_t                           f_pos;         // 文件指针偏移量
771     struct fown_struct               f_owner;       // 
787     struct address_space            *f_mapping;     // 内存映射相关该
795 };
```
文件相关的操作
```
/// @file include/linux/fs.h
1474 struct file_operations {
1475     struct module *owner;
1476     loff_t (*llseek) (struct file *, loff_t, int);
1477     ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
1478     ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
1479     ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
1480     ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
1481     ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
1482     ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
1483     int (*iterate) (struct file *, struct dir_context *);
1484     unsigned int (*poll) (struct file *, struct poll_table_struct *);
1485     long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
1486     long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
1487     int (*mmap) (struct file *, struct vm_area_struct *);
1488     int (*open) (struct inode *, struct file *);
1489     int (*flush) (struct file *, fl_owner_t id);
1490     int (*release) (struct inode *, struct file *);
1491     int (*fsync) (struct file *, loff_t, loff_t, int datasync);
1492     int (*aio_fsync) (struct kiocb *, int datasync);
1493     int (*fasync) (int, struct file *, int);
1494     int (*lock) (struct file *, int, struct file_lock *);
1495     ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
1496     unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
1497     int (*check_flags)(int);
1498     int (*flock) (struct file *, int, struct file_lock *);
1499     ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
1500     ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
1501     int (*setlease)(struct file *, long, struct file_lock **);
1502     long (*fallocate)(struct file *file, int mode, loff_t offset,
1503               loff_t len);
1504     int (*show_fdinfo)(struct seq_file *m, struct file *f);
1505 };
```
文件常用操作
- `llseek`：更新偏移位置
- `read`、`write`、`aio_read`、`aio_write`：文件读写
- `poll`：进程检查此文件上是否有活动并且（可选）进入睡眠状态直到有活动时唤醒
- `mmap`：内存映射
- `open`：文件打开
- `flush`：文件刷新
- `release`：当最后一个引用的文件关闭，调用

## 文件系统注册
这里的文件系统是指可能会被挂载到目录树中的各个实际文件系统，所谓实际文件系统，即是指VFS中的实际操作最终要通过它们来完成而已，并不意味着它们一定要存在于某种特定的存储设备上。分别通过`register_filesystem`和`unregister_filesystem`完成文件系统的注册和移除。注册过程实际上将表示各实际文件系统的`struct file_system_type`数据结构的实例化，然后形成一个链表，内核中用一个名为`file_systems`的全局变量来指向该链表的表头。
```
#include <linux/fs.h>
extern int register_filesystem(struct file_system_type *);
extern int unregister_filesystem(struct file_system_type *);
```

### `file_system_type`
Linxu用`file_system_type`来表征一个文件系统的类型
```
/// @file include/linux/fs.h
1768 struct file_system_type {
1769     const char               *name;          // 名字，比如“ext2”
1770     int                       fs_flags;      // 类型标志，在下面定义
1771 #define FS_REQUIRES_DEV       1 
1772 #define FS_BINARY_MOUNTDATA   2
1773 #define FS_HAS_SUBTYPE        4
1774 #define FS_USERNS_MOUNT       8 
1775 #define FS_USERNS_DEV_MOUNT   16
1776 #define FS_USERNS_VISIBLE     32
1777 #define FS_NOEXEC             64
1778 #define FS_RENAME_DOES_D_MOVE 32768
1779     struct dentry *(*mount) (struct file_system_type *, int,
1780                const char *, void *);        // 挂载操作
1781     void (*kill_sb) (struct super_block *);  // 在unmount时清理所属的超级块
1782     struct module            *owner;
1783     struct file_system_type  *next;          // 下一个file_system_type
1784     struct hlist_head         fs_supers;
1785 
1786     struct lock_class_key     s_lock_key;
1787     struct lock_class_key     s_umount_key;
1788     struct lock_class_key     s_vfs_rename_key;
1789     struct lock_class_key     s_writers_key[SB_FREEZE_LEVELS];
1790 
1791     struct lock_class_key     i_lock_key;
1792     struct lock_class_key     i_mutex_key;
1793     struct lock_class_key     i_mutex_dir_key;
1794 };
```

### `register_filesystem`
功能是将`fs`对象添加到全局的`file_systems`对象链表中。如果`file_systems`对象链表中有与`fs`同名字的对象，则返回错误的值，否则返回0.
```
/// @file fs/filesystems.c
46 static struct file_system_type **find_filesystem(const char *name, unsigned len)
47 {   
48     struct file_system_type **p;
49     for (p=&file_systems; *p; p=&(*p)->next) // 牛逼NB
50         if (strlen((*p)->name) == len &&
51             strncmp((*p)->name, name, len) == 0)
52             break;
53     return p;
54 }

69 int register_filesystem(struct file_system_type * fs)
70 {
71     int res = 0;
72     struct file_system_type ** p;
73 
74     BUG_ON(strchr(fs->name, '.'));
75     if (fs->next)
76         return -EBUSY;
77     write_lock(&file_systems_lock);
78     p = find_filesystem(fs->name, strlen(fs->name));
79     if (*p)
80         res = -EBUSY;
81     else
82         *p = fs;
83     write_unlock(&file_systems_lock);
84     return res;
85 }
```

### `unregister_filesystem`
```
101 int unregister_filesystem(struct file_system_type * fs)
102 {
103     struct file_system_type ** tmp;
104 
105     write_lock(&file_systems_lock);
106     tmp = &file_systems;
107     while (*tmp) {
108         if (fs == *tmp) {
109             *tmp = fs->next;
110             fs->next = NULL;
111             write_unlock(&file_systems_lock);
112             synchronize_rcu();
113             return 0;
114         }
115         tmp = &(*tmp)->next;
116     }
117     write_unlock(&file_systems_lock);
118 
119     return -EINVAL;
120 }
```

## 文件系统挂载
TODO...