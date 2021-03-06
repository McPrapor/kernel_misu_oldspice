#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#endif

#include <cust_gpio_usage.h>

//#define FPGA_EARLY_PORTING

#ifndef FPGA_EARLY_PORTING
#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>  
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <cust_i2c.h>
#include <mach/upmu_hw.h>



/***************************************************************************** 
 * Define
 *****************************************************************************/
#define TPS_I2C_BUSNUM  1//I2C_I2C_LCD_BIAS_CHANNEL//for I2C channel 0//sophia
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x3E
/***************************************************************************** 
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;

/***************************************************************************** 
 * Function Prototype
 *****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);

#ifndef BUILD_LK
extern kal_uint16 pmic_set_register_value(PMU_FLAGS_LIST_ENUM flagname,kal_uint32 val);
#endif
/***************************************************************************** 
 * Data Structure
 *****************************************************************************/

struct tps65132_dev	{	
    struct i2c_client	*client;

};

static const struct i2c_device_id tps65132_id[] = {
    { I2C_ID_NAME, 0 },
    { }
};

//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
//static struct i2c_client_address_data addr_data = { .forces = forces,};
//#endif
static struct i2c_driver tps65132_iic_driver = {
    .id_table	= tps65132_id,
    .probe		= tps65132_probe,
    .remove		= tps65132_remove,
    //.detect		= mt6605_detect,
    .driver		= {
        .owner	= THIS_MODULE,
        .name	= "tps65132",
    },

};
/***************************************************************************** 
 * Extern Area
 *****************************************************************************/ 



/***************************************************************************** 
 * Function
 *****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{  
    printk( "*********hx8394f tps65132_iic_probe\n");
    printk("*********hx8394f TPS: info==>name=%s addr=0x%x\n",client->name,client->addr);
    tps65132_i2c_client  = client;		
    return 0;      
}


static int tps65132_remove(struct i2c_client *client)
{  	
    printk( "*********hx8394f tps65132_remove\n");
    tps65132_i2c_client = NULL;
    i2c_unregister_device(client);
    return 0;
}


int TPS65132_write_bytes(unsigned char addr, unsigned char value)
{	
    int ret = 0;
    struct i2c_client *client = tps65132_i2c_client;
    char write_data[2]={0};	
    write_data[0]= addr;
    write_data[1] = value;
    ret=i2c_master_send(client, write_data, 2);
    if(ret<0)
        printk("*********hx8394f tps65132 write data fail !!\n");	
    return ret ;
}
EXPORT_SYMBOL_GPL(TPS65132_write_bytes);



/*
 * module load/unload record keeping
 */

static int __init tps65132_iic_init(void)
{

    printk( "*********hx8394f tps65132_iic_init\n");
    i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
    printk( "*********hx8394f tps65132_iic_init2\n");
    i2c_add_driver(&tps65132_iic_driver);
    printk( "*********hx8394f tps65132_iic_init success\n");	
    return 0;
}

