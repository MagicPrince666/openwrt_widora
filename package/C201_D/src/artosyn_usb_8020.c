#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/kthread.h>

//in test mode
//#define _DBG_

#define USB_SKEL_VENDOR_ID        0xaaaa
#define USB_SKEL_PRODUCT_ID       0xaa97
#define DSCONT  -2

#define NORMAL_MODE      0
#define UPGRADE_MODE     1
#define CMD_BYPASS_MODE  2

#define GET_INFO                 _IOWR('s',100,long)
#define SET_BANDWITH            _IOWR('s',126,long)
#define SET_FREQ_RANG           _IOWR('s',127,long)
#define SET_FREQ_MODE           _IOWR('s',128,long)
#define SET_FREQ_MATCH	         _IOWR('s',129,long)
#define UPGRADE_START           _IOWR('s',130,long)
#define UPGRADE_DATA            _IOWR('s',131,long)
#define UPGRADE_STATE           _IOWR('s',132,long)
#define UPGRADE_COMPLETE        _IOWR('s',133,long)
#define SET_PWR_LEVER           _IOWR('s',134,long)
#define SET_CERTIFIN_MODE       _IOWR('s',135,long)
#define SET_WIR_PWR             _IOWR('s',136,long)
#define SET_WIR_QAM             _IOWR('s',137,long)
#define TO_CMD_BYPASS_MODE     _IOWR('s',138,long)
#define TO_NORMAL_MODE         _IOWR('s',139,long)


//usr cmd

#define CMD_GET_WIRINFO	    0x20
#define CMD_SET_FREQMATCH		0x10
#define CMD_SET_BANDWITH		0x11
#define CMD_SET_FREQRANG		0x12
#define CMD_SET_FREQMODE		0x13
#define CMD_SET_PWRLEVER		0x14
#define CMD_SET_CERTIFIN		0x15
#define CMD_SET_WIRPWR		    0x16
#define CMD_SET_QAM            0x17



#define CMD_UPGRADE            0x08


#define MAX_POLL_CMD	9
#define CMD_PARAM1     6

char cmd_0[7] = {0x60,0x60,0x55,0xaa,0x01,CMD_GET_WIRINFO,CMD_GET_WIRINFO};                     //查询命令
char cmd_1[7] = {0x60,0x60,0x55,0xaa,0x01,CMD_SET_FREQMATCH,CMD_SET_FREQMATCH};                 //使能对频命令
char cmd_2[8] = {0x60,0x60,0x55,0xaa,0x02,CMD_SET_BANDWITH,0x00,CMD_SET_BANDWITH};              //带宽选择命令
char cmd_3[8] = {0x60,0x60,0x55,0xaa,0x02,CMD_SET_FREQRANG,0x00,CMD_SET_FREQRANG};              //频段选择命令
char cmd_4[10] = {0x60,0x60,0x55,0xaa,0x04,CMD_SET_FREQMODE,0x00,0x00,0x00,CMD_SET_FREQMODE};	 //频点模式设置命令
char cmd_5[8] = {0x60,0x60,0x55,0xaa,0x02,CMD_SET_PWRLEVER,0x00,CMD_SET_PWRLEVER};              //设置省电模式
char cmd_6[8] = {0x60,0x60,0x55,0xaa,0x02,CMD_SET_CERTIFIN,0x00,CMD_SET_CERTIFIN};              //设置认证方式
char cmd_7[9] = {0x60,0x60,0x55,0xaa,0x03,CMD_SET_WIRPWR,0x00,0x00,CMD_SET_WIRPWR};             //设置发射功率
char cmd_8[8] = {0x60,0x60,0x55,0xaa,0x02,CMD_SET_QAM,0x00,CMD_SET_QAM};                        //设置QAM模式


typedef struct CMD_POLL{
	
    char *cmd_table;
    unsigned char run_cmd;
    unsigned char need_run;
    unsigned char cmdlen;
    unsigned int i;				//计数器
    unsigned int timeout;
	
}CMD_POLL;

CMD_POLL table_cmd[MAX_POLL_CMD] = {
	
    {cmd_0,CMD_GET_WIRINFO,0,7,0,100,},
    {cmd_1,CMD_SET_FREQMATCH,0,7,0,50,},
    {cmd_2,CMD_SET_BANDWITH,0,8,0,50,},
    {cmd_3,CMD_SET_FREQRANG,0,8,0,50,},
    {cmd_4,CMD_SET_FREQMODE,0,10,0,50,},
    {cmd_5,CMD_SET_PWRLEVER,0,8,0,50,},
    {cmd_6,CMD_SET_CERTIFIN,0,8,0,50,},
    {cmd_7,CMD_SET_WIRPWR,0,9,0,50,},
    {cmd_8,CMD_SET_QAM,0,8,0,50,},
	
};

typedef struct WIR_INFO{

    volatile uint8_t        encoder_brcidx;
    volatile uint8_t        match_state;
    volatile uint8_t        bandwidth;
    volatile uint8_t        freq_range;
    volatile uint8_t        freq_mode;
    volatile uint8_t        freq_chanl;
    volatile uint8_t       vedio_space[4];

}WIR_INFO;

WIR_INFO wir_info;

typedef struct UPGRADE{
    
    volatile uint8_t flag;
    volatile uint8_t H_id;
    volatile uint8_t L_id;
    volatile uint8_t state1;
    volatile uint8_t state2;

}UPGRADE;


/* wir_flag:
 * bit0		1: wir_info 数据有效
 *			0: wir_info 数据无效
 *
 */
int wir_flag = 0;	

UPGRADE upg_ret;

/* Get a minor range for your devices from the usb maintainer */
#define USB_SKEL_MINOR_BASE	192
/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER		2048
#define WRITES_IN_FLIGHT	8

/* table of devices that work with this driver */
static const struct usb_device_id artosyn_table[] = {
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa89) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa90) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa91) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa92) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa93) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa94) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa95) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa96) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa97) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa98) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa99) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa9a) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa9b) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa9c) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa9d) },
    { USB_DEVICE(USB_SKEL_VENDOR_ID, 0xaa9e) },
    { }
};


