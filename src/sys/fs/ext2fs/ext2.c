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
    int32_t count, i;
    int result;
    uint32_t size;

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

    (*fs)->block_size = 1024 << (*fs)->superblock.s_log_block_size;
    (*fs)->group_count = (*fs)->superblock.s_blocks_count / (*fs)->superblock.s_blocks_per_group;
    size = EXT2_SECTORS((*fs)->group_count * sizeof(struct ext2_group_descriptor));
    (*fs)->groups = (struct ext2_group_descriptor*) kmalloc(size);

    // read the group descriptors
    kprintf("Reading %d bytes\n", size);
    result = kdev_read(device, (*fs)->groups, size, 4, 0);
    if (result != size)
    {
        kprintf("Error reading group descriptors\n");
        result = -EINVAL;
        goto ESCAPE;
    }
    // read the block bitmap for each group
    size = EXT2_SECTORS( (*fs)->group_count * (*fs)->block_size );
    (*fs)->block_bitmap = (uint8_t*) kmalloc(size);
    for (i = 0; i < (*fs)->group_count; ++i)
    {
        result = kdev_read(device, (*fs)->block_bitmap + (i * 1024), SECTORSIZE * 2, EXT2_BLOCK_SECTOR((*fs)->groups[i].g_block_bitmap), 0);
        if (result != SECTORSIZE * 2)
            panic("fuu");
        kprintf("Group #%d have %d free blocks and bitmap starts from %d\n", i, (*fs)->groups[i].g_free_blocks_count, (*fs)->groups[i].g_block_bitmap);
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
