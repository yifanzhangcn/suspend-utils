/*
 * swsusp.h
 *
 * Common definitions for user space suspend and resume handlers.
 *
 * Copyright (C) 2005 Rafael J. Wysocki <rjw@sisk.pl>
 *
 * This file is released under the GPLv2.
 *
 */

#include <asm/page.h>
#include <stdint.h>

#include "encrypt.h"

#define SNAPSHOT_IOC_MAGIC	'3'
#define SNAPSHOT_FREEZE			_IO(SNAPSHOT_IOC_MAGIC, 1)
#define SNAPSHOT_UNFREEZE		_IO(SNAPSHOT_IOC_MAGIC, 2)
#define SNAPSHOT_ATOMIC_SNAPSHOT	_IOW(SNAPSHOT_IOC_MAGIC, 3, void *)
#define SNAPSHOT_ATOMIC_RESTORE		_IO(SNAPSHOT_IOC_MAGIC, 4)
#define SNAPSHOT_FREE			_IO(SNAPSHOT_IOC_MAGIC, 5)
#define SNAPSHOT_SET_IMAGE_SIZE		_IOW(SNAPSHOT_IOC_MAGIC, 6, unsigned long)
#define SNAPSHOT_AVAIL_SWAP		_IOR(SNAPSHOT_IOC_MAGIC, 7, void *)
#define SNAPSHOT_GET_SWAP_PAGE		_IOR(SNAPSHOT_IOC_MAGIC, 8, void *)
#define SNAPSHOT_FREE_SWAP_PAGES	_IO(SNAPSHOT_IOC_MAGIC, 9)
#define SNAPSHOT_SET_SWAP_FILE		_IOW(SNAPSHOT_IOC_MAGIC, 10, unsigned int)
#define SNAPSHOT_IOC_MAXNR	10

#define	LINUX_REBOOT_MAGIC1	0xfee1dead
#define	LINUX_REBOOT_MAGIC2	672274793

#define LINUX_REBOOT_CMD_RESTART	0x01234567
#define LINUX_REBOOT_CMD_HALT		0xCDEF0123
#define LINUX_REBOOT_CMD_POWER_OFF	0x4321FEDC
#define LINUX_REBOOT_CMD_RESTART2	0xA1B2C3D4
#define LINUX_REBOOT_CMD_SW_SUSPEND	0xD000FCE2

struct new_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

struct swsusp_info {
	struct new_utsname	uts;
	uint32_t		version_code;
	unsigned long		num_physpages;
	int			cpus;
	unsigned long		image_pages;
	unsigned long		pages;
	unsigned long		size;
	/* The following fields are added by the userland */
	loff_t			map_start;
	uint32_t		image_flags;
	unsigned char		checksum[16];
#ifdef CONFIG_ENCRYPT
	char			salt[IVEC_SIZE];
	struct RSA_data		rsa_data;
	struct encrypted_key	key_data;
#endif
} __attribute__((aligned(PAGE_SIZE)));

#define IMAGE_CHECKSUM		0x0001
#define IMAGE_COMPRESSED	0x0002
#define IMAGE_ENCRYPTED		0x0004
#define IMAGE_USE_RSA		0x0008

#define SWSUSP_SIG	"ULSUSPEND"

struct swsusp_header {
	char reserved[PAGE_SIZE - 20 - sizeof(loff_t)];
	loff_t image;
	char	orig_sig[10];
	char	sig[10];
} __attribute__((packed, aligned(PAGE_SIZE)));

static inline int freeze(int dev)
{
	return ioctl(dev, SNAPSHOT_FREEZE, 0);
}

static inline int unfreeze(int dev)
{
	return ioctl(dev, SNAPSHOT_UNFREEZE, 0);
}

static inline int atomic_snapshot(int dev, int *in_suspend)
{
	return ioctl(dev, SNAPSHOT_ATOMIC_SNAPSHOT, in_suspend);
}

static inline int atomic_restore(int dev)
{
	return ioctl(dev, SNAPSHOT_ATOMIC_RESTORE, 0);
}

static inline int free_snapshot(int dev)
{
	return ioctl(dev, SNAPSHOT_FREE, 0);
}

static inline int set_image_size(int dev, unsigned int size)
{
	return ioctl(dev, SNAPSHOT_SET_IMAGE_SIZE, size);
}

static inline void reboot(void)
{
	syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
		LINUX_REBOOT_CMD_RESTART, 0);
}

static inline void power_off(void)
{
	syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
		LINUX_REBOOT_CMD_POWER_OFF, 0);
}

/*
 *	The swap map is a data structure used for keeping track of each page
 *	written to a swap partition.  It consists of many swap_map_page
 *	structures that contain each an array of MAP_PAGE_SIZE swap area
 *	descriptiors.
 *
 *	These structures are stored on the swap and linked together with the
 *	help of the .next_swap member.
 *
 *	The swap map is created during suspend.  The swap map pages are
 *	allocated and populated one at a time, so we only need one memory
 *	page to set up the entire structure.
 *
 *	During resume we also only need to use one swap_map_page structure
 *	at a time.
 */

struct swap_area {
	loff_t offset;
	unsigned int size;
};

#define MAP_PAGE_ENTRIES	((PAGE_SIZE - sizeof(loff_t)) / sizeof(struct swap_area))

struct swap_map_page {
	struct swap_area entries[MAP_PAGE_ENTRIES];
	loff_t next_swap;
};

struct buf_block {
	unsigned short size;
	char data[PAGE_SIZE];
};

#define BUFFER_SIZE	0x20000

#define SNAPSHOT_DEVICE	"/dev/snapshot"
#define RESUME_DEVICE ""

#define IMAGE_SIZE	(500 * 1024 * 1024)

#define SUSPEND_LOGLEVEL	1
#define MAX_LOGLEVEL		8

#define GEN_PARAM	6

#ifdef CONFIG_COMPRESS
#define COMPRESS_PARAM	1
#else
#define COMPRESS_PARAM	0
#endif

#ifdef CONFIG_ENCRYPT
#define ENCRYPT_PARAM	2
#else
#define ENCRYPT_PARAM	0
#endif

#define PARAM_NO	(GEN_PARAM + COMPRESS_PARAM + ENCRYPT_PARAM)