MODULE_DEVICE_TABLE(usb, artosyn_table);

/* Structure to hold all of our device specific stuff */
struct usb_artosyn {
    struct usb_device       *udev;			/* the usb device for this device */
    struct usb_interface    *interface;		/* the interface for this device */
    struct semaphore        limit_sem;		/* limiting the number of writes in progress */
    struct usb_anchor       submitted;		/* in case we need to retract our submissions */
    struct urb              *bulk_in_urb;		/* the urb to read data with */
    unsigned char           *bulk_in_buffer;	/* the buffer to receive data */
    size_t	bulk_in_size;		/* the size of the receive buffer */
    size_t	bulk_in_filled;		/* number of bytes in the buffer */
    size_t	bulk_in_copied;		/* already copied to user space */
    volatile __u8	bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
    volatile __u8	bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
    volatile __u8	bulk_inter_endpointAddr;	/* the address of the bulk out endpoint */
    volatile int	errors;			/* the last request tanked */
    int		open_count;		/* count the number of openers */
    bool	ongoing_read;		/* a read is going on */
    bool	processed_urb;		/* indicates we haven't processed the urb */
    spinlock_t          err_lock;		/* lock for errors */
    struct kref	        kref;
    struct mutex       io_mutex;		/* synchronize I/O with disconnect */
    struct mutex       ctrl_mutex;
    struct completion	bulk_in_completion;	/* to wait for an ongoing read */
    struct completion	int_out_completion;	/* to wait for an ongoing read */
    struct completion	bulk_out_completion;	/* to wait for an ongoing read */
#ifdef _DBG_
    struct task_struct  *fifo_test;
#endif
    struct task_struct	*fifo_thread;
    struct task_struct	*fifo_cmd_thread;
    volatile int	    fifo_size;
    volatile char       *fifo_buf;
    volatile int		*buf_filled_len;
    struct mutex	fifo_mutex;
    volatile bool	    stop;
    volatile bool	    read_fifo_en;
    volatile int		wr_counter;
    volatile int		rd_index;
    volatile int		max_wr_counter;
    volatile int		wr_index;
    volatile int        thread_cnt;

    volatile int        mode;

};

#define to_artosyn_dev(d) container_of(d, struct usb_artosyn, kref)
static struct usb_driver artosyn_driver;
static void artosyn_draw_down(struct usb_artosyn *dev);

int getfilledbuf(void *_dev)
{
	struct usb_artosyn *dev = _dev;
	int data;
	while (dev->wr_counter == 0)
	{
		if(dev->stop)
		return DSCONT;
		usleep_range(1000,2000);
	}
	data = dev->rd_index;
	if (++dev->rd_index == dev->max_wr_counter) 
	dev->rd_index=0;
	return data;
}

int getfilledindex(void *_dev)
{
	struct usb_artosyn *dev = _dev;
	int data;
	if(dev->wr_counter == 0)
		return -1;
	data = dev->rd_index;
	return data;
}

int getemptybuf(void *_dev)
{
	struct usb_artosyn *dev = _dev;
	int data;
	if(dev->wr_counter == dev->max_wr_counter)
		return dev->max_wr_counter;
	data = dev->wr_index;
	if (++dev->wr_index == dev->max_wr_counter) 
	dev->wr_index= 0;
	return data;
}

static void artosyn_delete(struct kref *kref)
{
	struct usb_artosyn *dev = to_artosyn_dev(kref);
	dev->stop = 1;
	while(dev->thread_cnt > 0)
	{
		usleep_range(10000,20000);
	}
	usb_free_urb(dev->bulk_in_urb);
	usb_put_dev(dev->udev);
	kfree((const void *)(dev->bulk_in_buffer));
	kfree((const void *)(dev->fifo_buf));
	kfree((const void *)(dev->buf_filled_len));
	kfree((const void *)dev);
}

static void stream_on_callback(struct urb *urb)
{
	struct usb_artosyn *dev;
	dev = urb->context;

	if (urb->status) {
		if (!(urb->status == -ENOENT ||
			urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN))
			printk("interface int ep err\n");

		spin_lock(&dev->err_lock);
		dev->errors = urb->status;
		spin_unlock(&dev->err_lock);
	}

	/* free up our allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,urb->transfer_buffer, urb->transfer_dma);
	complete(&dev->int_out_completion);
}

//open的时候开启视频流(固件特性)
int Stream_On(struct usb_artosyn *dev)
{
	struct urb *urb = NULL;
	char *buf = NULL;
	int retval;
		
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) 
	{
		retval = -ENOMEM;
		printk("urb alloc error\n");
		return retval;
	}
	buf = usb_alloc_coherent(dev->udev, 64, GFP_KERNEL,&urb->transfer_dma);
	if (!buf) 
	{
		retval = -ENOMEM;
		usb_free_urb(urb);
		printk("stream on alloc error\n");
		return retval;
	}
	buf[0] = 0x44;
	buf[1] = 0x44;
	buf[2] = 0x44;
	buf[3] = 0x44;
	usb_fill_int_urb(urb, dev->udev,usb_sndintpipe(dev->udev, 0x01),
									buf, 64, stream_on_callback, dev,1);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_anchor_urb(urb, &dev->submitted);
	retval = usb_submit_urb(urb, GFP_KERNEL);
	wait_for_completion_interruptible_timeout(&dev->int_out_completion,10000);
	usb_unanchor_urb(urb);
	usb_free_urb(urb);
	
	return retval;
}

static int artosyn_open(struct inode *inode, struct file *file)
{
	struct usb_artosyn *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	interface = usb_find_interface(&artosyn_driver, subminor);
	if (!interface) 
	{
		printk("error, can't find device for minor %d",subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}
	kref_get(&dev->kref);
	mutex_lock(&dev->io_mutex);
	if (!dev->open_count++) {
		retval = usb_autopm_get_interface(interface);
			if (retval) {
				dev->open_count--;
				mutex_unlock(&dev->io_mutex);
				kref_put(&dev->kref, artosyn_delete);
				goto exit;
			}
	}
	file->private_data = dev;
	if(dev->bulk_in_endpointAddr == 0x86)
	{
		if(Stream_On(dev) < 0)
		{
			printk("stream on err\n");
			retval = -ENODEV;
		}
	}
	mutex_unlock(&dev->io_mutex);

exit:
	return retval;
}

static int artosyn_release(struct inode *inode, struct file *file)
{
	struct usb_artosyn *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	mutex_lock(&dev->io_mutex);
	if (!--dev->open_count && dev->interface)
		usb_autopm_put_interface(dev->interface);
	mutex_unlock(&dev->io_mutex);

	kref_put(&dev->kref, artosyn_delete);
	return 0;
}

static int artosyn_flush(struct file *file, fl_owner_t id)
{
	struct usb_artosyn *dev;
	int res;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* wait for io to stop */
	mutex_lock(&dev->io_mutex);
	artosyn_draw_down(dev);
	/* read out errors, leave subsequent opens a clean slate */
	spin_lock_irq(&dev->err_lock);
	res = dev->errors ? (dev->errors == -EPIPE ? -EPIPE : -EIO) : 0;
	dev->errors = 0;
	spin_unlock_irq(&dev->err_lock);

	mutex_unlock(&dev->io_mutex);

	return res;
}

