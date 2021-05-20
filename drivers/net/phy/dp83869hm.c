#include <config.h>
#include <common.h>
#include <dp83869hm.h>
#include <phy.h>


/* DP83869HM PHY Registers */
#define DP83869_DEVADDR		0xA
#define DP83869_PHY_ID		0x2000a0f1

#define DP83869_GIGABIT_SUPPORT 0xF
#define MII_DP83869_PHYCTRL	0x10
#define MII_DP83869_MICR	0x12
#define MII_DP83869_CFG2	0x14
#define MII_DP83869_BISCR	0x16
#define DP83869_CTRL		0x1f
#define DP83869_CFG4		0x0031
#define DP83869_RGMIICTL	0x0032
#define DP83869_STRAP_STS1	0x006E
#define DP83869_RGMIIDCTL	0x0086
#define DP83869_IO_MUX_CFG	0x0170
#define DP83869_OP_MODE_DECODE  0x1DF
#define DP83869_FX_CTRL     0xC00
#define DP83869_FX_STS      0xC01
#define DP83869_FX_PHYID1   0xC02
#define DP83869_FX_PHYID2   0xC03
#define DP83869_FX_ANADV    0xC04
#define DP83869_FX_LPABL    0xC05
#define DP83869_FX_ANEXP    0xC06
#define DP83869_FX_LOCNP    0xC07
#define DP83869_FX_LPNP     0xC08



#define MII_DP83869HM_EXTENDED_CTRL	0x0b
#define MII_DP83869HM_EXTENDED_DATAW	0x0c
#define MII_DP83869HM_EXTENDED_DATAR	0x0d
#define MII_DP83869HM_PHY_CTL		0x1f
#define MIIM_DP83869HM_PHYCTL_1000	(1 << 6)
#define MIIM_DP83869HM_PHYCTL_100		(1 << 5)
#define MIIM_DP83869HM_PHYCTL_10		(1 << 4)
#define MIIM_DP83869HM_PHYCTL_DUPLEX	(1 << 3)

#define CTRL1000_PREFER_MASTER		(1 << 10)
#define CTRL1000_CONFIG_MASTER		(1 << 11)
#define CTRL1000_MANUAL_CONFIG		(1 << 12)

#define BIT(x)			(1 << (x))

#define DP83869_SW_RESET	BIT(15)
#define DP83869_SW_RESTART	BIT(14)

/* MICR Interrupt bits */
#define MII_DP83869_MICR_AN_ERR_INT_EN		BIT(15)
#define MII_DP83869_MICR_SPEED_CHNG_INT_EN	BIT(14)
#define MII_DP83869_MICR_DUP_MODE_CHNG_INT_EN	BIT(13)
#define MII_DP83869_MICR_PAGE_RXD_INT_EN	BIT(12)
#define MII_DP83869_MICR_AUTONEG_COMP_INT_EN	BIT(11)
#define MII_DP83869_MICR_LINK_STS_CHNG_INT_EN	BIT(10)
#define MII_DP83869_MICR_FALSE_CARRIER_INT_EN	BIT(8)
#define MII_DP83869_MICR_SLEEP_MODE_CHNG_INT_EN	BIT(4)
#define MII_DP83869_MICR_WOL_INT_EN		BIT(3)
#define MII_DP83869_MICR_XGMII_ERR_INT_EN	BIT(2)
#define MII_DP83869_MICR_POL_CHNG_INT_EN	BIT(1)
#define MII_DP83869_MICR_JABBER_INT_EN		BIT(0)

/* RGMIICTL bits */
#define DP83869_RGMII_TX_CLK_DELAY_EN		BIT(1)
#define DP83869_RGMII_RX_CLK_DELAY_EN		BIT(0)

/* STRAP_STS1 bits */
#define DP83869_STRAP_STS1_RESERVED		BIT(11)

/* PHY CTRL bits */
#define DP83869_PHYCR_FIFO_DEPTH_SHIFT		14
#define DP83869_PHYCR_RESERVED_MASK	BIT(11)
#define DP83869_MDI_CROSSOVER		5
#define DP83869_MDI_CROSSOVER_AUTO	2
#define DP83869_MDI_CROSSOVER_MDIX	2
#define DP83869_PHYCTRL_SGMIIEN			0x0800
#define DP83869_PHYCTRL_RXFIFO_SHIFT	12
#define DP83869_PHYCTRL_TXFIFO_SHIFT	14

/* RGMIIDCTL bits */
#define DP83869_RGMII_TX_CLK_DELAY_SHIFT	4

