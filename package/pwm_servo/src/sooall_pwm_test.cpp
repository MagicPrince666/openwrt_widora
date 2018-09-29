/*------------------------------------------------------------------
Author: qianrushizaixian
refer to:  blog.csdn.net/qianrushizaixian/article/details/46536005
------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h> //-- -lm
#include <stdbool.h> //bool
#include <signal.h>
#include <iostream>

#include "sooall_pwm.h"
#include "my_socket_s.h"

#define PWM_DEV "/dev/sooall_pwm"

using namespace std;

int pwm_fd = -1;
int gpio_fd[4];
struct pwm_cfg  cfg;

static void sigint_handler(int sig)
{
	cfg.no        =   1;
	cfg.threshold = 150;
	cfg.datawidth =   2000;
	ioctl(pwm_fd,PWM_CONFIGURE, &cfg);

	cfg.no        =   0;
	cfg.threshold = 0;
	cfg.datawidth =   200;
	ioctl(pwm_fd,PWM_CONFIGURE, &cfg);
    printf("--- quit the loop! ---\n");
	exit(0);
}

int set_servo(int pwm_num)
{
	int tmp;

	cfg.no        =   pwm_num;    /* pwm0 */
	cfg.clksrc    =   PWM_CLK_100KHZ; //40MHZ or 100KHZ;
	cfg.clkdiv    =   PWM_CLK_DIV0; //DIV2 100/2=50KHZ
	cfg.old_pwm_mode = true;    /* true=old mode --- false=new mode */
	cfg.stop_bitpos = 63; // stop position of send data 0-63
	cfg.idelval   =   0;
	cfg.guardval  =   0; //
	cfg.guarddur  =   0; //
	cfg.wavenum   =   0;  /* forever loop */
	cfg.datawidth =   2000;////--limit 2^13-1=8191 100KHz/2000 = 50Hz = 20ms
	cfg.threshold =   150; //100 - 200
	//---period=1000/100(KHZ)*(DIV(1-128))*datawidth   (us)
	//---period=1000/40(MHz)*(DIV)*datawidth       (ns)
	if(cfg.old_pwm_mode == true)
	{
           if(cfg.clksrc == PWM_CLK_100KHZ)
		{
			tmp=pow(2.0,(float)(cfg.clkdiv));
			printf("tmp=%d,set PWM period=%d us\n",tmp,(int)(1000.0/100.0*tmp*(int)(cfg.datawidth))); // div by integer is dangerous!!!
		}
           else if(cfg.clksrc == PWM_CLK_40MHZ)
		{
			tmp=pow(2.0,(float)(cfg.clkdiv));
			printf("tmp=%d,set PWM period=%d ns\n",tmp,(int)(1000.0/40.0*tmp*(int)(cfg.datawidth))); // div by integer is dangerous!!!
		}
         } 

	else if(cfg.old_pwm_mode == false)
	{
		printf("senddata0= %#08x  senddata1= %#08x \n",cfg.senddata0,cfg.senddata1); 
	}

	ioctl(pwm_fd, PWM_CONFIGURE, &cfg);
	ioctl(pwm_fd, PWM_ENABLE, &cfg);
	return 0;
}