static void artosyn_read_bulk_callback(struct urb *urb)
{
	struct usb_artosyn *dev;

	dev = urb->context;
	spin_lock(&dev->err_lock);
	/* sync/async unlink faults aren't errors */
	if (urb->status)
	{
		if (!(urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN))
		printk("nonzero read bulk status received: %d",urb->status);
		dev->errors = urb->status;
	} 
	else 
	{
		dev->bulk_in_filled = urb->actual_length;
	}
	dev->ongoing_read = 0;
	spin_unlock(&dev->err_lock);

	complete(&dev->bulk_in_completion);
}

static int cmd_rec(struct usb_artosyn *_dev,char *buffer,int count)
{
	int index;
	int rv;
	size_t cnt = count;
	struct usb_artosyn *dev = _dev;

	if (!dev->bulk_in_urb)
		return 0;
	rv = mutex_lock_interruptible(&dev->io_mutex);
	if (rv < 0)
		return rv;
	if (!dev->interface) {		/* disconnect() was called */
		rv = -ENODEV;
		goto exit_read;
	}
	if(count > 0)
		dev->read_fifo_en = 1;
	else if(count == 0)
	{
		dev->read_fifo_en = 0;
		usleep_range(500,600);
		dev->wr_counter = 0;
		dev->rd_index = 0;
		dev->wr_index = 0;
		goto exit_read;	
		return 0;
	}

	while(cnt > 0)
	{
		index = getfilledindex(dev);

		if(index < 0 || dev->stop > 0)
			break;
		
		if(cnt >= dev->buf_filled_len[index])
		{
			memcpy((void *)buffer,(const void *)(&dev->fifo_buf[index*dev->fifo_size]),dev->buf_filled_len[index]);
			buffer += dev->buf_filled_len[index];
			cnt -= dev->buf_filled_len[index];

			if (++dev->rd_index == dev->max_wr_counter) 
				dev->rd_index=0;

			mutex_lock(&dev->fifo_mutex);
			--dev->wr_counter;
			mutex_unlock(&dev->fifo_mutex);
		}
		else
		{
			memcpy((void *)buffer,(const void *)(&dev->fifo_buf[index*dev->fifo_size]),cnt);
			buffer += cnt;
    	dev->buf_filled_len[index] -= cnt;
			memcpy((void *)(&dev->fifo_buf[index*dev->fifo_size]),(const void *)(&dev->fifo_buf[index*dev->fifo_size+cnt]),dev->buf_filled_len[index]);
			cnt = 0;
		}
	}

exit_read:
	mutex_unlock(&dev->io_mutex);
	if(rv < 0)
		return rv;
	if(dev->stop > 0)
		return -1;
	else
		return (count - cnt);
}

int is_head(char buf)
{
	static unsigned char head_cnt = 0;
	switch((unsigned char)buf)
	{
		case 0x60:
			head_cnt ++;
			break;

		case 0xaa:
			if(head_cnt == 3)
			{
				return 1;
			}
		else
			head_cnt = 1;
			break;
		case 0x55:
			if(head_cnt > 0) 
				head_cnt = 3;
			break;
	
	default:
		head_cnt = 0;
	}
	
	return 0;
}

char cmd_rec_buf[512];

void cmd_handler(char *cmd_buf,unsigned char cmd_len)
{
	unsigned char cmdid;
	int i;
	
	cmdid = cmd_buf[0];

	switch(cmdid)
	{
		case CMD_SET_FREQMATCH:
			for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQMATCH)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			printk("handler CMD_SET_FREQMATCH\n");
			break;
            
		case CMD_SET_BANDWITH:
			printk("handler CMD_SET_BANDWITH\n");
			break;
		case CMD_SET_FREQRANG:
			printk("handler CMD_SET_FREQRANG\n");
            
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQRANG)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;
            
		case CMD_SET_FREQMODE:
			printk("handler CMD_SET_FREQMODE\n");

            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQMODE)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;
            
		case CMD_SET_PWRLEVER:
			printk("handler CMD_SET_PWRLEVER\n");

            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_PWRLEVER)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;	
            
 		case CMD_SET_CERTIFIN:
			printk("handler CMD_SET_CERTIFIN\n");

            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_CERTIFIN)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;
            
		case CMD_SET_WIRPWR:
			printk("handler CMD_SET_WIRPWR\n");

            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_WIRPWR)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;

        case CMD_SET_QAM:
            printk("handler CMD_SET_QAM\n");
            
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_QAM)
                {
                    //disable run
                    if(cmd_buf[1] == 0)		//数据有效
                        table_cmd[i].need_run = 0;
                    break;
                }
            }
			break;
            
		case CMD_GET_WIRINFO: //查询命令
			
			if(cmd_buf[1] == 0)		//数据有效
			{
				*(WIR_INFO *)(&wir_info) = *(WIR_INFO *)(&cmd_buf[2]);
				wir_flag |= (1 << 0);
			}
			break;

        case CMD_UPGRADE:
            upg_ret.flag    = 1;//数据更新
            upg_ret.state1  = cmd_buf[1];
            upg_ret.state2  = cmd_buf[2];
            upg_ret.L_id    = cmd_buf[3];
            upg_ret.H_id    = cmd_buf[4];
            
            //printk("handler CMD_UPGRADE\n");
            //printk("---1: %d  2:%d  3:%d  4:%d----\n",cmd_buf[1],cmd_buf[2],cmd_buf[3],cmd_buf[4]);
            break;
            
		case 0x30:
			
			break;
	}
}

