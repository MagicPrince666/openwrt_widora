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
#include <linux/vmalloc.h>


#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

//in test mode
//#define _DBG_
int Is_Stream = 1;

#define USB_SKEL_VENDOR_ID        0xaaaa
#define USB_SKEL_PRODUCT_ID       0xaa97
#define DSCONT  -2

//cmd flag
#define NORMAL_MODE       0
#define UPGRADE_MODE      1 << 1
#define _CMD_BYPASS_MODE  1 << 2

//av flag
#define TX_NEED_PARSE     3

#define UPGRADE_START           _IOWR('s',130,long)
#define UPGRADE_DATA            _IOWR('s',131,long)
#define UPGRADE_STATE           _IOWR('s',132,long)
#define UPGRADE_COMPLETE        _IOWR('s',133,long)
#define CMD_BYPASS_MODE         _IOWR('s',138,long)
#define CMD_NORMAL_MODE         _IOWR('s',139,long)
#define TX_PARSE_MODE           _IOWR('s',140,long)

#ifdef CONFIG_COMPAT

#define COMPAT_UPGRADE_START           _IOWR('s',130,compat_long_t)
#define COMPAT_UPGRADE_DATA            _IOWR('s',131,compat_long_t)
#define COMPAT_UPGRADE_STATE           _IOWR('s',132,compat_long_t)
#define COMPAT_UPGRADE_COMPLETE        _IOWR('s',133,compat_long_t)
#define COMPAT_CMD_BYPASS_MODE         _IOWR('s',138,compat_long_t)
#define COMPAT_CMD_NORMAL_MODE         _IOWR('s',139,compat_long_t)
#define COMPAT_TX_PARSE_MODE           _IOWR('s',140,compat_long_t)

#endif

/* Get a minor range for your devices from the usb maintainer */
#define USB_SKEL_MINOR_BASE    192
/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER        2048
#define WRITES_IN_FLIGHT    8

#define PKG_MAX 2048
#define usr_pkg_len 136

typedef struct UPGRADE{
    
    volatile uint8_t flag;
    volatile uint8_t H_id;
    volatile uint8_t L_id;
    volatile uint8_t state1;
    volatile uint8_t state2;

}UPGRADE;

UPGRADE upg_ret;

//for dbg----
//int pkg_head_cnt = 0;
//int pkg_end_cnt = 0;
//int rec_total_cnt = 0;
//int send_total_cnt = 0;
//------


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
    struct usb_device       *udev;            /* the usb device for this device */
    struct usb_interface    *interface;        /* the interface for this device */
    struct semaphore        limit_sem;        /* limiting the number of writes in progress */
    struct usb_anchor       submitted;        /* in case we need to retract our submissions */
    struct urb              *bulk_in_urb;        /* the urb to read data with */
    unsigned char           *bulk_in_buffer;    /* the buffer to receive data */
    size_t    bulk_in_size;        /* the size of the receive buffer */
    size_t    bulk_in_filled;        /* number of bytes in the buffer */
    size_t    bulk_in_copied;        /* already copied to user space */
    volatile __u8    bulk_in_endpointAddr;    /* the address of the bulk in endpoint */
    volatile __u8    bulk_out_endpointAddr;    /* the address of the bulk out endpoint */
    volatile __u8    bulk_inter_endpointAddr;    /* the address of the bulk out endpoint */
    volatile int    errors;            /* the last request tanked */
    int        open_count;        /* count the number of openers */
    bool    ongoing_read;        /* a read is going on */
    bool    processed_urb;        /* indicates we haven't processed the urb */
    spinlock_t          err_lock;        /* lock for errors */
    struct kref            kref;
    struct mutex       io_mutex;        /* synchronize I/O with disconnect */
    struct mutex       ctrl_mutex;
    struct completion    bulk_in_completion;    /* to wait for an ongoing read */
    struct completion    int_out_completion;    /* to wait for an ongoing read */
    struct completion    bulk_out_completion;    /* to wait for an ongoing read */
#ifdef _DBG_
    struct task_struct  *fifo_test;
#endif
    struct task_struct    *fifo_thread;
    struct task_struct    *fifo_cmd_thread;
    struct task_struct    *send_fifo_thread;
    volatile int        fifo_size;
    volatile char       *fifo_buf;
    volatile int        *buf_filled_len;
    struct mutex        fifo_mutex;
    volatile bool        rec_stop;
    volatile bool        send_stop;
    volatile bool        read_fifo_en;
    volatile bool        send_fifo_en;
    volatile int        wr_counter;
    volatile int        rd_index;
    volatile int        max_wr_counter;
    volatile int        wr_index;
    volatile int       thread_cnt;

    struct USB_TX        *usb_tx;
    volatile int        pkg_wr_index;
    volatile int        pkg_rd_index;
    volatile int        pkg_table[PKG_MAX];
    volatile int        pkg_max;
    volatile int        pkg_max_cnt;

    volatile int        mode;

};

#define to_artosyn_dev(d) container_of(d, struct usb_artosyn, kref)
static struct usb_driver artosyn_driver;
static void artosyn_draw_down(struct usb_artosyn *dev);


#define SINGLE_FIFO_SIZE   2048
#define FIFO_CNT                    1000
#define TOTAL_FIFO_SIZE    (SINGLE_FIFO_SIZE * FIFO_CNT)