static void __exit tps65132_iic_exit(void)
{
    printk( "*********hx8394f tps65132_iic_exit\n");
    i2c_del_driver(&tps65132_iic_driver);  
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL"); 
#endif

#else

#ifdef BUILD_LK
//extern int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value);
#endif
extern int TPS65132_write_bytes(unsigned char addr, unsigned char value);

#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define GPIO_65132_ENP GPIO_LCD_BIAS_ENP_PIN //sophiarui
#define GPIO_65132_ENN GPIO_LCD_BIAS_ENN_PIN //sophiarui


#define REGFLAG_DELAY             							0XFEFF
#define REGFLAG_END_OF_TABLE      							0xFFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID1 0x83
#define LCM_ID2 0x94
#define LCM_ID3_0F 0x0f
#define LCM_ID3_F1 0xf1
#define LCM_ID3_F2 0xf2
#define LCM_ID3_F3 0xf3
#define LCM_ID3_F4 0xf4
#define LCM_ID3_F5 0xf5

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

//#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
static int SET_RESET_PIN(int v){
 mt_set_gpio_mode(GPIO146, GPIO_MODE_00);
 mt_set_gpio_dir(GPIO146, GPIO_DIR_OUT);
 mt_set_gpio_out(GPIO146, v);
}
#define SET_GPIO_OUT(gpio_num,val)    						(lcm_util.set_gpio_out((gpio_num),(val)))


#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


#define _LCM_DEBUG_

#ifdef BUILD_LK
#define printk printf
#endif

#ifdef _LCM_DEBUG_
#define lcm_debug(fmt, args...) printk(fmt, ##args)
#else
#define lcm_debug(fmt, args...) do { } while (0)
#endif

#ifdef _LCM_INFO_
#define lcm_info(fmt, args...) printk(fmt, ##args)
#else
#define lcm_info(fmt, args...) do { } while (0)
#endif
#define lcm_err(fmt, args...) printk(fmt, ##args)

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

#if 0
static struct LCM_setting_table lcm_initialization_setting[] = {

    /*
Note :

Data ID will depends on the following rule.

count of parameters > 1	=> Data ID = 0x39
count of parameters = 1	=> Data ID = 0x15
count of parameters = 0	=> Data ID = 0x05

Structure Format :

{DCS command, count of parameters, {parameter list}}
{REGFLAG_DELAY, milliseconds of time, {}},

...

Setting ending by predefined flag

{REGFLAG_END_OF_TABLE, 0x00, {}}
     */


//must use 0x39 for init setting for all register.

{0xb9, 3, {0xff,0x83,0x94}},        //EXTC=1
{REGFLAG_DELAY, 5, {}},   

{0xba, 6, {0x33,0x83,0xa8,0x65,0xb2,0x09}},
{REGFLAG_DELAY, 5, {}},  

    //VGH=13V /VGL=-10V
{0xb1, 15, {0x6c,0x11,0x0e,0x37,0x04,0x11,0xf1,0x80,0xdf,0x94,0x23,0x80,0xc0,0xd2,0x18}},
{REGFLAG_DELAY, 5, {}},   

{0xb2, 11, {0x00,0x64,0x0e,0x0d,0x32,0x23,0x08,0x08,0x1c,0x4d,0x00}},
{REGFLAG_DELAY, 5, {}},  

{0xb4, 12, {0x00,0xff,0x03,0x46,0x03,0x46,0x03,0x46,0x01,0x6a,0x01,0x6a}},
{REGFLAG_DELAY, 5, {}},  

{0xbf,3,{0x41,0x0e,0x01}},
{REGFLAG_DELAY, 5, {}},  

{0xd3, 37, {0x00,0x07,0x00,0x00,0x00,0x10,0x00,0x32,0x10,0x05,
    0x00,0x05,0x32,0x10,0x00,0x00,0x00,0x32,0x10,0x00,
    0x00,0x00,0x36,0x03,0x09,0x09,0x37,0x00,0x00,0x37,
    0x00,0x00,0x00,0x00,0x0a,0x00,0x01}},
{REGFLAG_DELAY, 5, {}},

{0xd5, 44, {0x02,0x03,0x00,0x01,0x06,0x07,0x04,0x05,0x20,0x21,
    0x22,0x23,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
    0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
    0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x24,0x25,
    0x18,0x18,0x19,0x19}},
{REGFLAG_DELAY, 5, {}}, 

{0xd6, 44, {0x05,0x04,0x07,0x06,0x01,0x00,0x03,0x02,0x23,0x22,
    0x21,0x20,0x18,0x18,0x18,0x18,0x18,0x18,0x58,0x58,
    0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
    0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x25,0x24,
    0x19,0x19,0x18,0x18}},
{REGFLAG_DELAY, 5, {}},  


{0xe0, 42, {0x02,0x0a,0x0b,0x24,0x29,0x33,0x1a,0x3c,0x08,0x0b,
    0x0d,0x17,0x0F,0x12,0x15,0x13,0x14,0x08,0x12,0x16,
    0x19,0x02,0x0a,0x0b,0x24,0x29,0x33,0x1a,0x3c,0x08,
    0x0b,0x0d,0x17,0x0f,0x12,0x15,0x13,0x14,0x08,0x12,
    0x16,0x19}},
{REGFLAG_DELAY, 5, {}},  

{0xcc, 1, {0x09}},
{REGFLAG_DELAY, 5, {}},  

{0xc7, 4, {0x00,0xc0,0x40,0xc0}},
{REGFLAG_DELAY, 5, {}},  

{0xb6, 2, {0x69,0x7f}},
{REGFLAG_DELAY, 5, {}},  

{0xc0, 2, {0x30,0x14}},
{REGFLAG_DELAY, 5, {}},   

{0xbc, 1, {0x07}},
{REGFLAG_DELAY, 5, {}},  

{0x51, 1, {0xff}},
{0x53, 1, {0x24}},
{0x55, 1, {0x00}},
{REGFLAG_DELAY, 20, {}},  

{0x35, 1, {0x00}},
{REGFLAG_DELAY, 10, {}},  

{0xb6, 1, {0x62}},
{REGFLAG_DELAY, 10, {}},
{REGFLAG_END_OF_TABLE, 0x00, {}} 


// Note
// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
};
#endif

static void init_lcm_registers(void)
{
		unsigned int data_array[16];

		data_array[0] = 0x00043902;
		data_array[1] = 0x9483ffb9;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00073902;
		data_array[1] = 0x680363ba;  //62
		data_array[2] = 0x00c0b26b;
		dsi_set_cmdq(data_array, 3, 1);
		//MDELAY(2);

		data_array[0] = 0x000b3902;
		data_array[1] = 0x721250b1;
		data_array[2] = 0xb1543109;
		data_array[3] = 0x00476471;
		dsi_set_cmdq(data_array, 4, 1);
		//MDELAY(2);
//esd fix
		data_array[0] = 0x000d3902;
		data_array[1] = 0x648000b2;
		data_array[2] = 0x002f0806;
		data_array[3] = 0xc0000000;
		data_array[4] = 0x0000001b;
		dsi_set_cmdq(data_array, 5, 1);
		//MDELAY(2);
//
		data_array[0] = 0x00163902;
		data_array[1] = 0x0d7674b4;
		data_array[2] = 0x015b5a5c;
		data_array[3] = 0x00357e05;
		data_array[4] = 0x0d76743f;
		data_array[5] = 0x015b5a5c;
		data_array[6] = 0x00007e05;
		dsi_set_cmdq(data_array, 7, 1);//
		//MDELAY(2);
#if 0
		data_array[0] = 0x003b3902;
		data_array[1] = 0x100500e0;
		data_array[2] = 0x211d1917;
		data_array[3] = 0x69554320;
		data_array[4] = 0x968c786b;
		data_array[5] = 0xabafab9c;
		data_array[6] = 0x6163c8ba;
		data_array[7] = 0x746f6c66;
		data_array[8] = 0x05007f7f;
		data_array[9] = 0x1d19160f;
		data_array[10] = 0x55432021;
		data_array[11] = 0x8c786b69;
		data_array[12] = 0xafab9c96;
		data_array[13] = 0x63c8baab;
		data_array[14] = 0x6f6c6661;
		data_array[15] = 0x007f7f74;
		dsi_set_cmdq(data_array, 16, 1);
		MDELAY(2);
#endif
		data_array[0] = 0x00223902;
		data_array[1] = 0x060000d3;
		data_array[2] = 0x0a074006;
		data_array[3] = 0x0110320a;
		data_array[4] = 0x15520100;
		data_array[5] = 0x32070507;
		data_array[6] = 0x00000010;
		data_array[7] = 0x05054467;
		data_array[8] = 0x270e0e37;
		data_array[9] = 0x00004006;
		dsi_set_cmdq(data_array, 10, 1);
		//MDELAY(2);

		data_array[0] = 0x002d3902;
		data_array[1] = 0x002120d5;
		data_array[2] = 0x04030201;
		data_array[3] = 0x18070605;
		data_array[4] = 0x18181818;
		data_array[5] = 0x18181818;
		data_array[6] = 0x18181818;
		data_array[7] = 0x18181818;
		data_array[8] = 0x18181818;
		data_array[9] = 0x18181818;
		data_array[10] = 0x19181818;
		data_array[11] = 0x24181819;
		data_array[12] = 0x00000025;
		dsi_set_cmdq(data_array, 13, 1);
		//MDELAY(2);

		data_array[0] = 0x002d3902;
		data_array[1] = 0x072524d6;
		data_array[2] = 0x03040506;
		data_array[3] = 0x18000102;
		data_array[4] = 0x18181818;
		data_array[5] = 0x58181818;
		data_array[6] = 0x18181858;
		data_array[7] = 0x18181818;
		data_array[8] = 0x18181818;
		data_array[9] = 0x18181818;
		data_array[10] = 0x18181818;
		data_array[11] = 0x20191918;
		data_array[12] = 0x00000021;
		dsi_set_cmdq(data_array, 13, 1);
		//MDELAY(2);
#if 0
		data_array[0] = 0x00033902;
		data_array[1] = 0x006d6db6;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);
#endif

		data_array[0] = 0x003b3902;
		data_array[1] = 0x130800e0;
		data_array[2] = 0x25221e1a;
		data_array[3] = 0x73604c25;
		data_array[4] = 0x9c937f74;
		data_array[5] = 0xadb1afa1;
		data_array[6] = 0x6062c7b9;
		data_array[7] = 0x6c666563;
		data_array[8] = 0x08007f70;
		data_array[9] = 0x221e1912;
		data_array[10] = 0x604c2525;
		data_array[11] = 0x94807473;
		data_array[12] = 0xb1b0a29c;
		data_array[13] = 0x61c7b9ad;
		data_array[14] = 0x6765625f;
		data_array[15] = 0x007f746d;
		dsi_set_cmdq(data_array, 16, 1);
		//MDELAY(2);


		data_array[0] = 0x00023902;
		data_array[1] = 0x00000bcc;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00033902;
		data_array[1] = 0x00731fc0;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000002d4;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000001bd;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000060b1;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000000bd;
		dsi_set_cmdq(data_array, 2, 1);
		//MDELAY(2);

                data_array[0] = 0x00083902;
		data_array[1] = 0x508140bf;
                data_array[2] = 0x01fc1a00;
		dsi_set_cmdq(data_array, 3, 1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000ff51;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(5);

		data_array[0] = 0x00023902;
		data_array[1] = 0x00000055;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(5);

		data_array[0] = 0x00023902;
		data_array[1] = 0x00002453;
		dsi_set_cmdq(data_array, 2, 1);


		data_array[0] = 0x00023902;
		data_array[1] = 0x000001bd;
		dsi_set_cmdq(data_array, 2, 1);
		
                data_array[0] = 0x00043902;
		data_array[1] = 0x134464b1;
		dsi_set_cmdq(data_array, 2, 1);

                data_array[0] = 0x00023902;
		data_array[1] = 0x000000bd;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0] = 0x00110500;
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(120);
	
		data_array[0] = 0x00290500;
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(10);

#if 0

		data_array[0] = 0x00043902;
		data_array[1] = 0x9483ffb9;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00033902;
		data_array[1] = 0x008373ba;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00103902;
		data_array[1] = 0x15556cb1;
		data_array[2] = 0xf1110411;
		data_array[3] = 0x2395e880;
		data_array[4] = 0x58d2c080;
		dsi_set_cmdq(data_array, 5, 1);
		MDELAY(2);

		data_array[0] = 0x000c3902;
		data_array[1] = 0x106400b2;
		data_array[2] = 0x081c3207;
		data_array[3] = 0x004d1c08;
		dsi_set_cmdq(data_array, 4, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000007bc;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x000d3902;
		data_array[1] = 0x03ff00b4;
		data_array[2] = 0x035c035c;
		data_array[3] = 0x0170015c;
		data_array[4] = 0x00000070;
		dsi_set_cmdq(data_array, 5, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000055d2;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x001f3902;
		data_array[1] = 0x000600d3;
		data_array[2] = 0x00081a01;
		data_array[3] = 0x00071032;
		data_array[4] = 0x0f155407;
		data_array[5] = 0x12020405;
		data_array[6] = 0x33070510;
		data_array[7] = 0x370b0b33;
		data_array[8] = 0x00070710;
		dsi_set_cmdq(data_array, 9, 1);
		MDELAY(2);

		data_array[0] = 0x002d3902;
		data_array[1] = 0x181919d5;
		data_array[2] = 0x1a1b1b18;
		data_array[3] = 0x0605041a;
		data_array[4] = 0x02010007;
		data_array[5] = 0x18212003;
		data_array[6] = 0x18232218;
		data_array[7] = 0x18181818;
		data_array[8] = 0x18181818;
		data_array[9] = 0x18181818;
		data_array[10] = 0x18181818;
		data_array[11] = 0x18181818;
		data_array[12] = 0x00000018;
		dsi_set_cmdq(data_array, 13, 1);
		MDELAY(2);

		data_array[0] = 0x002d3902;
		data_array[1] = 0x191818d6;
		data_array[2] = 0x1a1b1b19;
		data_array[3] = 0x0102031a;
		data_array[4] = 0x05060700;
		data_array[5] = 0x18222304;
		data_array[6] = 0x18202118;
		data_array[7] = 0x18181818;
		data_array[8] = 0x18181818;
		data_array[9] = 0x18181818;
		data_array[10] = 0x18181818;
		data_array[11] = 0x18181818;
		data_array[12] = 0x00000018;
		dsi_set_cmdq(data_array, 13, 1);
		MDELAY(2);

		data_array[0] = 0x002b3902;
		data_array[1] = 0x090300e0;
		data_array[2] = 0x1c3f2723;
		data_array[3] = 0x0b0a063a;
		data_array[4] = 0x12100d17;
		data_array[5] = 0x11071311;
		data_array[6] = 0x03001814;
		data_array[7] = 0x3f292309;
		data_array[8] = 0x0a063a1c;
		data_array[9] = 0x110e170b;
		data_array[10] = 0x07131112;
		data_array[11] = 0x00181312;
		dsi_set_cmdq(data_array, 12, 1);
		MDELAY(2);

		data_array[0] = 0x00033902;
		data_array[1] = 0x001430c0;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00053902;
		data_array[1] = 0x00c000c7;
		data_array[2] = 0x000000c0;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000009cc;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x000087df;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000ff51;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00083902;
		data_array[1] = 0x14001fc9;
		data_array[2] = 0x001e811e;
		dsi_set_cmdq(data_array, 3, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x00000055;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(2);

		data_array[0] = 0x00023902;
		data_array[1] = 0x0000005e;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0] = 0x000a3902;
		data_array[1] = 0x2f3040ca;
		data_array[2] = 0x23262d2e;
		data_array[3] = 0x00002021;
		dsi_set_cmdq(data_array, 4, 1);

		data_array[0] = 0x00233902;
		data_array[1] = 0x000000ce;
		data_array[2] = 0x10101010;
		data_array[3] = 0x20202020;
		data_array[4] = 0x30202020;
		data_array[5] = 0x30303030;
		data_array[6] = 0x30303030;
		data_array[7] = 0x30303030;
		data_array[8] = 0x30303030;
		data_array[9] = 0x00003030;
		dsi_set_cmdq(data_array, 10, 1);

		data_array[0] = 0x00023902;
		data_array[1] = 0x00002453;
		dsi_set_cmdq(data_array, 2, 1);

		data_array[0] = 0x00110500;
		dsi_set_cmdq(data_array, 1, 1);
		MDELAY(200);

		data_array[0] = 0x00013902;
		data_array[1] = 0x00000029;
		dsi_set_cmdq(data_array, 2, 1);
		MDELAY(10);
#endif
	 
}

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 150, {}},

    // Display ON
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
    // Display off sequence
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
    {0x10, 0, {0x00}},
    {REGFLAG_DELAY, 150, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;
    lcm_debug("%s %d\n", __func__,__LINE__);

    for(i = 0; i < count; i++) {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    lcm_debug("%s %d\n", __func__,__LINE__);
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    lcm_debug("%s %d\n", __func__,__LINE__);
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
    params->dsi.mode   = CMD_MODE;
#else
    params->dsi.mode   = BURST_VDO_MODE;
#endif

   // params->dsi.esd_check_enable = 0;

    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 2;//4;//2;
    params->dsi.vertical_backporch					= 14;//16;//14;
    params->dsi.vertical_frontporch					= 16;//16;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 40;//6;//2;
    params->dsi.horizontal_backporch				= 42;//44;//44;//42;
    params->dsi.horizontal_frontporch				= 44;//44;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;

    params->dsi.lcm_esd_check_table[0].cmd			= 0x09;
    params->dsi.lcm_esd_check_table[0].count		= 3;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
    params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
    params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;

    params->dsi.lcm_esd_check_table[1].cmd			= 0xd9;
    params->dsi.lcm_esd_check_table[1].count		= 1;
    params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;

    params->dsi.lcm_esd_check_table[2].cmd			= 0x45;
    params->dsi.lcm_esd_check_table[2].count		= 2;
    params->dsi.lcm_esd_check_table[2].para_list[0] = 0x05;
    params->dsi.lcm_esd_check_table[2].para_list[1] = 0x1e;

    params->dsi.PLL_CLOCK = 198;//dsi clock customization: should config clock value directly

    params->pwm_min = 7;
    params->pwm_default = 87;
    params->pwm_max = 255;
}

#ifdef BUILD_LK
int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
#endif
static unsigned int lcm_compare_id(void)
{
#if 1
    unsigned int id1 = 0xFF;
    unsigned int id2 = 0xFF;
    unsigned int id3 = 0xFF;
    unsigned char buffer[3];
    unsigned int data_array[16];
    int lcm_adc = 0, data[4] = {0,0,0,0};		

    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//sophiarui
    mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
    MDELAY(15);
    mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//sophiarui
    mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
    MDELAY(7);

    lcm_debug("hx8394f_hd720_dsi_vdo_truly %s %d\n", __func__,__LINE__);
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(10);

    //set EXTC
    data_array[0]=0x00043902;
    data_array[1]=0x9483ffb9;
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(1);

    //set maximum return size
    data_array[0] = 0x00033700;
    dsi_set_cmdq(data_array, 1, 1);

    //read id R04h
    data_array[0] = 0x00040600;
    dsi_set_cmdq(data_array, 1, 1);

    read_reg_v2(0x04, buffer, 3);

    id1 = buffer[0]; //we only need ID
    id2 = buffer[1];

    MDELAY(5);
    read_reg_v2(0xDC, buffer, 1);
    id3 = buffer[0];

#if defined(BUILD_LK)
    printf("%s,[LK] xutao id1 = 0x%08x\n", __func__, id1);
    printf("%s,[LK] xutao id2 = 0x%08x\n", __func__, id2);
    printf("%s,[LK] xutao id3 = 0x%08x\n", __func__, id3);
#else
    printk("%s, hx8394f_hd720_dsi_vdoid1 id1 = 0x%08x\n", __func__, id1);
    printk("%s, hx8394f_hd720_dsi_vdoid1 id2 = 0x%08x\n", __func__, id2);
#endif
    return ((LCM_ID1== id1) && (LCM_ID2== id2) && (LCM_ID3_0F== id3||
                 LCM_ID3_F1==id3||LCM_ID3_F2==id3||LCM_ID3_F3==id3||LCM_ID3_F4==id3
                 ||LCM_ID3_F5==id3))?1:0;
    #endif
}

#ifndef FPGA_EARLY_PORTING

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h> 
#include <platform/mt_pmic.h>

#define TPS65132_SLAVE_ADDR_WRITE  0x7C  
static struct mt_i2c_t TPS65132_I2C;

int TPS65132_WRITE_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    TPS65132_I2C.id = I2C1;
    /* Since i2c will left shift 1 bit, we need to set FAN5405 I2C address to >>1 */
    TPS65132_I2C.addr = (TPS65132_SLAVE_ADDR_WRITE >> 1);
    TPS65132_I2C.mode = ST_MODE;
    TPS65132_I2C.speed = 100;
    len = 2;

    ret_code = i2c_write(&TPS65132_I2C, write_data, len);
    printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}
EXPORT_SYMBOL_GPL(tps65132_write_byte);
#else

//	extern int mt8193_i2c_write(u16 addr, u32 data);
//	extern int mt8193_i2c_read(u16 addr, u32 *data);

//	#define TPS65132_write_byte(add, data)  mt8193_i2c_write(add, data)
//	#define TPS65132_read_byte(add)  mt8193_i2c_read(add)


#endif
#endif

static void lcm_init(void)
{
    unsigned char cmd = 0x0;
    unsigned char data = 0xFF;
    int ret=0;
    cmd=0x00;
    data=0x0E;

    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);//sophiarui
    mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ONE);
    MDELAY(15);
    mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);//sophiarui
    mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ONE);
    MDELAY(7);