void protocol_rec_handler(struct usb_artosyn *_dev)
{
	static unsigned char cmd_rec_flag = 0;
	static unsigned char cmd_rec_len = 0;
	static unsigned char rd_cnt = 0;

	char buffer[16];
	unsigned char check_sum = 0;
	int i,ret;
	
	if(cmd_rec_flag == 2)
	{
			rd_cnt += cmd_rec(_dev,&cmd_rec_buf[rd_cnt],cmd_rec_len - rd_cnt);
			
			//一整包接收完成
			if(rd_cnt >= cmd_rec_len)
			{
				for(i = 0;i < (cmd_rec_len - 1);i ++)
				{
					check_sum += cmd_rec_buf[i];
				}
				if(check_sum == cmd_rec_buf[cmd_rec_len - 1])
				{
					cmd_handler(cmd_rec_buf,cmd_rec_len - 1);	
				}
				
				//参数回归
				cmd_rec_flag = 0;
				cmd_rec_len = 0;
				rd_cnt = 0;
					
			}
			
	}
	else
	{
		while(1)
		{
			ret = cmd_rec(_dev,buffer,1);
			if(ret > 0)
			{
				if(cmd_rec_flag == 1)
				{
					cmd_rec_flag = 2;
					cmd_rec_len = buffer[0] + 1;	//多一位是校验
					break;
				}	
		
				if(is_head(buffer[0]))
				{
					cmd_rec_flag = 1;
				}
			}
			else
				break;
		}
	}
}

static int artosyn_do_read_io(struct usb_artosyn *dev, size_t count)
{
	int rv;

	/* prepare a read */
	usb_fill_bulk_urb(dev->bulk_in_urb,
			dev->udev,
			usb_rcvbulkpipe(dev->udev,
			dev->bulk_in_endpointAddr),
			dev->bulk_in_buffer,
			min(dev->bulk_in_size, count),
			artosyn_read_bulk_callback,
			dev);
	/* tell everybody to leave the URB alone */
	spin_lock_irq(&dev->err_lock);
	dev->ongoing_read = 1;
	spin_unlock_irq(&dev->err_lock);

	/* do it */
	rv = usb_submit_urb(dev->bulk_in_urb, GFP_KERNEL);
	if (rv < 0) 
	{
		printk("failed submitting read urb, error %d",rv);
		dev->bulk_in_filled = 0;
		//rv = (rv == -ENOMEM) ? rv : -EIO;
		spin_lock_irq(&dev->err_lock);
		dev->ongoing_read = 0;
		spin_unlock_irq(&dev->err_lock);
	}
	return rv;
}

static ssize_t artosyn_read(struct file *file, char *buffer, size_t count,loff_t *ppos)
{
	int index;
	int rv;
	size_t cnt = count;
	struct usb_artosyn *dev;
	dev = file->private_data;

	if(dev->bulk_inter_endpointAddr == 0x01)		//in debug mode,no need read
	    if(dev->mode == NORMAL_MODE)
		    return 0;
		
	if (!dev->bulk_in_urb)
		return 0;
	rv = mutex_lock_interruptible(&dev->io_mutex);
	if (rv < 0)
		return rv;
	if (!dev->interface) {		/* disconnect() was called */
		rv = -ENODEV;
		goto exit_read;
	}
	if(count > 0)
		dev->read_fifo_en = 1;
	else if(count == 0)
	{
		dev->read_fifo_en = 0;
		usleep_range(500,600);
		dev->wr_counter = 0;
		dev->rd_index = 0;
		dev->wr_index = 0;
		goto exit_read;	
		return 0;
	}

	while(cnt > 0)
	{
		index = getfilledindex(dev);

		if(index < 0 || dev->stop > 0)
			break;
		
		if(cnt >= dev->buf_filled_len[index])
		{
			copy_to_user(buffer,(const void *)(&dev->fifo_buf[index*dev->fifo_size]),dev->buf_filled_len[index]);
			buffer += dev->buf_filled_len[index];
			cnt -= dev->buf_filled_len[index];

			if (++dev->rd_index == dev->max_wr_counter) 
				dev->rd_index=0;

			mutex_lock(&dev->fifo_mutex);
			--dev->wr_counter;
			mutex_unlock(&dev->fifo_mutex);
		}
		else
		{
			copy_to_user(buffer,(const void *)(&dev->fifo_buf[index*dev->fifo_size]),cnt);
			buffer += cnt;
    	dev->buf_filled_len[index] -= cnt;
			memcpy((void *)(&dev->fifo_buf[index*dev->fifo_size]),(const void *)(&dev->fifo_buf[index*dev->fifo_size+cnt]),dev->buf_filled_len[index]);
			cnt = 0;
		}

	}

exit_read:
	mutex_unlock(&dev->io_mutex);
	if(rv < 0)
		return rv;
	if(dev->stop > 0)
		return -1;
	else
		return (count - cnt);
}

