#ifndef MACHINA_FS_EXT2
#define MACHINA_FS_EXT2


#include <stdint.h>
#include <os/kmalloc.h>


#define EXT2_SUPER_MAGIC      (0xEF53)


/**
 * @brief ext2 superblock
 */
struct ext2_superblock
{
    int32_t  s_inodes_count;         // Total number of inodes in the filesystem (used and free)
    int32_t  s_blocks_count;         // Total number of blocks in the filesystem (used, free and reserved)
    int32_t  s_r_blocks_count;       // Total number of blocks reserved for super user
    int32_t  s_free_blocks_count;    // Total number of free blocks (include reserved blocks)
    int32_t  s_free_inodes_count;    // Total number of free inodes
    int32_t  s_first_data_block;     // First data block
    int32_t  s_log_block_size;       // Value to be shifted by 1024 and result in the block size
    int32_t  s_log_frag_size;        // Value to be shifted by 1024 and result in the fragment size
    int32_t  s_blocks_per_group;     // Total number of blocks per group
    int32_t  s_frags_per_group;      // Total number of fragments per group
    int32_t  s_inodes_per_group;     // Total number of inodes per group
    int32_t  s_mtime;                // POSIX time of the last time the filesystem was mounted
    int32_t  s_wtime;                // POSIX time of the last write access to the filesystem
    int16_t  s_mnt_count;            // Mount count since the last full check
    int16_t  s_max_mnt_count;        // Maximum mount count before a full check is performed
    uint16_t s_magic;                // ext2 image magic number (0xEF53)
    int16_t  s_state;                // Filesystem state
    int16_t  s_errors;               // Behaviour when detecting errors
    int16_t  s_minor_rev_level;      // Minor revision level
    int32_t  s_lastcheck;            // POSIX time of last check
    int32_t  s_checkinterval;        // Maximum POSIX interval allowed between filesystem checks
    int32_t  s_creator_os;           // OS/Kernel that created the filesystem
    int32_t  s_rev_level;            // Revision level
    int16_t  s_def_resuid;           // Default user id for reserved blocks
    int16_t  s_def_resgid;           // Default gtoup id for reserved blocks
    /*
     * These fields are for EXT2_DYNAMIC_REV superblocks only.
     *
     * If there is a bit set in the incompatible feature set that
     * the kernel doesn't support, the filesystem will not be mounted.
     */
    int32_t  s_first_ino;            // First inode useable for standard file
    int16_t  s_inode_size;           // Size of inode structure */
    int16_t  s_block_group_nr;       // Number of the block group hosting this superblock structure
    int32_t  s_feature_compat;       // Bitmask of compatible features
    int32_t  s_feature_incompat;     // Bitmask of incompatible features
    int32_t  s_feature_ro_compat;    // Bitmask of "read-only" features
    uint8_t  s_uuid[16];             // 128-bit uuid of the volume
    uint8_t  s_volume_name[16];      // Volume name (mostly unused)
    uint8_t  s_last_mounted[64];     // Path where the filesystem was last mounted
    int32_t  s_algo_bitmap;          // Used by compression algorithms to determine the compression method used
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT2_COMPAT_PREALLOC flag is on.
     */
    uint8_t  s_prealloc_blocks;      // Number of blocks to pre-allocate when creating a new regular file
    uint8_t  s_prealloc_dir_blocks;  // Number of blocks to pre-allocate when creating a new directory
    uint16_t s_padding1;
    /*
     * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
     */
    uint8_t  s_journal_uuid[16];     // uuid of the journal superblock
    uint32_t s_journal_inum;         // inode number of the journal file
    uint32_t s_journal_dev;          // Device number of the the journal file
    uint32_t s_last_orphan;          // inode number of the list (start) of inodes to delete
    uint32_t s_hash_seed[4];         // Seeds used for the hash algorithm for directory indexing
    uint8_t  s_def_hash_version;     // Default hash version used for directory indexing
    uint8_t  s_padding2;
    uint16_t s_padding3;
    int32_t  s_default_mount_opts;   // Default mount options for this file system
    int32_t  s_first_meta_bg;        // Block group ID of the first metablock group
    uint32_t s_padding4[190];
};


struct ext2_filesystem
{
    struct ext2_superblock superblock;
    dev_t device;
};


void ext2_initialize();

#endif // MACHINA_FS_EXT2