int set_moto(int pwm_num)
{
	int tmp;
	FILE* set_export;

    //打开设备节点
	set_export = fopen ("/sys/class/gpio/export", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/export!\n");
	else fprintf(set_export,"41");
	fclose(set_export);
    set_export = fopen ("/sys/class/gpio/export", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/export!\n");
	else fprintf(set_export,"42");
	fclose(set_export);
	//设置成输出
	set_export = fopen ("/sys/class/gpio/gpio41/direction", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/gpio41/direction!\n");
	else fprintf(set_export,"out");
	fclose(set_export);//设置
    set_export = fopen ("/sys/class/gpio/gpio42/direction", "w");
	if(set_export == NULL)printf ("Can't open /sys/class/gpio/gpio42/direction!\n");
	else fprintf(set_export,"out");
	fclose(set_export);//设置

	cfg.no        =   pwm_num;    /* pwm0 */
	//cfg.clksrc    =   PWM_CLK_100KHZ; //40MHZ or 100KHZ;
	cfg.clksrc    =   PWM_CLK_40MHZ; //40MHZ or 100KHZ;
	cfg.clkdiv    =   PWM_CLK_DIV0; //DIV2 40/2=20MHZ
	cfg.old_pwm_mode = true;    /* true=old mode --- false=new mode */
	cfg.stop_bitpos = 63; // stop position of send data 0-63
	cfg.idelval   =   0;
	cfg.guardval  =   0; //
	cfg.guarddur  =   0; //
	cfg.wavenum   =   0;  /* forever loop */
	cfg.datawidth =   2000;////--limit 2^13-1=8191 100KHz/2000 = 50Hz = 20ms
	cfg.threshold =   0; //100 - 200
	//---period=1000/100(KHZ)*(DIV(1-128))*datawidth   (us)
	//---period=1000/40(MHz)*(DIV)*datawidth       (ns)
	if(cfg.old_pwm_mode == true)
	{
           if(cfg.clksrc == PWM_CLK_100KHZ)
		{
			tmp=pow(2.0,(float)(cfg.clkdiv));
			printf("tmp=%d,set PWM period=%d us\n",tmp,(int)(1000.0/100.0*tmp*(int)(cfg.datawidth))); // div by integer is dangerous!!!
		}
           else if(cfg.clksrc == PWM_CLK_40MHZ)
		{
			tmp=pow(2.0,(float)(cfg.clkdiv));
			printf("tmp=%d,set PWM period=%d ns\n",tmp,(int)(1000.0/40.0*tmp*(int)(cfg.datawidth))); // div by integer is dangerous!!!
		}
         } 

	else if(cfg.old_pwm_mode == false)
	{
		printf("senddata0= %#08x  senddata1= %#08x \n",cfg.senddata0,cfg.senddata1); 
	}

	ioctl(pwm_fd, PWM_CONFIGURE, &cfg);
	ioctl(pwm_fd, PWM_ENABLE, &cfg);

	return 0;
}

void * test(void *arg)
{
	getchar();
	printf("start motor\n");

	cfg.no        =   0;
	cfg.threshold = 1000;
	ioctl(pwm_fd,PWM_CONFIGURE, &cfg);

	while(1){
		for(int tmp = 100;tmp < 200; tmp++){
			cfg.no        =   1;
			cfg.threshold = tmp;
			usleep(20000);
			ioctl(pwm_fd,PWM_CONFIGURE, &cfg);
		}
		for(int tmp = 200; tmp > 100; tmp--){
			cfg.no        =   1;
			cfg.threshold = tmp;
			usleep(20000);
			ioctl(pwm_fd,PWM_CONFIGURE,&cfg);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t pthread_id[2];//线程ID

	pwm_fd = open(PWM_DEV, O_RDWR);
	if (pwm_fd < 0) {
		printf("open pwm fd failed\n");
		system("insmod /lib/modules/3.18.29/sooall_pwm.ko");
		pwm_fd = open(PWM_DEV, O_RDWR);
		if (pwm_fd < 0) return -1;
	}
	
	//

	set_servo(1);
	set_moto(0);

	gpio_fd[0] = open ("/sys/class/gpio/gpio41/value", O_RDWR);
    gpio_fd[1] = open ("/sys/class/gpio/gpio42/value", O_RDWR);

	write(gpio_fd[0],"1",1);
	write(gpio_fd[1],"0",1);

	signal(SIGINT, sigint_handler);//信号处理
	


	if (pthread_create(&pthread_id[0], NULL, udp_net, NULL))
		cout << "Create udp_net error!" << endl;
	// if (pthread_create(&pthread_id[1], NULL, uart_rev, NULL))
	// 	cout << "Create uart_rev error!" << endl;    

	if(pthread_id[0] !=0) {                   
		pthread_join(pthread_id[0],NULL);
		cout << "udp_net "<< pthread_id[0]<< " exit!"  << endl;
	}
	// if(pthread_id[1] !=0) {                   
	// 	pthread_join(pthread_id[1],NULL);
	// 	cout << "uart_rev " << pthread_id[1] << " exit!"  << endl;
	// }
    

	return 0;
}