/* CFG2 bits */
#define MII_DP83869_CFG2_SPEEDOPT_10EN		0x0040
#define MII_DP83869_CFG2_SGMII_AUTONEGEN	0x0080
#define MII_DP83869_CFG2_SPEEDOPT_ENH		0x0100
#define MII_DP83869_CFG2_SPEEDOPT_CNT		0x0800
#define MII_DP83869_CFG2_SPEEDOPT_INTLOW	0x2000
#define MII_DP83869_CFG2_MASK			0x003F

/* User setting - can be taken from DTS */
#define DEFAULT_RX_ID_DELAY	DP83869_RGMIIDCTL_2_25_NS
#define DEFAULT_TX_ID_DELAY	DP83869_RGMIIDCTL_2_75_NS
#define DEFAULT_FIFO_DEPTH	DP83869_PHYCR_FIFO_DEPTH_4_B_NIB

/* IO_MUX_CFG bits */
#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_CTRL	0x1f

#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_MAX	0x0
#define DP83869_IO_MUX_CFG_IO_IMPEDANCE_MIN	0x1f
#define DP83869_IO_MUX_CFG_CLK_O_SEL_SHIFT	8
#define DP83869_IO_MUX_CFG_CLK_O_SEL_MASK	GENMASK(0x1f, DP83869_IO_MUX_CFG_CLK_O_SEL_SHIFT)

/* CFG4 bits */
#define DP83869_CFG4_PORT_MIRROR_EN		BIT(0)


/* PHY CTRL bits */
#define DP83869_PHYCR_FIFO_DEPTH_3_B_NIB	0x00
#define DP83869_PHYCR_FIFO_DEPTH_4_B_NIB	0x01
#define DP83869_PHYCR_FIFO_DEPTH_6_B_NIB	0x02
#define DP83869_PHYCR_FIFO_DEPTH_8_B_NIB	0x03

/* RGMIIDCTL internal delay for rx and tx */
#define DP83869_RGMIIDCTL_250_PS	0x0
#define DP83869_RGMIIDCTL_500_PS	0x1
#define DP83869_RGMIIDCTL_750_PS	0x2
#define DP83869_RGMIIDCTL_1_NS		0x3
#define DP83869_RGMIIDCTL_1_25_NS	0x4
#define DP83869_RGMIIDCTL_1_50_NS	0x5
#define DP83869_RGMIIDCTL_1_75_NS	0x6
#define DP83869_RGMIIDCTL_2_00_NS	0x7
#define DP83869_RGMIIDCTL_2_25_NS	0x8
#define DP83869_RGMIIDCTL_2_50_NS	0x9
#define DP83869_RGMIIDCTL_2_75_NS	0xa
#define DP83869_RGMIIDCTL_3_00_NS	0xb
#define DP83869_RGMIIDCTL_3_25_NS	0xc
#define DP83869_RGMIIDCTL_3_50_NS	0xd
#define DP83869_RGMIIDCTL_3_75_NS	0xe
#define DP83869_RGMIIDCTL_4_00_NS	0xf

/* IO_MUX_CFG - Clock output selection */
#define DP83869_CLK_O_SEL_CHN_A_RCLK		0x0
#define DP83869_CLK_O_SEL_CHN_B_RCLK		0x1
#define DP83869_CLK_O_SEL_CHN_C_RCLK		0x2
#define DP83869_CLK_O_SEL_CHN_D_RCLK		0x3
#define DP83869_CLK_O_SEL_CHN_A_RCLK_DIV5	0x4
#define DP83869_CLK_O_SEL_CHN_B_RCLK_DIV5	0x5
#define DP83869_CLK_O_SEL_CHN_C_RCLK_DIV5	0x6
#define DP83869_CLK_O_SEL_CHN_D_RCLK_DIV5	0x7
#define DP83869_CLK_O_SEL_CHN_A_TCLK		0x8
#define DP83869_CLK_O_SEL_CHN_B_TCLK		0x9
#define DP83869_CLK_O_SEL_CHN_C_TCLK		0xA
#define DP83869_CLK_O_SEL_CHN_D_TCLK		0xB
#define DP83869_CLK_O_SEL_REF_CLK		0xC