typedef struct USB_TX{

    volatile unsigned int     rec_fifo_size;
    char                        rec_fifo_buf[TOTAL_FIFO_SIZE];
    volatile unsigned int     rec_max_wr_counter;
    unsigned int               rec_buf_filled_len[FIFO_CNT];
    volatile unsigned int     rec_rd_index;
    volatile unsigned int     rec_wr_index;
    
    volatile unsigned int     slen;
    volatile unsigned int     elen;
    char                        sbuf[512];
    char                        ebuf[512];
                                            
}USB_TX;

void Usb_Tx1_Init(USB_TX *usb_tx)
{
    int i;
        
    memset(usb_tx,0,sizeof(USB_TX));
    
    for(i = 0;i < 128;i ++)
    {
        usb_tx->sbuf[i] = 0x55;
        usb_tx->ebuf[i] = 0xaa;
    }
    
    usb_tx->sbuf[128] = 0x10;
    usb_tx->sbuf[129] = 0x20;
    usb_tx->sbuf[130] = 0x30;
    usb_tx->sbuf[131] = 0x40;
    usb_tx->sbuf[132] = 0x50;
    usb_tx->sbuf[133] = 0x60;
    usb_tx->sbuf[134] = 0x70;
    usb_tx->sbuf[135] = 0x80;
    
    usb_tx->ebuf[128] = 0x11;
    usb_tx->ebuf[129] = 0x21;
    usb_tx->ebuf[130] = 0x31;
    usb_tx->ebuf[131] = 0x41;
    usb_tx->ebuf[132] = 0x51;
    usb_tx->ebuf[133] = 0x61;
    usb_tx->ebuf[134] = 0x71;
    usb_tx->ebuf[135] = 0x81;
    
    for(i = 136;i < 200;i ++)
        usb_tx->ebuf[i] = 0;
    
    usb_tx->slen = 136;
    usb_tx->elen = 200;
    
    usb_tx->rec_max_wr_counter = FIFO_CNT;
    usb_tx->rec_fifo_size = SINGLE_FIFO_SIZE;
}

void Tx1_Usr_Buffer_Push(USB_TX *usb_tx,char *buffer,unsigned int count)
{
    int index;

    if((usb_tx->rec_rd_index < (usb_tx->rec_wr_index + 1)) && (usb_tx->rec_rd_index > usb_tx->rec_wr_index))
    {
        printk("usb send full\n");
        return;
    }

    index = usb_tx->rec_wr_index;

    copy_from_user((void *)(usb_tx->rec_fifo_buf + index * usb_tx->rec_fifo_size),buffer,count);
    usb_tx->rec_buf_filled_len[index] = count;

    if (++usb_tx->rec_wr_index == usb_tx->rec_max_wr_counter)
        usb_tx->rec_wr_index = 0;

}

void Tx1_Local_Buffer_Push(USB_TX *usb_tx,char *buffer,unsigned int count)
{
    int index;
    
    if((usb_tx->rec_rd_index < (usb_tx->rec_wr_index + 1)) && (usb_tx->rec_rd_index > usb_tx->rec_wr_index))
    {
        printk("usb send full\n");
        return;
    }

    index = usb_tx->rec_wr_index;
    
    memcpy((void *)(usb_tx->rec_fifo_buf + index * usb_tx->rec_fifo_size),buffer,count);
    usb_tx->rec_buf_filled_len[index] = count;

    if (++usb_tx->rec_wr_index == usb_tx->rec_max_wr_counter)
        usb_tx->rec_wr_index = 0;


}

unsigned int Tx1_Read_Align(USB_TX *usb_tx,int cnt)
{
    int checkindex;
    int total = 0;
            
    //check data len
        
    checkindex = usb_tx->rec_rd_index;
    while(checkindex != usb_tx->rec_wr_index)
    {
        total += usb_tx->rec_buf_filled_len[checkindex];
        if(total >= cnt)
            break;
        if (++checkindex == usb_tx->rec_max_wr_counter)
            checkindex = 0;
    }

    if(total >= 16)
    {
        if(cnt >= total)
            cnt = total & 0xfffffff0;
        else
            cnt = cnt & 0xfffffff0;
            
        return cnt;
    }
    else
        return 0;
        
}

unsigned int Usb_Tx1_Read(USB_TX *usb_tx,char *buffer,unsigned int count)
{
    unsigned int cnt = count;
    int index;

    while(cnt > 0)
    {

        if(usb_tx->rec_rd_index == usb_tx->rec_wr_index)
        {
           break;
        }
        index = usb_tx->rec_rd_index;

        if(cnt >= usb_tx->rec_buf_filled_len[index])
        {
            memcpy(buffer,&usb_tx->rec_fifo_buf[index*usb_tx->rec_fifo_size],usb_tx->rec_buf_filled_len[index]);
            buffer += usb_tx->rec_buf_filled_len[index];
            cnt -= usb_tx->rec_buf_filled_len[index];
            if (++usb_tx->rec_rd_index == usb_tx->rec_max_wr_counter)
                usb_tx->rec_rd_index = 0;    
        }
        else
        {
            memcpy(buffer,(const void *)(&usb_tx->rec_fifo_buf[index*usb_tx->rec_fifo_size]),cnt);
            buffer += cnt;
            usb_tx->rec_buf_filled_len[index] -= cnt;
            memcpy(&usb_tx->rec_fifo_buf[index*usb_tx->rec_fifo_size],&usb_tx->rec_fifo_buf[index*usb_tx->rec_fifo_size+cnt],usb_tx->rec_buf_filled_len[index]);
            cnt = 0;
        }
    }

    return (count - cnt);
}


