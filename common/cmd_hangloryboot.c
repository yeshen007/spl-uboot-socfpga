/***********************************************************************************************
 * 				hanglory double boot
 * 				author yezheng
 *               cmd_hangloryboot.c
 *********************************************************************************************/
#define DEBUG

#include <command.h>
#include <common.h>
#include <errno.h>
#include <spl.h>
#include <image.h>
#include <hanglory_config.h>


struct hanglory_image_info hanglory_uboot_image; 

/* 分析头部信息，提取到hanglory_uboot_image */
static void hanglory_uboot_parse_image_header(const struct image_header *header)
{
	u32 header_size = sizeof(struct image_header);

	if (image_get_magic(header) == IH_MAGIC) {
		if (hanglory_uboot_image.flags & SPL_COPY_PAYLOAD_ONLY) {
			/*
			 * On some system (e.g. powerpc), the load-address and
			 * entry-point is located at address 0. We can't load
			 * to 0-0x40. So skip header in this case.
			 */
			hanglory_uboot_image.load_addr = image_get_load(header);
			hanglory_uboot_image.entry_point = image_get_ep(header);
			hanglory_uboot_image.size = image_get_data_size(header);
		} else {
			printf("coming here hahahahahahaha\n");
			hanglory_uboot_image.entry_point = image_get_ep(header);		//除去头部后的地址
			/* Load including the header */
			hanglory_uboot_image.load_addr = hanglory_uboot_image.entry_point -
				header_size;					//头部地址
			hanglory_uboot_image.size = image_get_data_size(header) +
				header_size;					//镜像的长度（包括头部）
		}
		hanglory_uboot_image.os = image_get_os(header);
		hanglory_uboot_image.name = image_get_name(header);
		hanglory_uboot_image.crc = image_get_dcrc(header);
		hanglory_uboot_image.crc_size = image_get_data_size(header);		//crc的数据长度，不包括头部
		debug("spl: payload image: %s load addr: 0x%x size: %d\n",
			hanglory_uboot_image.name, hanglory_uboot_image.load_addr, hanglory_uboot_image.size);
	} else {
		debug("mkimage signature not found - ih_magic = %x\n",
			header->ih_magic);
		hanglory_uboot_image.size = 0;
		hanglory_uboot_image.entry_point = 0;
		hanglory_uboot_image.load_addr = 0;
		hanglory_uboot_image.os = 0;
		hanglory_uboot_image.name = "LINUX";
		hanglory_uboot_image.crc_size = 0;
	}
}



static int hanglory_uboot_crc_check(void)
{
	/* crc校验 */	
	u32 calculated_crc;
	if (hanglory_uboot_image.crc_size != 0) {
		debug("Verifying Checksum ... ");
		calculated_crc = crc32_wd(0,
			(unsigned char *)hanglory_uboot_image.entry_point,
			hanglory_uboot_image.crc_size, CHUNKSZ_CRC32);
		if (calculated_crc != hanglory_uboot_image.crc) {		/* 校验没通过，返回错误码 */
			puts("Bad image with mismatched CRC\n");
			debug("CRC calculate from 0x%08x "
				"with length 0x%08x\n",
				hanglory_uboot_image.entry_point, hanglory_uboot_image.crc_size);
			debug("CRC Result : Expected 0x%08x "
				"Calculated 0x%08x\n",
				hanglory_uboot_image.crc, calculated_crc);
			return -EAGAIN;
		} else {	
			debug("OK\n");
			return 0;
		}
	} else {
		debug("wrong image header\n");	
		return -EAGAIN;
	}

}

static int do_hangloryboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s;

	/* 提取uImage头部 */
	struct image_header *header;
	header = (struct image_header *)(CONFIG_KERNEL_LOAD_ADDR);
	hanglory_uboot_parse_image_header(header);

	/* 对uIamge校验 */
	int ret;
	ret = hanglory_uboot_crc_check();
	
	/* 如果通过则执行oriboot命令，没通则执行secondboot命令 */
	if (ret == 0) {
        puts("oriboot.....\n");
		s = getenv("oriboot");
		run_command_list(s, -1, 0);
	} else {
        puts("secondboot.....\n");
		s =getenv("secondboot");
		run_command_list(s, -1, 0);
	}

	return 0;
}

U_BOOT_CMD(
	hangloryboot,	CONFIG_SYS_MAXARGS,	1,	do_hangloryboot,
	"short help info",	//help不带参数显示的帮助信息
	"long help info"	//help hanglory显示的帮助信息
);