/* ti dp83869hm */
#if 0
static int dp83869hm_config(struct phy_device *phydev)
{
	unsigned ctrl1000 = 0;
	const unsigned master = CTRL1000_PREFER_MASTER | CTRL1000_CONFIG_MASTER | CTRL1000_MANUAL_CONFIG;
	unsigned features = phydev->drv->features;

	if (getenv("disable_giga"))
		features &= ~(SUPPORTED_1000baseT_Half | SUPPORTED_1000baseT_Full);
	/* force master mode for 1000BaseT due to chip errata */
	if (features & SUPPORTED_1000baseT_Half)
		ctrl1000 |= ADVERTISE_1000HALF | master;
	if (features & SUPPORTED_1000baseT_Full)
		ctrl1000 |= ADVERTISE_1000FULL | master;
	phydev->advertising = phydev->supported = features;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, ctrl1000);
	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);
	return 0;
}
#endif

static int dp83869hm_config(struct phy_device *phydev)
{
	unsigned int val, delay, cfg2;
	int ret, bs;
    debug("dp83869hm_config begin \n");
#if (CONFIG_EMAC_BASE == CONFIG_EMAC0_BASE)
    /* Restart the PHY.  */
	val = phy_read(phydev, DP83869_DEVADDR, DP83869_CTRL);
	phy_write(phydev, DP83869_DEVADDR, DP83869_CTRL, val | DP83869_SW_RESTART);//ok

    val = phy_read(phydev, DP83869_DEVADDR, DP83869_OP_MODE_DECODE);
    val &= (~(0x7 << 0));
    //val |= (0x1 << 6);
    val |= (0x1 << 0);
    phy_write(phydev, DP83869_DEVADDR, DP83869_OP_MODE_DECODE, val);
    val = 0xffffffff;
    val = phy_read(phydev, DP83869_DEVADDR, DP83869_OP_MODE_DECODE);
    printf("dp83869hm addr 0x%x after set is 0x%x \n", DP83869_OP_MODE_DECODE, val);

    val = 0x1140;
    phy_write(phydev, DP83869_DEVADDR, /*DP83869_FX_CTRL*/ MII_BMCR, val);
    //phy_write(phydev, DP83869_DEVADDR, /*DP83869_FX_CTRL*/ MII_BMCR, 0x140);

    if (0) {
		phy_write(phydev, DP83869_DEVADDR, MII_DP83869_PHYCTRL,
			(DP83869_MDI_CROSSOVER_AUTO << DP83869_MDI_CROSSOVER) |(DP83869_PHYCR_FIFO_DEPTH_4_B_NIB << DP83869_PHYCR_FIFO_DEPTH_SHIFT));//ok

		val = phy_read(phydev, DP83869_DEVADDR, MII_DP83869_PHYCTRL);//ok
		val &= ~DP83869_PHYCR_RESERVED_MASK;//ok
		phy_write(phydev, DP83869_DEVADDR, MII_DP83869_PHYCTRL, val);//ok
	} 

	if (0) {
		val = phy_read(phydev, DP83869_DEVADDR, DP83869_RGMIICTL);
        val |= (DP83869_RGMII_TX_CLK_DELAY_EN | DP83869_RGMII_RX_CLK_DELAY_EN);//ok
		phy_write(phydev, DP83869_DEVADDR, DP83869_RGMIICTL, val);//ok

		delay = (DP83869_RGMIIDCTL_2_25_NS | (DP83869_RGMIIDCTL_2_75_NS << DP83869_RGMII_TX_CLK_DELAY_SHIFT));
		phy_write(phydev, DP83869_DEVADDR, DP83869_RGMIIDCTL, delay);

        
	}
#endif

#if 0
    int index = 0;
    for(index = 0; index <= 0x4F; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }

    for(index = 0x6e; index <= 0x6e; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }

    for(index = 0x86; index <= 0x86; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }

    for(index = 0x134; index <= 0x135; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }
    
    for(index = 0x170; index <= 0x170; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }
    
    for(index = 0x180; index <= 0x1A6; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }

    for(index = 0x1DF; index <= 0x1E0; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }

    for(index = 0xC00; index <= 0xC19; index++)
    {
        val = 0xffff;
        val = phy_read(phydev, DP83869_DEVADDR, index);
        printf("RegTest addr 0x%x is 0x%04x \n", index, val&0xFFFF);
    }
#endif
    debug("dp83869hm_config end \n");
	return 0;

err_out:
	return ret;
}

static struct phy_driver dp83869hm_driver = {
	.name = "TI dp83869hm",
    .uid  = DP83869_PHY_ID,
    .mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &dp83869hm_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_ti_init(void)
{
	phy_register(&dp83869hm_driver);
    debug("phy_ti_init over \n");
	return 0;
}
