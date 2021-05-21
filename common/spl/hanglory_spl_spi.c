/* hanglory double boot  */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>
#include <errno.h>
#include <spl.h>
#include <hanglory_config.h>
#include <image.h>
#include <malloc.h>


struct hanglory_image_info hanglory_spl_image; 

/* 分析头部信息，提取到hanglory_spl_image */
static void hanglory_spl_parse_image_header(const struct image_header *header)
{
	u32 header_size = sizeof(struct image_header);

	if (image_get_magic(header) == IH_MAGIC) {
		if (hanglory_spl_image.flags & SPL_COPY_PAYLOAD_ONLY) {
			/*
			 * On some system (e.g. powerpc), the load-address and
			 * entry-point is located at address 0. We can't load
			 * to 0-0x40. So skip header in this case.
			 */
			hanglory_spl_image.load_addr = image_get_load(header);
			hanglory_spl_image.entry_point = image_get_ep(header);
			hanglory_spl_image.size = image_get_data_size(header);
		} else {
			printf("coming here hahahahahahaha\n");
			hanglory_spl_image.entry_point = image_get_load(header);		//除去头部后的地址0x01000040
			/* Load including the header */
			hanglory_spl_image.load_addr = hanglory_spl_image.entry_point -
				header_size;					//头部地址0x01000000
			hanglory_spl_image.size = image_get_data_size(header) +
				header_size;					//镜像的长度（包括头部）
		}
		hanglory_spl_image.os = image_get_os(header);
		hanglory_spl_image.name = image_get_name(header);
		hanglory_spl_image.crc = image_get_dcrc(header);
		hanglory_spl_image.crc_size = image_get_data_size(header);		//crc的数据长度，不包括头部
		debug("spl: payload image: %s load addr: 0x%x size: %d\n",
			hanglory_spl_image.name, hanglory_spl_image.load_addr, hanglory_spl_image.size);
	} else {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
			header->ih_magic);
		/* Let's assume U-Boot will not be more than 200 KB */
		hanglory_spl_image.size = CONFIG_SYS_MONITOR_LEN;
		hanglory_spl_image.entry_point = CONFIG_SYS_UBOOT_START;
		hanglory_spl_image.load_addr = CONFIG_SYS_TEXT_BASE;
		hanglory_spl_image.os = IH_OS_U_BOOT;
		hanglory_spl_image.name = "U-Boot";
		hanglory_spl_image.crc_size = 0;
	}
}


/* 加载备份uboot到ddr,同时提取头部信息到hanglory_spl_image */
static void hanglory_spl_spi_load_image(void)
{
	struct spi_flash *flash;
	struct image_header *header;

	/*
	 * Load U-Boot image from SPI flash into RAM
	 */

	flash = spi_flash_probe(CONFIG_SPL_SPI_BUS, CONFIG_SPL_SPI_CS,
				CONFIG_SF_DEFAULT_SPEED, SPI_MODE_3);
	if (!flash) {
		puts("SPI probe failed.\n");
		hang();
	}

	/* use CONFIG_SYS_TEXT_BASE as temporary storage area */
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	/* Load u-boot, mkimage header is 64 bytes. */
	spi_flash_read(flash, CONFIG_HANGLORY_SPI_U_BOOT_OFFS, 0x40,
			(void *) header);
	hanglory_spl_parse_image_header(header);
	spi_flash_read(flash, CONFIG_HANGLORY_SPI_U_BOOT_OFFS,
		       hanglory_spl_image.size, (void *)hanglory_spl_image.load_addr);
}


/* crc校验和启动，参考jump_to_image_no_args() */
static void hanglory_spl_crc_check_and_boot(void)
{
	/* crc校验 */	
	u32 calculated_crc;
	if (hanglory_spl_image.crc_size != 0) {
		debug("Verifying Checksum ... ");
		calculated_crc = crc32_wd(0,
			(unsigned char *)hanglory_spl_image.entry_point,
			hanglory_spl_image.crc_size, CHUNKSZ_CRC32);
		if (calculated_crc != hanglory_spl_image.crc) {		/* 校验没通过，挂起 */
			puts("Bad image with mismatched CRC\n");
			debug("CRC calculate from 0x%08x "
				"with length 0x%08x\n",
				hanglory_spl_image.entry_point, hanglory_spl_image.size);
			debug("CRC Result : Expected 0x%08x "
				"Calculated 0x%08x\n",
				hanglory_spl_image.crc, calculated_crc);
			hang();
		} else
			debug("OK\n");
	} else {	/*  没有crc，同样挂起 */
		debug("no crc\n");
		hang();
	}

	/* 启动uboot */
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)hanglory_spl_image.entry_point;

	debug("image entry point: 0x%lx\n", hanglory_spl_image.entry_point);
	image_entry();
}

/* 备份启动 */
void hanglory_spl_boot(void)
{
	/* 加载备份uboot，提取头部信息到hanglory_spl_image */
	hanglory_spl_spi_load_image();
	/* 校验和启动uboot */
	hanglory_spl_crc_check_and_boot();	

}