int getfilledbuf(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int data;
    while (dev->wr_counter == 0)
    {
        if(dev->rec_stop)
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
    
    dev->rec_stop = 1;
    dev->send_stop = 1;
    while(dev->thread_cnt > 0)
    {
        usleep_range(10000,20000);
    }
    
    if(dev->bulk_inter_endpointAddr == 0x06)
        vfree((const void *)(dev->usb_tx));
    
    usb_free_urb(dev->bulk_in_urb);
    usb_put_dev(dev->udev);
    kfree((const void *)(dev->bulk_in_buffer));
    if(dev->fifo_buf != NULL)
        vfree((const void *)(dev->fifo_buf));
    if(dev->buf_filled_len != NULL)
        vfree((const void *)(dev->buf_filled_len));
    
    kfree((const void *)dev);
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
    usb_free_coherent(urb->dev, urb->transfer_buffer_length,urb->transfer_buffer, urb->transfer_dma);          

    complete(&dev->bulk_out_completion);
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

//open��ʱ������Ƶ��(�̼�����)
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

int send_null_qtd(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int ret;
    int retval = 0;
    struct urb *urb = NULL;
    char *buf = NULL;
    
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!urb) 
    {
        retval = -ENOMEM;
        goto error;
    }
    
    buf = usb_alloc_coherent(dev->udev, 4, GFP_KERNEL,&urb->transfer_dma);
    if (!buf) 
    {
        retval = -ENOMEM;
        goto error;
    }
    
    memset(buf,0,512);
    
    usb_fill_int_urb(urb, dev->udev,
        usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
        buf, 4, artosyn_write_bulk_callback, dev,1);
        
    urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
        
    usb_anchor_urb(urb, &dev->submitted);
    
    usb_submit_urb(urb, GFP_KERNEL);
    retval = usb_submit_urb(urb, GFP_KERNEL);
    if(retval < 0)
    {
        //printk("--- retval = %d ---\n",retval);
        goto error_unanchor;
    }
    
    ret = wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,10000);
    if(ret < 0)
        printk("sig has found:%d\n",ret);
        
    usb_free_urb(urb);
    
    return 0;

error_unanchor:
    usb_unanchor_urb(urb);
    
error:
    
    if (urb)
    {
        usb_free_coherent(dev->udev, 4, buf, urb->transfer_dma);
        usb_free_urb(urb);
    }

    return -1;
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
    
    /*
    if(dev->bulk_in_endpointAddr == 0x86)
    {
        if(Stream_On(dev) < 0)
        {
            printk("stream on err\n");
            retval = -ENODEV;
        }
    }
    */
    
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

static ssize_t artosyn_read_stream(struct file *file, char *buffer, size_t count,loff_t *ppos)
{
    int index;
    int rv;
    size_t cnt = count;
    struct usb_artosyn *dev;
    dev = file->private_data;

    if(dev->bulk_inter_endpointAddr == 0x01)        //in debug mode,no need read
        if(dev->mode == NORMAL_MODE)
            return 0;
    
    if (!dev->bulk_in_urb)
        return 0;
    rv = mutex_lock_interruptible(&dev->io_mutex);
    if (rv < 0)
        return rv;
    if (!dev->interface) {        /* disconnect() was called */
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
        if(index < 0 || dev->rec_stop > 0)
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
    if(dev->rec_stop > 0)
        return -1;
    else
        return (count - cnt);
}


static ssize_t artosyn_read_pkg(struct file *file, char *buffer, size_t count,loff_t *ppos)
{
    int index;
    int rv;
    volatile int pkg_cnt;
    struct usb_artosyn *dev;
        
    size_t cnt = count;
    dev = file->private_data;

    if(dev->bulk_inter_endpointAddr == 0x01)        //in debug mode,no need read
        if(dev->mode == NORMAL_MODE)
            return 0;
        
    if (!dev->bulk_in_urb)
        return 0;
    rv = mutex_lock_interruptible(&dev->io_mutex);
    if (rv < 0)
        return rv;
    if (!dev->interface) {        /* disconnect() was called */
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

    mutex_lock(&dev->fifo_mutex);
    if(dev->pkg_max_cnt <= 0)
    {
        mutex_unlock(&dev->fifo_mutex);
        goto exit_read;
    }
    pkg_cnt = dev->pkg_table[dev->pkg_rd_index];
    mutex_unlock(&dev->fifo_mutex);

    while(cnt > 0)
    {
        if(pkg_cnt > 0)
            pkg_cnt --;
        else
            break;
                
        index = getfilledindex(dev);
        
        if(index < 0 || dev->rec_stop > 0)
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
            if (++dev->rd_index == dev->max_wr_counter) 
                dev->rd_index=0;
    
            mutex_lock(&dev->fifo_mutex);
            --dev->wr_counter;
            mutex_unlock(&dev->fifo_mutex);
    
        }
    }

    mutex_lock(&dev->fifo_mutex);

    dev->pkg_table[dev->pkg_rd_index] = 0;
    if (++dev->pkg_rd_index == dev->pkg_max)
        dev->pkg_rd_index = 0;

    if(dev->pkg_max_cnt > 0)
    -- dev->pkg_max_cnt;
    
    mutex_unlock(&dev->fifo_mutex);

exit_read:
    mutex_unlock(&dev->io_mutex);
    if(rv < 0)
        return rv;
    if(dev->rec_stop > 0)
        return -1;
    else
        if((count - cnt) > usr_pkg_len)
            return (count - cnt - usr_pkg_len);
        else
            return 0;
}

static ssize_t artosyn_read(struct file *file, char *buffer, size_t count,loff_t *ppos)
{
    struct usb_artosyn *dev;
    dev = file->private_data;

    if(dev->bulk_in_endpointAddr == 0x86 && Is_Stream == 0)
        return artosyn_read_pkg(file,buffer,count,ppos);
    else
        return artosyn_read_stream(file,buffer,count,ppos);
}

static ssize_t artosyn_write(struct file *file, const char *user_buffer,
              size_t count, loff_t *ppos)
{
    struct usb_artosyn *dev;
    int retval = 0;
    struct urb *urb = NULL;
    char *buf = NULL;
    char head_buf[128];
    int usr_data_len;
    int head_len = 8;
    char *usr_data = NULL; 
    int i;
    size_t cnt;
    size_t writesize;
    
    dev = file->private_data;
    
    if(dev->bulk_inter_endpointAddr == 0x01)        //in debug mode,no need wirte
        if(dev->mode == NORMAL_MODE)
            return 0;
    
    if (count == 0)
        return 0;
    
    retval = dev->errors;
    if (retval < 0) 
    {
        dev->errors = 0;
        goto exit;
    }
    
    if (!dev->interface) 
    {
        retval = -ENODEV;
        goto exit;
    }                
    
    //in parse mode
    if(dev->mode == TX_NEED_PARSE)
    {
        if(Is_Stream)
        {
            
            urb = usb_alloc_urb(0, GFP_KERNEL);
            if (!urb) 
            {
                retval = -ENOMEM;
                goto exit;
            }
            
            if(copy_from_user(head_buf, user_buffer, count))
            {
                retval = -EFAULT;
                goto exit;
            }
            
            usr_data_len = head_buf[4] << 24 | head_buf[5] << 16 | head_buf[6] << 8 | head_buf[7];
            usr_data = (char *)(head_buf[8] << 24| head_buf[9] << 16 | head_buf[10] << 8 | head_buf[11]);

            writesize = min((head_len + usr_data_len), (size_t)MAX_TRANSFER);
            
            buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL,&urb->transfer_dma);
            if (!buf) 
            {
                retval = -ENOMEM;
                goto exit;
            }
            
            for(i = 0;i < head_len;i ++)
                buf[i] = head_buf[i];
            
            if(copy_from_user(buf + head_len, usr_data, usr_data_len))
            {
                retval = -EFAULT;
                goto exit;
            }
            
            mutex_lock(&dev->io_mutex);
            
            usb_fill_int_urb(urb, dev->udev,
            usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
            buf, writesize, artosyn_write_bulk_callback, dev,1);
            
            urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
            usb_anchor_urb(urb, &dev->submitted);
            retval = usb_submit_urb(urb, GFP_KERNEL);
            
            mutex_unlock(&dev->io_mutex);
                
            if (retval)
            {
                usb_unanchor_urb(urb);
            
                if (urb) 
                {
                    usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
                    usb_free_urb(urb);
                }
                goto exit;
            }
            
            wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,10000);
            usb_free_urb(urb);
            
            return (writesize - head_len);
                
        }
        else
        {
            if(copy_from_user(head_buf, user_buffer, count))
            {
              retval = -EFAULT;
              goto exit;
            }
            
            usr_data_len = head_buf[4] << 24 | head_buf[5] << 16 | head_buf[6] << 8 | head_buf[7];
            usr_data = (char *)(head_buf[8] << 24| head_buf[9] << 16 | head_buf[10] << 8 | head_buf[11]);
            cnt = usr_data_len;
            
            if(cnt > 0)
                dev->send_fifo_en = 1;
            else if(cnt == 0)
            {
                dev->send_fifo_en = 0;
                usleep_range(500,600);
                return 0;
            }
            
            //����usr data len �Ӱ�ͷ���Ӱ�β
            
            usr_data_len += dev->usb_tx->slen;
            usr_data_len += dev->usb_tx->elen;
            
            head_buf[4] = (char)(usr_data_len >> 24); //usr data len
            head_buf[5] = (char)(usr_data_len >> 16); //usr data len
            head_buf[6] = (char)(usr_data_len >> 8);  //usr data len
            head_buf[7] = (char)usr_data_len;         //usr data len
        
                
            //���Ͱ�ͷ
            Tx1_Local_Buffer_Push(dev->usb_tx,head_buf,head_len);
            
            Tx1_Local_Buffer_Push(dev->usb_tx,dev->usb_tx->sbuf,dev->usb_tx->slen);
            
            //�û�����
            
            while(cnt >= SINGLE_FIFO_SIZE)
            {
            
                Tx1_Usr_Buffer_Push(dev->usb_tx,usr_data,SINGLE_FIFO_SIZE);
                
                usr_data += SINGLE_FIFO_SIZE;
                cnt -= SINGLE_FIFO_SIZE;
            
            }
                
            if(cnt > 0)
            {
                Tx1_Usr_Buffer_Push(dev->usb_tx,usr_data,cnt);
            }
            
            
            //���Ͱ�β
            Tx1_Local_Buffer_Push(dev->usb_tx,dev->usb_tx->ebuf,dev->usb_tx->elen);
            
            return usr_data_len;
        }
    }
    else    //stream
    {
        writesize = min(count, (size_t)MAX_TRANSFER);
        
        urb = usb_alloc_urb(0, GFP_KERNEL);
        if (!urb) 
        {
            retval = -ENOMEM;
            goto exit;
        }
        
        buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL,&urb->transfer_dma);
        if (!buf) 
        {
            if (urb)
            {
                usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
                usb_free_urb(urb);
            }
            
            retval = -ENOMEM;
            goto exit;
        }
        
        if (copy_from_user(buf, user_buffer, writesize)) 
        {
        
            if (urb)
            {
                usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
                usb_free_urb(urb);
            }
            
          retval = -EFAULT;
          goto exit;
        }
        
        mutex_lock(&dev->io_mutex);
        
        usb_fill_int_urb(urb, dev->udev,
        usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
        buf, writesize, artosyn_write_bulk_callback, dev,1);
        
        urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
        usb_anchor_urb(urb, &dev->submitted);
        retval = usb_submit_urb(urb, GFP_KERNEL);
        
        mutex_unlock(&dev->io_mutex);
        
        if (retval)
        {
            usb_unanchor_urb(urb);
        
            if (urb) 
            {
                usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
                usb_free_urb(urb);
            }
            goto exit;
        }
        
        wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,10000);
        usb_free_urb(urb);
        
        return writesize;
        
  }

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
    if (urb) 
    {
        usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
        usb_free_urb(urb);
    }

    return retval;
}