static void artosyn_write_bulk_callback(struct urb *urb)
{
	struct usb_artosyn *dev;

	dev = urb->context;
	/* sync/async unlink faults aren't errors */
	if (urb->status) 
	{
		if (!(urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN))
			printk("nonzero write bulk status received: %d",urb->status);

		spin_lock(&dev->err_lock);
		dev->errors = urb->status;
		spin_unlock(&dev->err_lock);
	}
	/* free up our allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,
			  urb->transfer_buffer, urb->transfer_dma);		  
	complete(&dev->bulk_out_completion);
}

int send_cmd(void *_dev,int cmd_len,char *cmd_buf)
{
	int i;
	struct usb_artosyn *dev = _dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;
	size_t writesize = min(cmd_len, 256);

    for(i = 0;i < cmd_len;i ++)
        printk("%x ",cmd_buf[i]);

    printk("\n");
    
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) 
	{
		retval = -ENOMEM;
		goto error;
	}
	
	buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL,&urb->transfer_dma);
	if (!buf) 
	{
		retval = -ENOMEM;
		goto error;
	}

	memcpy(buf, cmd_buf, writesize);
	
	mutex_lock(&dev->io_mutex);
	if (!dev->interface) 
	{		
		mutex_unlock(&dev->io_mutex);
		retval = -ENODEV;
		goto error;
	}
	
	usb_fill_int_urb(urb, dev->udev,
		usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
		buf, writesize, artosyn_write_bulk_callback, dev,1);
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		usb_anchor_urb(urb, &dev->submitted);
	
	retval = usb_submit_urb(urb, GFP_KERNEL);
	mutex_unlock(&dev->io_mutex);
	if (retval)
	{
		goto error_unanchor;
	}
	wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,10000);
	usb_free_urb(urb);
	return writesize;

error_unanchor:
	usb_unanchor_urb(urb);
error:
	if (urb) {
		usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
		usb_free_urb(urb);
	}

	return retval;
}

void send_pkg(void *_dev,CMD_POLL *p)
{
	char buf[512];
	p->i ++;
	if(p->i > p->timeout && p->need_run)
	{

		p->i = 0;
		memcpy(buf,p->cmd_table,p->cmdlen);
		send_cmd(_dev,p->cmdlen,buf);

	}
}

static ssize_t artosyn_write(struct file *file, const char *user_buffer,
			  size_t count, loff_t *ppos)
{
	struct usb_artosyn *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;
	size_t writesize = min(count, (size_t)MAX_TRANSFER);

	dev = file->private_data;
	
	if(dev->bulk_inter_endpointAddr == 0x01)		//in debug mode,no need wirte
	    if(dev->mode == NORMAL_MODE)
		    return 0;
		
	if (count == 0)
		goto exit;
	retval = dev->errors;
	if (retval < 0) 
	{
		dev->errors = 0;
		retval = (retval == -EPIPE) ? retval : -EIO;
	}
	if (retval < 0)
		goto error;
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) 
	{
		retval = -ENOMEM;
		goto error;
	}
	buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL,&urb->transfer_dma);
	if (!buf) 
	{
		retval = -ENOMEM;
		goto error;
	}
	if (copy_from_user(buf, user_buffer, writesize)) 
	{
		retval = -EFAULT;
		goto error;
	}
	
	mutex_lock(&dev->io_mutex);
	if (!dev->interface) 
	{		
		mutex_unlock(&dev->io_mutex);
		retval = -ENODEV;
		goto error;
	}						
	usb_fill_int_urb(urb, dev->udev,
		usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
		buf, writesize, artosyn_write_bulk_callback, dev,1);
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		usb_anchor_urb(urb, &dev->submitted);
	retval = usb_submit_urb(urb, GFP_KERNEL);
	mutex_unlock(&dev->io_mutex);
	if (retval)
	{
		goto error_unanchor;
	}
	wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,10000);
	usb_free_urb(urb);
	return writesize;

error_unanchor:
	usb_unanchor_urb(urb);
error:
	if (urb) {
		usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
		usb_free_urb(urb);
	}
	up(&dev->limit_sem);

exit:
	return retval;
}

unsigned char sum_data(char *buffer,int len)
{
    int i;
    int sum = 0;
    for(i = 0;i < len;i ++)
        sum += buffer[i];

    return (unsigned char)sum;
}

static long artosyn_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int count;
	int i;
	long ret = 0;
	char buffer[512];
	struct usb_artosyn *dev;
	
	dev = file->private_data;
	if(dev->bulk_inter_endpointAddr != 0x01)
		return 0;
	
	mutex_lock(&dev->ctrl_mutex);
	
    switch(cmd)
	{		
		case GET_INFO:
			//printk("we are now 1111 at GET_INFO\n");
			if(wir_flag & 0x01)
			{
				*(WIR_INFO *)buffer = wir_info;
				copy_to_user((void *)arg,buffer,sizeof(WIR_INFO));
				ret = 0;
			}
			else
				ret = 1;//数据无效
		    break;
            
		case SET_BANDWITH:
			//printk("we are now at SET_BANDWITH\n");
            ret = 0;
		    break;
        
        case SET_FREQ_MATCH:

            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQMATCH)
                {
                    //fill cmd
                    //enable run
                    if(table_cmd[i].need_run == 1)
                        ret = 1;
                    else
                        table_cmd[i].need_run = 1;
                    break;
                }
            }
            
			printk("we are now at SET_FREQ_MATCH\n");
            ret = 0;
		    break;    
		case SET_FREQ_RANG:
			//printk("we are now at SET_FREQ_RANG\n");

            copy_from_user(buffer,(void *)arg,1);
            
            //start freq range polling
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQRANG)
                {
                    //fill cmd
                    table_cmd[i].cmd_table[CMD_PARAM1] = buffer[0];
                    table_cmd[i].cmd_table[CMD_PARAM1 + 1] = buffer[0] + CMD_SET_FREQRANG;
                    //enable run
                    if(table_cmd[i].need_run == 1)
                        ret = 1;
                    else
                        table_cmd[i].need_run = 1;
                    break;
                }
            }
            ret = 0;
		    break;
            
		case SET_FREQ_MODE:
			//printk("we are now at SET_FREQ_MODE\n");
            
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_FREQMODE)
                {
                    //fill cmd
                    copy_from_user(&table_cmd[i].cmd_table[CMD_PARAM1],(const void *)arg,3);
                    table_cmd[i].cmd_table[CMD_PARAM1 + 3] = CMD_SET_FREQMODE;
                    table_cmd[i].cmd_table[CMD_PARAM1 + 3] += table_cmd[i].cmd_table[CMD_PARAM1];
                    table_cmd[i].cmd_table[CMD_PARAM1 + 3] += table_cmd[i].cmd_table[CMD_PARAM1 + 1];
                    table_cmd[i].cmd_table[CMD_PARAM1 + 3] += table_cmd[i].cmd_table[CMD_PARAM1 + 2];
                    //enable run
                    if(table_cmd[i].need_run == 1)
                        ret = 1;
                    else
                        table_cmd[i].need_run = 1;

                    break;
                }
            }
            ret = 0;
		    break;

       case SET_PWR_LEVER:
			printk("we are now at CMD_SET_PWRLEVER\n");
            
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                if(table_cmd[i].run_cmd == CMD_SET_PWRLEVER)
                {
                    //fill cmd
                    copy_from_user(&table_cmd[i].cmd_table[CMD_PARAM1],(void *)arg,2);
                    table_cmd[i].cmd_table[CMD_PARAM1 + 1] = CMD_SET_PWRLEVER;
                    table_cmd[i].cmd_table[CMD_PARAM1 + 1] += table_cmd[i].cmd_table[CMD_PARAM1];
                    
                    //enable run
                    if(table_cmd[i].need_run == 1)
                        ret = 1;
                    else
                        table_cmd[i].need_run = 1;

                    break;
                }
            }
            ret = 0;
		    break;
       
        case SET_CERTIFIN_MODE:
             printk("we are now at CMD_SET_CERTIFIN\n");
             
             for(i = 0;i < MAX_POLL_CMD;i ++)
             {
                 if(table_cmd[i].run_cmd == CMD_SET_CERTIFIN)
                 {
                     //fill cmd
                     copy_from_user(&table_cmd[i].cmd_table[CMD_PARAM1],(void *)arg,2);
                     table_cmd[i].cmd_table[CMD_PARAM1 + 1] = CMD_SET_CERTIFIN;
                     table_cmd[i].cmd_table[CMD_PARAM1 + 1] += table_cmd[i].cmd_table[CMD_PARAM1];
                     
                     //enable run
                     if(table_cmd[i].need_run == 1)
                         ret = 1;
                     else
                         table_cmd[i].need_run = 1;

                     break;
                 }
             }
             ret = 0;
             break;

        case SET_WIR_PWR:
             printk("we are now at CMD_SET_WIRPWR\n");
             
             for(i = 0;i < MAX_POLL_CMD;i ++)
             {
                 if(table_cmd[i].run_cmd == CMD_SET_WIRPWR)
                 {
                     //fill cmd
                     copy_from_user(&table_cmd[i].cmd_table[CMD_PARAM1],(void *)arg,2);
                     table_cmd[i].cmd_table[CMD_PARAM1 + 2] = CMD_SET_WIRPWR;
                     table_cmd[i].cmd_table[CMD_PARAM1 + 2] += table_cmd[i].cmd_table[CMD_PARAM1];
                     table_cmd[i].cmd_table[CMD_PARAM1 + 2] += table_cmd[i].cmd_table[CMD_PARAM1 + 1];
                     
                     //enable run
                     if(table_cmd[i].need_run == 1)
                         ret = 1;
                     else
                         table_cmd[i].need_run = 1;

                     break;
                 }
             }
             ret = 0;
             break;

        case SET_WIR_QAM:
             printk("we are now at CMD_WIR_QAM\n");
             
             for(i = 0;i < MAX_POLL_CMD;i ++)
             {
                 if(table_cmd[i].run_cmd == CMD_SET_QAM)
                 {
                     //fill cmd
                     copy_from_user(&table_cmd[i].cmd_table[CMD_PARAM1],(void *)arg,1);
                     table_cmd[i].cmd_table[CMD_PARAM1 + 1] = CMD_SET_QAM;
                     table_cmd[i].cmd_table[CMD_PARAM1 + 1] += table_cmd[i].cmd_table[CMD_PARAM1];
                     
                     //enable run
                     if(table_cmd[i].need_run == 1)
                         ret = 1;
                     else
                         table_cmd[i].need_run = 1;

                     break;
                 }
             }
             ret = 0;
             break;
 
/*        
		case UPGRADE_START:
			printk("we are now at UPGRADE_START\n");
//close poll cmd
    		for(i = 0;i < MAX_POLL_CMD;i ++)
    		{
    			table_cmd[i].need_run = 0;
    		}
		    break;
*/

        case UPGRADE_START:
            
            dev->mode = UPGRADE_MODE;
            break;

        case TO_CMD_BYPASS_MODE:

            dev->mode = CMD_BYPASS_MODE;
            break;
            
        case TO_NORMAL_MODE:
            
            dev->mode = NORMAL_MODE;
            break;

//get info restart
		case UPGRADE_COMPLETE:
			//printk("we are now at UPGRADE_COMPLETE\n");
            dev->mode = NORMAL_MODE;
		    break;
            
		case UPGRADE_DATA:
			if(copy_from_user((void *)(&buffer[10]), (void*)arg, 4))
			{
			    mutex_unlock(&dev->ctrl_mutex);
				return -EFAULT;
			}
			count = buffer[13];
			count += buffer[12] << 8;
			count += buffer[11] << 16;
			count += buffer[10] << 24;

			if(copy_from_user((void *)(&buffer[10]), (void *)arg, count))
			{
				mutex_unlock(&dev->ctrl_mutex);
				return -EFAULT;
			}
            
            count = count - 4;
//head
            buffer[4] = 0x60;
            buffer[5] = 0x60;
            buffer[6] = 0x55;
            buffer[7] = 0xaa;
//len
            buffer[8] = count + 1;
//cmd            
            buffer[9] = 0x08;
//usr data
			memcpy((void *)(&buffer[10]),(const void *)(&buffer[14]),count);
//sum            
            buffer[count + 10] = sum_data(&buffer[9],count + 1);

            upg_ret.flag = 0;
            
            send_cmd(dev,count + 7,&buffer[4]);
            
			for(i = 0;i < (count + 30);i ++)
				printk(" %x ",buffer[i]);

            printk("\n");

			//printk("we are now at UPGRADE_DATA\n");
		    break;

//ret state
        case UPGRADE_STATE:
            //printk("we are now at UPGRADE_STATE\n");

            buffer[0] = upg_ret.flag;
            buffer[1] = upg_ret.H_id;
            buffer[2] = upg_ret.L_id;
            buffer[3] = upg_ret.state1;
            buffer[4] = upg_ret.state2;

        for(i = 0;i < 5;i ++)
            printk("# %d ",buffer[i]);
        
            copy_to_user((void *)arg,buffer,5);
            ret = 0;

            break;


	}
	
	mutex_unlock(&dev->ctrl_mutex);
	
	return ret;
}

