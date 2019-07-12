/*该文件价为文件系统实例，主要是devfs和blkfs，blkfs主要整合ramdisk.c的接口 */
/*讲义， procfs和devfs没有对应的设备，所以dev为null */
/*在网上看到lookup的实现，用了hash，不过如果我设置的inode
 * number很少，可以遍历寻找，inode再存一个path，当然感觉怪怪的 */