#ifdef BUILD_LK
printf("%s: xutao: ret_code: %d\n", __func__);

ret=TPS65132_WRITE_byte(cmd,data);
if(ret)    	
printf("[LK]TM050-----tps6132----cmd=%0x--i2c write error----\n",cmd);    	
else
printf("[LK]TM050----tps6132----cmd=%0x--i2c write success----\n",cmd);  
		
#else
ret=TPS65132_write_bytes(cmd,data);
if(ret<0)
printk("[KERNEL]TM050-----tps6132---cmd=%0x-- i2c write error-----\n",cmd);
else
printk("[KERNEL]TM050-----tps6132---cmd=%0x-- i2c write success-----\n",cmd);
#endif

cmd=0x01;
data=0x0E;
#ifdef BUILD_LK
ret=TPS65132_WRITE_byte(cmd,data);
if(ret)    	
printf("[LK]TM050-----tps6132----cmd=%0x--i2c write error----\n",cmd);    	
else
printf("[LK]TM050----tps6132----cmd=%0x--i2c write success----\n",cmd); 

TPS65132_WRITE_byte(0x03, 0x33);//VSP/VSN FLOATING //0x03--- 0x40 ---tablet
TPS65132_WRITE_byte(0xFF, 0x80);  
#else
ret=TPS65132_write_bytes(cmd,data);
if(ret<0)
printk("[KERNEL]TM050-----tps6132---cmd=%0x-- i2c write error-----\n",cmd);
else
printk("[KERNEL]TM050-----tps6132---cmd=%0x-- i2c write success-----\n",cmd);
TPS65132_write_bytes(0x03, 0x33);
TPS65132_write_bytes(0xFF, 0x80);
#endif

    MDELAY(20);
    lcm_debug("%s %d\n", __func__,__LINE__);
    SET_RESET_PIN(1);
    MDELAY(2);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(60);

    init_lcm_registers();
    //push_table(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}