static const struct file_operations artosyn_fops = {
	.owner =	THIS_MODULE,
	.read =		artosyn_read,
	.write =	artosyn_write,
	.open =		artosyn_open,
	.release =	artosyn_release,
	.flush =	artosyn_flush,
	.llseek =	noop_llseek,
	.unlocked_ioctl = artosyn_ioctl,
};

static struct usb_class_driver artosyn_class = {
	.name =		"artosyn_port%d",
	.fops =		&artosyn_fops,
	.minor_base =	USB_SKEL_MINOR_BASE,
};

static int fifo_handle(void *_dev)
{
	struct usb_artosyn *dev = _dev;
	int *buf_filled_len = (int *)(dev->buf_filled_len);
	int index;
	int rv;
	int max_flag;

	dev->thread_cnt ++;
	while(!dev->stop)
	{
		if(dev->read_fifo_en)
		{
			rv = artosyn_do_read_io(dev, dev->fifo_size);
			if (rv < 0)
			{
				printk("direct read error  %x \n",rv);
				usleep_range(10000,20000);
				if(-ENODEV == rv)
					break;
				continue;
			}
			
			do
			{
				rv = wait_for_completion_interruptible_timeout(&dev->bulk_in_completion,100);
			}while(rv == 0 && dev->stop == 0);

			rv = dev->errors;

			if (rv < 0) 
			{
				dev->errors = 0;
				rv = (rv == -EPIPE) ? rv : -EIO;
				dev->bulk_in_filled = 0;
				printk("read from usb error\n");
				continue;
			}
	
			if (( dev->bulk_in_urb->status == -ENOENT ||
                    		dev->bulk_in_urb->status == -ECONNRESET ||
                    		dev->bulk_in_urb->status == -ESHUTDOWN))
			{
				usleep_range(10000,11000);
				continue;
			}
		
			max_flag = 0;
			while((index = getemptybuf(dev)) ==  dev->max_wr_counter)
			{
				if ( 0 == max_flag)
				{
					printk(KERN_ERR"this buf is full, waiting the next reading----\n");
					max_flag = 1;
				}
				if(!dev->read_fifo_en)
					break;
				else if(dev->stop)
				{
					if(dev->thread_cnt > 0)
					dev->thread_cnt --;
					return 0;
				}

				usleep_range(500,600);
			}
			memcpy((void *)(dev->fifo_buf + index * dev->fifo_size),dev->bulk_in_buffer,dev->bulk_in_filled);
			buf_filled_len[index] = dev->bulk_in_filled;
			mutex_lock(&dev->fifo_mutex);
			++dev->wr_counter;
			mutex_unlock(&dev->fifo_mutex);
		}
		else
			usleep_range(20000,25000);
	}

	if(dev->thread_cnt > 0)
		dev->thread_cnt --;

	return 0;
}
	