static long artosyn_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int count;
    int i;
    long ret = 0;
    char buffer[512];
    struct usb_artosyn *dev;
    
    dev = file->private_data;
//    if(dev->bulk_inter_endpointAddr != 0x01)
//        return 0;
    
    mutex_lock(&dev->ctrl_mutex);
    
    switch(cmd)
    {        
        
        case UPGRADE_START:
            
            dev->mode = UPGRADE_MODE;
            break;
        
        case CMD_BYPASS_MODE:
        
            dev->mode = _CMD_BYPASS_MODE;
            break;
            
        case CMD_NORMAL_MODE:
            
            dev->mode = NORMAL_MODE;
            break;
        
        case TX_PARSE_MODE:
            
            dev->mode = TX_NEED_PARSE;
            
            break;
                      
            
        //get info restart
        case UPGRADE_COMPLETE:
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

            break;
        
        //ret state
        case UPGRADE_STATE:
        
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

#ifdef CONFIG_COMPAT

static long compat_artosyn_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    void __user *arg64 = compat_ptr(arg);
    if(!file->f_op || !file->f_op->unlocked_ioctl)
    {
        printk("file->f_op or file->f_op->unlocked_ioctl is NULL!!\n");
        return -ENOTTY;
    }

    switch(cmd)
    {        

        case COMPAT_UPGRADE_START:     
            ret = file->f_op->unlocked_ioctl(file, UPGRADE_START, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_UPGRADE_START failed!\n");
        break;
        
        case COMPAT_CMD_BYPASS_MODE:       
            ret = file->f_op->unlocked_ioctl(file, CMD_BYPASS_MODE, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_CMD_BYPASS_MODE failed!\n");
        break;
        
        case COMPAT_CMD_NORMAL_MODE:        
            ret = file->f_op->unlocked_ioctl(file, CMD_NORMAL_MODE, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_CMD_NORMAL_MODE failed!\n");
        break;
        
        case COMPAT_UPGRADE_COMPLETE:       
            ret = file->f_op->unlocked_ioctl(file, UPGRADE_COMPLETE, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_UPGRADE_COMPLETE failed!\n");
        break;
        
        case COMPAT_UPGRADE_DATA:
            ret = file->f_op->unlocked_ioctl(file, UPGRADE_DATA, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_UPGRADE_DATA failed!\n");
        break;

        case COMPAT_TX_PARSE_MODE:
            ret = file->f_op->unlocked_ioctl(file, TX_PARSE_MODE, (unsigned long)arg64);
            if (ret < 0)
                printk("COMPAT_TX_PARSE_MODE failed!\n");
        break;

        default:
            printk("CMD unsupported!!!!!!\n");
        break;

    }
    
    return ret;
}
#endif

static const struct file_operations artosyn_fops = {
    .owner =    THIS_MODULE,
    .read =        artosyn_read,
    .write =    artosyn_write,
    .open =        artosyn_open,
    .release =    artosyn_release,
    .flush =    artosyn_flush,
    .llseek =    noop_llseek,
    .unlocked_ioctl = artosyn_ioctl,

#ifdef CONFIG_COMPAT
    .compat_ioctl = compat_artosyn_ioctl,
#endif

};

static struct usb_class_driver artosyn_class = {
    .name =        "artosyn_port%d",
    .fops =        &artosyn_fops,
    .minor_base =    USB_SKEL_MINOR_BASE,
};


static int send_fifo_handle(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int retval;
    struct urb *urb = NULL;
    char *buf = NULL;
    int r_cnt;

    dev->thread_cnt ++;
        
    while(!dev->send_stop)
    {
    
        if(dev->send_fifo_en)
        {
            r_cnt = Tx1_Read_Align(dev->usb_tx,SINGLE_FIFO_SIZE);
            if(r_cnt <= 0)
            {
                usleep_range(500,600);
                
                if(dev->send_stop || !dev->interface)
                    break;
                        
                continue;
            }
            
            urb = usb_alloc_urb(0, GFP_KERNEL);
            if (!urb) 
            {
                retval = -ENOMEM;
                printk("send urb alloc error\n");
                return retval;
            }
            
            buf = usb_alloc_coherent(dev->udev,r_cnt,GFP_KERNEL,&urb->transfer_dma);
            if (!buf)
            {
                retval = -ENOMEM;
                usb_free_urb(urb);
                printk("fifo handle buf alloc error\n");
                return retval;
            }

            Usb_Tx1_Read(dev->usb_tx,buf,r_cnt);

            usb_fill_int_urb(urb, dev->udev,
            usb_sndintpipe(dev->udev, dev->bulk_inter_endpointAddr),
            buf, r_cnt, artosyn_write_bulk_callback, dev,1);
            
            urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
//            usb_anchor_urb(urb, &dev->submitted);
            retval = usb_submit_urb(urb, GFP_KERNEL);
            
//            send_total_cnt += r_cnt;
            
            if (retval) 
            {
                    printk("send fifo urb error %d---r_cnt = %d",retval,r_cnt);
            }

            do{
                
                if(dev->send_stop || !dev->interface)
                    break;
                retval = wait_for_completion_interruptible_timeout(&dev->bulk_out_completion,200);
            
            }while(retval == 0);
            
            usb_free_urb(urb);
                
            if(dev->send_stop || !dev->interface)
                    break;
                        
        }
        else
            usleep_range(20000,25000);
    }

    printk("write fifo handle end\n");

    if(dev->thread_cnt > 0)
        dev->thread_cnt --;

    return 0;

}


volatile int find_state = 0;
volatile int start_step = 0;
volatile int end_step = 0;

int is_pkg_start(char num)
{
/*
    unsigned char start[10] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};

    if(start[start_step] == num)
        start_step ++;
    else
        start_step = 0;
*/ 
    switch((unsigned char)num)
    {
        case 0x10:
            start_step = 1;
        break;

        case 0x20:
            if(start_step == 1)
                start_step = 2;
            else
                start_step = 0;
        break;

        case 0x30:
            if(start_step == 2)            
                start_step = 3;
            else
                start_step = 0;
        break;

        case 0x40:
            if(start_step == 3)
                start_step = 4;
            else
                start_step = 0;
        break;

        case 0x50:
            if(start_step == 4)
                start_step = 5;
            else
                start_step = 0;
        break;

        case 0x60:
            if(start_step == 5)
                start_step = 6;
            else
                start_step = 0;
        break;

        case 0x70:
            if(start_step == 6)
                start_step = 7;
            else
                start_step = 0;
        break;

        case 0x80:
            if(start_step == 7)
                start_step = 8;
            else
                start_step = 0;
        break;

        default:
            start_step  = 0;
    }   


        
    if(start_step == 8)
    {
        start_step = 0;
        return 1;
    }
    else
        return 0;

}


int is_pkg_end(char num)
{
/*
    unsigned char end[10] = {0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x81};

    if(end[end_step] == num)
        end_step ++;
    else
        end_step = 0;
*/    

    switch((unsigned char)num)
    {
        case 0x11:
            end_step = 1;
        break;

        case 0x21:
            if(end_step == 1)
                end_step = 2;
            else
                end_step = 0;
        break;

        case 0x31:
            if(end_step == 2)            
                end_step = 3;
            else
                end_step = 0;
        break;

        case 0x41:
            if(end_step == 3)
                end_step = 4;
            else
                end_step = 0;
        break;

        case 0x51:
            if(end_step == 4)
                end_step = 5;
            else
                end_step = 0;
        break;

        case 0x61:
            if(end_step == 5)
                end_step = 6;
            else
                end_step = 0;
        break;

        case 0x71:
            if(end_step == 6)
                end_step = 7;
            else
                end_step = 0;
        break;

        case 0x81:
            if(end_step == 7)
                end_step = 8;
            else
                end_step = 0;
        break;

        default:
            end_step  = 0;
    }   


    if(end_step == 8)
    {
        end_step = 0;
        return 1;
    }
    else
        return 0;

}

static int fifo_handle_pkg(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int *buf_filled_len = (int *)(dev->buf_filled_len);
    int index;
    int rv;
    int max_flag;
    int hig,low,mid;
    volatile int i,j;
  volatile int head_num = 0;
  volatile int need_back = 0;

    dev->thread_cnt ++;
    while(!dev->rec_stop)
    {
        i = 0;
        j = 0;
        
        if(dev->read_fifo_en)
        {
            if(dev->fifo_buf == NULL || dev->buf_filled_len == NULL)
            {
                usleep_range(10000,20000);
                continue;
            }    
                
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
            }while(rv == 0 && dev->rec_stop == 0);

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

            //rec_total_cnt += dev->bulk_in_filled;

            head_num= 0;
        
            for(i = 0;i < 5;i ++)
            {
                switch(find_state)
                {

                   case 2:         //βδ����

                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i] == 0xaa)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 1] == 0xaa)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 2] == 0xaa)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 3] == 0xaa)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 4] == 0xaa)
                        {
                            if(i == 0)
                                need_back = 1;
                            else
                                i --;
                            
                            find_state = 3;
                        }
                    
                        break;
                        
                    case 0:         //ͷδ����

                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i] == 0x55)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 1] == 0x55)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 2] == 0x55)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 3] == 0x55)
                        if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * i + 4] == 0x55)
                        {
                
                            if(i == 0)
                                need_back = 1;
                            else
                                i --;

                            find_state = 1;

                        }
                        break;
                        
                    case 1:         //ͷ������

                        if(need_back)
                        {
                            need_back = 0;
                            i --;
                        }
                        
                        //�ֲ�

                        low = dev->bulk_in_filled / 5 * i;
                        hig = dev->bulk_in_filled;

                        for(j = i;j < 5;j ++)
                            if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * j] != 0x55)
                            {
                                hig = dev->bulk_in_filled / 5 * j;
                                break;
                            }
                            
                        if(j == 5)
                            hig = dev->bulk_in_filled;
                        
                        while((low + 16) < hig)
                        {

                        //printk("%d_%d\n",low,hig);
                        
                            mid = (low + hig) / 2;
                            
                            if((dev->bulk_in_buffer[mid] == 0x55)
                                &&(dev->bulk_in_buffer[mid + 1] == 0x55)
                                &&(dev->bulk_in_buffer[mid + 2] == 0x55)
                                &&(dev->bulk_in_buffer[mid + 3] == 0x55)
                                &&(dev->bulk_in_buffer[mid + 4] == 0x55))
                                    low = mid;
                            else
                                    hig = mid;
                            
                        }
                                           
                        //ϸ��
                        for(j = low;j < dev->bulk_in_filled;j ++)
                        //for(j = dev->bulk_in_filled / 5 * i;j < dev->bulk_in_filled;j ++)
                        {
                        
                            if(dev->bulk_in_buffer[j] == 0x55)
                            {
                                continue;
                            }
                            
                            if(is_pkg_start(dev->bulk_in_buffer[j]))
                            {
 
                                //pkg_head_cnt ++;
                                
                                head_num = j + 1;

                                //����i��ֵ
                                i = j * 5 / dev->bulk_in_filled;
                                find_state = 2;

                                break;
                            }
                        }

                        break;

                    case 3:         //β������

                        if(need_back)
                        {
                            need_back = 0;
                            i --;
                        }
                        
                        //�ֲ�
                        low = dev->bulk_in_filled / 5 * i;
                        hig = dev->bulk_in_filled;

                        for(j = i;j < 5;j ++)
                            if(dev->bulk_in_buffer[dev->bulk_in_filled / 5 * j] != 0xaa)
                            {
                                hig = dev->bulk_in_filled / 5 * j;
                                break;
                            }
                        if(j == 5)
                            hig = dev->bulk_in_filled;
                      

                        while((low + 16) < hig)
                        {
                            mid = (low + hig) / 2;
                            
                            if((dev->bulk_in_buffer[mid] == 0xaa)
                                &&(dev->bulk_in_buffer[mid + 1] == 0xaa)
                                &&(dev->bulk_in_buffer[mid + 2] == 0xaa)
                                &&(dev->bulk_in_buffer[mid + 3] == 0xaa)
                                &&(dev->bulk_in_buffer[mid + 4] == 0xaa))
                                    low = mid;
                            else
                                    hig = mid;   
                        }

                        //ϸ��
                        for(j = low;j < dev->bulk_in_filled;j ++)
                        //for(j = dev->bulk_in_filled / 5 * i;j < dev->bulk_in_filled;j ++)
                        {
                            if(dev->bulk_in_buffer[j] == 0xaa)
                                continue;
                            
                            if(is_pkg_end(dev->bulk_in_buffer[j]))
                            {
                                //pkg_end_cnt ++;

                                //���ݴ���
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
                                    else if(dev->rec_stop)
                                    {
                                        if(dev->thread_cnt > 0)
                                        dev->thread_cnt --;
                                        return 0;
                                    }
                                    usleep_range(500,600);
                                }

                                
                                buf_filled_len[index] = j + 1 - head_num;
                                memcpy((void *)(dev->fifo_buf + index * dev->fifo_size),&dev->bulk_in_buffer[head_num],buf_filled_len[index]);

                                mutex_lock(&dev->fifo_mutex);
                                
                                ++dev->wr_counter;
                                
                                dev->pkg_table[dev->pkg_wr_index] ++;
                                
                                if (++dev->pkg_wr_index == dev->pkg_max)
                                    dev->pkg_wr_index = 0;
                                //pkg add
                                ++ dev->pkg_max_cnt;
                                
                                mutex_unlock(&dev->fifo_mutex);
                                
                                i = j * 5 / dev->bulk_in_filled;
                                
//                                rec_total_cnt += (j + 1 - head_num);
//                                pkg_head_cnt += buf_filled_len[index];

                                find_state = 0;

                                break;
                            }
                        }

                        break;
                }
    
            }


            if(find_state == 2 || find_state == 3)
            {
                if(head_num < dev->bulk_in_filled)
                {
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
                        else if(dev->rec_stop)
                        {
                            if(dev->thread_cnt > 0)
                            dev->thread_cnt --;
                            return 0;
                        }
                        usleep_range(500,600);
                    }

                    buf_filled_len[index] = dev->bulk_in_filled - head_num;
                    memcpy((void *)(dev->fifo_buf + index * dev->fifo_size),&dev->bulk_in_buffer[head_num],dev->bulk_in_filled);
                    
                    mutex_lock(&dev->fifo_mutex);
                    
                    ++dev->wr_counter;
                    dev->pkg_table[dev->pkg_wr_index] ++;

                    mutex_unlock(&dev->fifo_mutex);

//                    rec_total_cnt += (dev->bulk_in_filled - head_num);

//                    pkg_head_cnt += buf_filled_len[index];

                }
            }
        }
        else
            usleep_range(20000,25000);
    }

    if(dev->thread_cnt > 0)
        dev->thread_cnt --;

    return 0;
}