static void lcm_init_power(void)
{
#ifndef CONFIG_FPGA_EARLY_PORTING
#ifdef BUILD_LK
	printf("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,3);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	MDELAY(50);
	printf("%s, end\n", __func__);
#else
	printk("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,3);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	MDELAY(50);
	printk("%s, end\n", __func__);
#endif
#endif
}

static void lcm_suspend_power(void)
{
#ifndef CONFIG_FPGA_EARLY_PORTING
#ifdef BUILD_LK
	printf("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_EN,0);
	printf("%s, end\n", __func__);
#else
	printk("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_EN,0);
	printk("%s, end\n", __func__);
#endif
#endif
}

static void lcm_resume_power(void)
{
#ifndef CONFIG_FPGA_EARLY_PORTING
#ifdef BUILD_LK
	printf("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,3);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	MDELAY(50);
	printf("%s, end\n", __func__);
#else
	printk("%s, begin\n", __func__);
	pmic_set_register_value(PMIC_RG_VGP1_VOSEL,3);
	pmic_set_register_value(PMIC_RG_VGP1_EN,1);
	MDELAY(50);
	printk("%s, end\n", __func__);
#endif
#endif
}
static void lcm_suspend(void)
{
    lcm_debug("%s %d\n", __func__,__LINE__);
   


    push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

    SET_RESET_PIN(0);	
    MDELAY(15);	
    mt_set_gpio_mode(GPIO_65132_ENN, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENN, GPIO_OUT_ZERO);
    MDELAY(7);
    mt_set_gpio_mode(GPIO_65132_ENP, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_65132_ENP, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_65132_ENP, GPIO_OUT_ZERO);
    MDELAY(7);
 
    //SET_GPIO_OUT(GPIO_LCM_PWR_EN,0);//Disable LCM Power
}

static void lcm_resume(void)
{
    lcm_debug("%s %d\n", __func__,__LINE__);
    //	SET_GPIO_OUT(GPIO_LCM_PWR_EN,1);  //Enable LCM Power
    lcm_init();
    //	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height)
{
    lcm_debug("%s %d\n", __func__,__LINE__);
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;

    unsigned char x0_MSB = ((x0>>8)&0xFF);
    unsigned char x0_LSB = (x0&0xFF);
    unsigned char x1_MSB = ((x1>>8)&0xFF);
    unsigned char x1_LSB = (x1&0xFF);
    unsigned char y0_MSB = ((y0>>8)&0xFF);
    unsigned char y0_LSB = (y0&0xFF);
    unsigned char y1_MSB = ((y1>>8)&0xFF);
    unsigned char y1_LSB = (y1&0xFF);

    unsigned int data_array[16];

    data_array[0]= 0x00053902;
    data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
    data_array[2]= (x1_LSB);
    data_array[3]= 0x00000000;
    data_array[4]= 0x00053902;
    data_array[5]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[6]= (y1_LSB);
    data_array[7]= 0x00000000;
    data_array[8]= 0x002c3909;

    dsi_set_cmdq(data_array, 9, 0);
}

static unsigned int lcm_esd_check(void)
{
    unsigned int ret=FALSE;
#ifndef BUILD_LK
    char  *buffer;
    int   array[4];

#if 1
    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }
#endif
    array[0] = 0x00013700;
    dsi_set_cmdq(array, 1, 1);

    read_reg_v2(0x0A, buffer, 1);
    printk(" esd buffer0 =%x\n", buffer[0]);
    printk("\n************hx8394f---esd_check----********\n");

#if 1
    if(buffer[0]==0x08)
    {
        ret=FALSE;
    }
    else
    {			 
        ret=TRUE;
    }
#endif
#endif

    return ret;

}

static unsigned int lcm_esd_recover(void)
{
    lcm_init();

#ifndef BUILD_LK
    printk("lcm_esd_recover hx8394f\n");
#endif
    return TRUE;
}

LCM_DRIVER hx8394f_hd720_dsi_vdo_truly_v36_drv = 
{
    .name	    = "hx8394f_hd720_dsi_vdo_truly_v36",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
        .init_power		= lcm_init_power,
    .resume_power = lcm_resume_power,
    .suspend_power = lcm_suspend_power,
#if (LCM_DSI_CMD_MODE)
    .set_backlight	= lcm_setbacklight,
    .update   		= lcm_update,
#endif
    .esd_check   	= lcm_esd_check,
    .esd_recover   = lcm_esd_recover,
};