static int fifo_cmd_handle(void *_dev)
{
	int i = 0;
	struct usb_artosyn *dev = _dev;
	
	dev->thread_cnt ++;
	while(!dev->stop)
	{

        if(dev->mode == NORMAL_MODE)
        {
            for(i = 0;i < MAX_POLL_CMD;i ++)
            {
                send_pkg(_dev,&table_cmd[i]);
            }

            protocol_rec_handler(_dev);

            usleep_range(100,110);
        }
        else
            usleep_range(100000,11000);
        
	}
	if(dev->thread_cnt > 0)
		dev->thread_cnt --;

	return 0;
}
#ifdef _DBG_
static int fifo_handle_test(void *_dev)
{
	struct usb_artosyn *dev = _dev;
	int i;
	
	dev->thread_cnt ++;
	while(!dev->stop)
	{
		printk("buf cnt in drv : %d\n",dev->wr_counter);
		for(i=0;i<10;i++)
		{
			if(dev->stop)
				break;
			usleep_range(100000,110000);
		}
	}

	if(dev->thread_cnt > 0)
		dev->thread_cnt --;

}
#endif

static int artosyn_probe(struct usb_interface *interface,const struct usb_device_id *id)
{
	struct usb_artosyn *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;

	/* allocate memory for our device state and initialize it */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) 
	{
		printk("Out of memory\n");
		goto error;
	}
	kref_init(&dev->kref);
	sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
	mutex_init(&dev->io_mutex);
	spin_lock_init(&dev->err_lock);
	init_usb_anchor(&dev->submitted);
	init_completion(&dev->bulk_in_completion);
	init_completion(&dev->int_out_completion);
	init_completion(&dev->bulk_out_completion);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* set up the endpoint information */
	/* use only the first bulk-in and bulk-out endpoints */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) 
	{
		endpoint = &iface_desc->endpoint[i].desc;
		if (!dev->bulk_in_endpointAddr && usb_endpoint_is_bulk_in(endpoint)) 
		{
			/* we found a bulk in endpoint */
			buffer_size = le16_to_cpu(endpoint->wMaxPacketSize);
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->bulk_in_buffer) 
			{
				printk("Could not allocate bulk_in_buffer\n");
				goto error;
			}
			dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
			if (!dev->bulk_in_urb) 
			{
				printk("Could not allocate bulk_in_urb\n");
				goto error;
			}
		}
		if (!dev->bulk_out_endpointAddr && usb_endpoint_is_bulk_out(endpoint)) 
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		if (!dev->bulk_inter_endpointAddr && usb_endpoint_is_int_out(endpoint)) 
			dev->bulk_inter_endpointAddr = endpoint->bEndpointAddress;
	}
	if (!(dev->bulk_in_endpointAddr) && !(dev->bulk_out_endpointAddr) && !(dev->bulk_inter_endpointAddr))
		goto error;
	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);
	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &artosyn_class);
	if (retval) {
		/* something prevented us from registering this driver */
		printk("Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	/* let the user know what node this device is now attached to */
	dev->thread_cnt = 0;
//fifo_init
	mutex_init(&dev->fifo_mutex);
	mutex_init(&dev->ctrl_mutex);
	dev->fifo_size = dev->bulk_in_size;
	dev->max_wr_counter = 0x2000;
	dev->fifo_buf = kmalloc(dev->fifo_size * dev->max_wr_counter, GFP_KERNEL);
	if (!dev->fifo_buf) 
	{
		printk("Could not allocate fifo_buffer\n");
		goto error;
	}
	dev->buf_filled_len = kmalloc(dev->max_wr_counter * sizeof(int), GFP_KERNEL);
	if (!dev->buf_filled_len) 
	{
		printk("Could not allocate buf_filled_len\n");
		goto error;
	}
	dev->wr_counter = 0;
	dev->rd_index = 0;
	dev->wr_index = 0;
	dev->read_fifo_en = 0;
	dev->stop = 0;
	dev->fifo_thread =kthread_run(fifo_handle, dev, "usb fifo");
	if (IS_ERR(dev->fifo_thread)) 
	{
		PTR_ERR(dev->fifo_thread);
		dev->fifo_thread = NULL;
		goto error;
	}
   
 
	if(dev->bulk_inter_endpointAddr == 0x01)
	{
	    dev->mode = NORMAL_MODE;
		dev->fifo_cmd_thread =kthread_run(fifo_cmd_handle, dev, "usb cmd fifo");
		if (IS_ERR(dev->fifo_cmd_thread)) 
		{
			PTR_ERR(dev->fifo_cmd_thread);
			dev->fifo_cmd_thread = NULL;
			goto error;
		}
	}

	
#ifdef _DBG_
	dev->fifo_test =kthread_run(fifo_handle_test, dev, "usb fifo test");
	if (IS_ERR(dev->fifo_test))
	{
		PTR_ERR(dev->fifo_test);
		dev->fifo_test = NULL;
		goto error;
	}
#endif
	printk("usb fifo info: fifi_size = %d\n",dev->fifo_size);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, artosyn_delete);
	return retval;
}

