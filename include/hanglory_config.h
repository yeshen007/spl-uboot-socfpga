#ifndef HANGLORY_CONFIG_
#define HANGLORY_CONFIG_

#define HANGLORY_SPL_BOOT_UBOOT
#define HANGLORY_UBOOT_BOOT_KERNEL

#define CONFIG_HANGLORY_SPI_U_BOOT_OFFS 0x2060000
#define CONFIG_SYS_UBOOT_START CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN (200 * 1024)
#define CONFIG_SPL_SPI_CS		0
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_KERNEL_LOAD_ADDR		0x8000

/* uboot2012中的spl_image_info */
struct hanglory_image_info {
	const char *name;
	u8 os;
	u32 load_addr;
	u32 entry_point;
	u32 size;
	u32 flags;
	u32 crc;
	u32 crc_size;
};

extern void hanglory_spl_boot(void);

#endif