static int fifo_handle_stream(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int *buf_filled_len = (int *)(dev->buf_filled_len);
    int index;
    int rv;
    int max_flag;

    dev->thread_cnt ++;
    while(!dev->rec_stop)
    {
        if(dev->read_fifo_en)
        {
            
            if(dev->fifo_buf == NULL || dev->buf_filled_len == NULL)
            {
                usleep_range(10000,20000);
                continue;
            }    
            
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
            }while(rv == 0 && dev->rec_stop == 0);

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
                else if(dev->rec_stop)
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
    

#ifdef _DBG_
static int fifo_handle_test(void *_dev)
{
    struct usb_artosyn *dev = _dev;
    int i;
    
    dev->thread_cnt ++;
    while(!dev->rec_stop)
    {

        //printk("rec_total_cnt : %d  head : %d end : %d\n",rec_total_cnt,pkg_head_cnt,pkg_end_cnt);
        printk("pkg_wr_index : %d  pkg_rd_index : %d pkg_max_cnt : %d sr_counter : %d\n",dev->pkg_wr_index,dev->pkg_rd_index,dev->pkg_max_cnt,dev->wr_counter);
        
        for(i=0;i<10;i++)
        {
            if(dev->rec_stop)
                break;
            usleep_range(200000,210000);
        }
    }

    if(dev->thread_cnt > 0)
        dev->thread_cnt --;

    return 0;
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
    
    if(dev->bulk_in_endpointAddr == 0x86)
        dev->max_wr_counter = 0x800;
    else
        dev->max_wr_counter = 0x200;
    
    dev->fifo_buf = vmalloc(dev->fifo_size * dev->max_wr_counter);
    if (!dev->fifo_buf) 
    {
        printk("Could not allocate fifo_buffer\n");
        goto error;
    }
    dev->buf_filled_len = vmalloc(dev->max_wr_counter * sizeof(int));
    if (!dev->buf_filled_len) 
    {
        printk("Could not allocate buf_filled_len\n");
        goto error;
    }

    //pkg init

    dev->pkg_wr_index = 0;
    dev->pkg_rd_index = 0;
    dev->pkg_max = PKG_MAX;
    dev->pkg_max_cnt = 0;
    for(i = 0;i < dev->pkg_max;i ++)
      dev->pkg_table[i] = 0;

    dev->wr_counter = 0;
    dev->rd_index = 0;
    dev->wr_index = 0;
    dev->read_fifo_en = 0;
    dev->rec_stop = 0;
    dev->send_stop = 0;
    
    if(dev->bulk_in_endpointAddr == 0x86 && Is_Stream == 0)
    {
        dev->fifo_thread = kthread_run(fifo_handle_pkg, dev, "usb fifo pkg");
        if (IS_ERR(dev->fifo_thread)) 
        {
            PTR_ERR(dev->fifo_thread);
            dev->fifo_thread = NULL;
            goto error;
        }
    }
    else
    {
        dev->fifo_thread =kthread_run(fifo_handle_stream, dev, "usb fifo stream");
        if (IS_ERR(dev->fifo_thread)) 
        {
            PTR_ERR(dev->fifo_thread);
            dev->fifo_thread = NULL;
            goto error;
        }
    }

    if(dev->bulk_inter_endpointAddr == 0x06)
    {
      //send a null qtd to work around the hi kernel bug
      send_null_qtd(dev);
      
        //data init
        dev->usb_tx = vmalloc(sizeof(USB_TX));
        
        Usb_Tx1_Init(dev->usb_tx);
        
        dev->send_fifo_thread = kthread_run(send_fifo_handle, dev, "usb send fifo");
        if (IS_ERR(dev->send_fifo_thread)) 
        {
            dev->send_fifo_thread = NULL;
            goto error;
        }
    }


#ifdef _DBG_

    if(dev->bulk_in_endpointAddr == 0x86)
    {
          dev->fifo_test =kthread_run(fifo_handle_test, dev, "usb fifo test");
        if (IS_ERR(dev->fifo_test))
        {
            PTR_ERR(dev->fifo_test);
            dev->fifo_test = NULL;
            goto error;
        }
    }
    
#endif


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
    .name =        "skeleton",
    .probe =    artosyn_probe,
    .disconnect =    artosyn_disconnect,
    .suspend =    artosyn_suspend,
    .resume =    artosyn_resume,
    .pre_reset =    artosyn_pre_reset,
    .post_reset =    artosyn_post_reset,
    .id_table =    artosyn_table,
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