static void artosyn_disconnect(struct usb_interface *interface)
{
	struct usb_artosyn *dev;
	int minor = interface->minor;
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	/* give back our minor */
	usb_deregister_dev(interface, &artosyn_class);
	/* prevent more I/O from starting */
	mutex_lock(&dev->io_mutex);
	dev->interface = NULL;
	mutex_unlock(&dev->io_mutex);
	usb_kill_anchored_urbs(&dev->submitted);
	/* decrement our usage count */
	kref_put(&dev->kref, artosyn_delete);
	dev_info(&interface->dev, "USB artosyn #%d now disconnected", minor);
}

static void artosyn_draw_down(struct usb_artosyn *dev)
{
	int time;

	time = usb_wait_anchor_empty_timeout(&dev->submitted, 1000);
	if (!time)
		usb_kill_anchored_urbs(&dev->submitted);
	usb_kill_urb(dev->bulk_in_urb);
}

static int artosyn_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usb_artosyn *dev = usb_get_intfdata(intf);

	if (!dev)
		return 0;
	artosyn_draw_down(dev);
	return 0;
}

static int artosyn_resume(struct usb_interface *intf)
{
	return 0;
}

static int artosyn_pre_reset(struct usb_interface *intf)
{
	struct usb_artosyn *dev = usb_get_intfdata(intf);

	mutex_lock(&dev->io_mutex);
	artosyn_draw_down(dev);
	return 0;
}

static int artosyn_post_reset(struct usb_interface *intf)
{
	struct usb_artosyn *dev = usb_get_intfdata(intf);

	/* we are sure no URBs are active - no locking needed */
	dev->errors = -EPIPE;
	mutex_unlock(&dev->io_mutex);

	return 0;
}

static struct usb_driver artosyn_driver = {
	.name =		"skeleton",
	.probe =	artosyn_probe,
	.disconnect =	artosyn_disconnect,
	.suspend =	artosyn_suspend,
	.resume =	artosyn_resume,
	.pre_reset =	artosyn_pre_reset,
	.post_reset =	artosyn_post_reset,
	.id_table =	artosyn_table,
	.supports_autosuspend = 1,
};

static int __init usb_artosyn_init(void)
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register(&artosyn_driver);
	if (result)
		printk("usb_register failed. Error number %d", result);

	return result;
}

static void __exit usb_artosyn_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&artosyn_driver);
}

module_init(usb_artosyn_init);
module_exit(usb_artosyn_exit);

MODULE_LICENSE("GPL");
