#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#define ADAPTER_NAME     "TOY_I2C_ADAPTER"
/*
** This function used to get the functionalities that are supported 
** by this bus driver.
*/
static u32 toy_func(struct i2c_adapter *adapter)
{
    return (I2C_FUNC_I2C             |
            I2C_FUNC_SMBUS_QUICK     |
            I2C_FUNC_SMBUS_BYTE      |
            I2C_FUNC_SMBUS_BYTE_DATA |
            I2C_FUNC_SMBUS_WORD_DATA |
            I2C_FUNC_SMBUS_BLOCK_DATA);
}
/*
** This function will be called whenever you call I2C read, wirte APIs like
** i2c_master_send(), i2c_master_recv() etc.
*/
static s32 toy_i2c_xfer( struct i2c_adapter *adap, struct i2c_msg *msgs,int num )
{
    int i;
    
    for(i = 0; i < num; i++)
    {
        int j;
        struct i2c_msg *msg_temp = &msgs[i];
        
        pr_emerg("[Count: %d] [%s]: [Addr = 0x%x] [Len = %d] [Data] = ", i, __func__, msg_temp->addr, msg_temp->len);
        
        for( j = 0; j < msg_temp->len; j++ )
        {
            pr_cont("[0x%02x] ", msg_temp->buf[j]);
        }
    }
    return 0;
}
/*
** This function will be called whenever you call SMBUS read, wirte APIs
*/
static s32 toy_smbus_xfer(  struct i2c_adapter *adap, 
                            u16 addr,
                            unsigned short flags, 
                            char read_write,
                            u8 command, 
                            int size, 
                            union i2c_smbus_data *data
                         )
{
   	pr_info("In %s\n", __func__);
	int i=0;
	for (i=0; i<size; i++) {
		pr_emerg("CMD:%d flags:%d size:%d addr:%d Data:%d RW:%c ", command, flags, size, addr, data->byte, read_write);
	}
	return 0;
}
/*
** I2C algorithm Structure
*/
static struct i2c_algorithm toy_i2c_algorithm = {
    .smbus_xfer     = toy_smbus_xfer,
    .master_xfer    = toy_i2c_xfer,
    .functionality  = toy_func,
};
/*
** I2C adapter Structure
*/
static struct i2c_adapter toy_i2c_adapter = {
    .owner  = THIS_MODULE,
    .class  = I2C_CLASS_HWMON,//| I2C_CLASS_SPD,
    .algo   = &toy_i2c_algorithm,
    .name   = ADAPTER_NAME,
    .nr     = -1	/*	Dynamically assign the bus number	*/
};

static int __init toy_i2c_bus_driver_init(void) {
	int ret = -1;
	ret = i2c_add_numbered_adapter(&toy_i2c_adapter);
	if (ret) pr_emerg("fetching bus number failed");
	pr_emerg("Adapter number:%d\n", toy_i2c_adapter.nr);
	return 0;
}

static void __exit toy_i2c_bus_driver_exit(void)
{
    i2c_del_adapter(&toy_i2c_adapter);
    pr_emerg("Bus Driver Removed!!!\n");
}

module_init(toy_i2c_bus_driver_init);
module_exit(toy_i2c_bus_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vikas Siddhabhaktula");
MODULE_DESCRIPTION("Toy I2C Bus driver");
MODULE_VERSION("1");
