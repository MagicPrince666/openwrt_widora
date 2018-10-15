#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/gpio.h>
 
#define DRIVER_NAME    "mpu9250"
#define INT_PIN_CFG     (0x37)
#define AK8963_CTRL1   (0xA)
static struct i2c_client *mpu9250_i2c_device;
#define MPU9250_I2C_ADAPTER         	(0) 
 
//struct work_struct			polling_work ; 
struct delayed_work polling_work;
 
s32 mpu9250_i2c_read_data(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter, &msg, 1);
}
 
s32 mpu9250_i2c_write_data(struct i2c_client *client, u8 *data, u8 length)
{
	struct i2c_msg msg;
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = length;
	msg.buf = data;
	return i2c_transfer(client->adapter, &msg, 1);
}
 
void master_write_i2c_reg(struct i2c_client *client, u8 addr, u8 reg_data)
{
	unsigned char data[2] = {0, 0};
	unsigned int len = 0;
 
	data[len++] = (addr & 0xff);
	data[len++] = (reg_data & 0xff);
	
	mpu9250_i2c_write_data(client, data, len);
 
	return ;
}
 
void master_read_i2c_reg(struct i2c_client *client, u8 addr, u8 *ret_data, u8 length)
{
	
	unsigned char data[2];
	unsigned int len = 0;
	
	data[len++] = (addr & 0xff);
 
	mpu9250_i2c_write_data(client, data, len);
	
	mpu9250_i2c_read_data(client, ret_data, length);
	
	return ;
}
static void set_mode(struct i2c_client *client,int bypass)
{
	u8 data[2] = {0,0};
	u8 reg55_data = 0;
 
	master_read_i2c_reg(client,INT_PIN_CFG,data,1);
	//printk("[%s]%d: data[0] = 0x%2x\n",data[0]);
	if(bypass)
		reg55_data = data[0]| (0x2);
	else
		reg55_data = data[0] & (0xfd);
	master_write_i2c_reg(client,INT_PIN_CFG,reg55_data);
	//master_read_i2c_reg(client,INT_PIN_CFG,data,1);
	//printk("[%s]%d: data[0] = 0x%2x\n",data[0]);
}
static void read_reg(struct i2c_client *client)
{
	u8 data[2] = {0,0};
	u8 i = 0 ;
	u16 out_data = 0;
 
	client->addr = 0x68;
 
	master_read_i2c_reg(client,(59),data,1);
	master_read_i2c_reg(client,(60),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("x = %d,",out_data);
	master_read_i2c_reg(client,(61),data,1);
	master_read_i2c_reg(client,(62),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("y = %d,",out_data);
	master_read_i2c_reg(client,(63),data,1);
	master_read_i2c_reg(client,(64),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("z = %d,",out_data);
	printk("acc end.-\n");
 
 
	master_read_i2c_reg(client,(67),data,1);
	master_read_i2c_reg(client,(68),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("x = %d,",out_data);
	master_read_i2c_reg(client,(69),data,1);
	master_read_i2c_reg(client,(70),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("y = %d,",out_data);
	master_read_i2c_reg(client,(71),data,1);
	master_read_i2c_reg(client,(72),&data[1],1);
	out_data = data[0] << 8  | data[1];
	printk("z = %d,",out_data);
	printk("gyro end.-\n");
 
 
	//master_read_i2c_reg(client,(73),data,2);
	//printk("%d,%d---\n",data[0],data[1]);
	
 
	set_mode(client,1);
	client->addr=0x0c;
 
	master_read_i2c_reg(client,(0),data,1);
	master_read_i2c_reg(client,(4),&data[1],1);
	out_data = data[1] << 8  | data[0];
	printk("x = %d,",out_data);
	master_read_i2c_reg(client,(5),data,1);
	master_read_i2c_reg(client,(6),&data[1],1);
	out_data = data[1] << 8  | data[0];
	printk("y = %d,",out_data);
	master_read_i2c_reg(client,(7),data,1);
	master_read_i2c_reg(client,(8),&data[1],1);
	out_data = data[1] << 8  | data[0];
	printk("z = %d,",out_data);
	printk("mag end.-\n");
	client->addr = 0x68;
	set_mode(client,0);
 
 
}
static void data_polling( struct work_struct *work )
{
	printk("%s,%d=================\n",__func__,__LINE__);
	read_reg(mpu9250_i2c_device);
	schedule_delayed_work(&polling_work,50);
	
}
static int mpu9250_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	INIT_DELAYED_WORK(&polling_work, data_polling);
 
	//set_mode(client,1);// set to bypass mode
	//client->addr = 0x0C;
	//master_write_i2c_reg(client,0xA,0xe);
	//client->addr = 0x68;
	//set_mode(client,0);
 
	/*master_write_i2c_reg(client, 36, 0x40);
	master_write_i2c_reg(client, 40, 0x0A);
	master_write_i2c_reg(client, 41, 0x01);
	master_write_i2c_reg(client, 42, 0x81);
	master_write_i2c_reg(client, 100,0x01);
	*/
	schedule_work(&polling_work) ;
	return 0;
}
 
static int mpu9250_i2c_remove(struct i2c_client *client)
{
	cancel_delayed_work_sync(&polling_work);
	return 0;
}
 
static const struct i2c_device_id mpu9250_i2c_id_table[] = {
	{ DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, mpu9250_i2c_id_table);
 
static struct i2c_driver mpu9250_i2c_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
 
	.probe		= mpu9250_i2c_probe,
	.remove		= mpu9250_i2c_remove,
	//.remove		= __devexit_p(mpu9250_i2c_remove),
	.id_table	= mpu9250_i2c_id_table,
};
 
static struct i2c_board_info mpu9250_i2c_info = {
	 I2C_BOARD_INFO(DRIVER_NAME, 0x68),               
};
 
 
static int __init mpu9250_i2c_init(void)
{
        int result = -1;
	printk("%s,%d=====2============\n",__func__,__LINE__);
	struct i2c_adapter *adap = i2c_get_adapter(MPU9250_I2C_ADAPTER);
	mpu9250_i2c_device = i2c_new_device(adap, &mpu9250_i2c_info); 
	
	result = i2c_add_driver(&mpu9250_i2c_driver);
	if (result) {
		pr_err("failed\n");
		return result;
	}
	return 0;
}
 
static void __exit mpu9250_i2c_exit(void)
{
 	i2c_del_driver(&mpu9250_i2c_driver);
	i2c_unregister_device(mpu9250_i2c_device);
}
 
module_init(mpu9250_i2c_init);
module_exit(mpu9250_i2c_exit);
 
 
MODULE_AUTHOR("frank");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mpu9250 sensor driver");


