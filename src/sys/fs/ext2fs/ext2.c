#include <fs/ext2fs.h>
#include <errno.h>
#include <os/vfs.h>
#include <os/dev.h>


int ext2_open_filesystem(
    const char *deviceName,
    //const struct fsoptions *options,
    struct ext2_filesystem **fs )
{
    dev_t device;
    int32_t count;
    int result;

    device = kdev_open(deviceName);
    if (device < 0) return -ENODEV;
    if (kdev_get(device)->driver->type != DEV_TYPE_BLOCK) return -EINVAL;
    count = kdev_ioctl(device, IOCTL_GETDEVSIZE, NULL, 0);
    if (count < 0) return -EINVAL;

    // allocate memory for superblock
    *fs = (struct ext2_filesystem*) kmalloc( sizeof(struct ext2_filesystem) );
    if (*fs == NULL) return -ENOMEM;
    // read the superblock
    result = kdev_read(device, &(*fs)->superblock, SECTORSIZE * 2, 2, 0);
    if (result != SECTORSIZE * 2)
    {
        result = -ENODATA;
        goto ESCAPE;
    }

    // check the filesystem signature
    if ((*fs)->superblock.s_magic != EXT2_SUPER_MAGIC)
    {
        kprintf("Invalid magic number\n");
        result = -EINVAL;
        goto ESCAPE;
    }

    return 0;
ESCAPE:
    if (*fs != NULL) kfree(*fs);
    *fs = NULL;

    return result;

}


int ext2_mount(
    struct fs *fs,
    char *opts )
{
    return ext2_open_filesystem(fs->mntfrom, (struct ext2_filesystem **)&fs->data);
}


static struct fsops ext2fs =
{
    FSOP_OPEN | FSOP_CLOSE | FSOP_FSYNC | FSOP_READ |
    FSOP_TELL | FSOP_LSEEK | FSOP_STAT | FSOP_FSTAT |
    FSOP_OPENDIR | FSOP_READDIR,

    NULL,
    NULL,

    NULL,
    ext2_mount,
    NULL,

    NULL,

    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,

    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,
    NULL,

    NULL,
    NULL
};

void ext2_initialize()
{
    register_filesystem("ext2fs", &ext2fs);
}
