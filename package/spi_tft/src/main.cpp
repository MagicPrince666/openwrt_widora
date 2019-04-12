/*****************************************************************************************
 * 
 * 文件名  main.cpp
 * 描述    ：此处硬件初始化和线程控制
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 * 修改时间  ：2017-09-19
 * 修复找不到图片文件会发生段错误的bug
 * 增加外部字库文件 以显示中文汉字
*****************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "LCD.h"
#include "font.h"
//#include "piclib.h"
#include "spi.h"

uint8_t ref = 0;//刷新显示

void my_quit()
{
	// pwm_disable(1);
	// pwm_disable(2);
	// pwm_disable(3);
	// pwm_disable(4);
	//close(gpio_mmap_fd);
	//SPI_Close();
	//I2C_close();
	//gifdecoding = 0;
	exit(0);
}


void xianshi()//显示信息
{   
	BACK_COLOR = WHITE;
	POINT_COLOR = RED;	
	LCD_ShowString(0,20,lcddev.width,lcddev.height,16,(char *)"https://github.com/MagicPrince666/spi-tft.git");
	LCD_ShowString(0,12+16+24,lcddev.width,lcddev.height,16,(char *)"Designer:me ! is me!");
	
	LCD_Backlight(0x00);
}
void showqq()
{ 
	uint16_t x,y; 
	x=0;
	y=80;
	while(y<lcddev.height-39)
	{
		x=0;
		while(x<lcddev.width-39)
		{
			showimage(x,y);	
			x+=40;
		}
		y+=40;
	 }	  
} 

void showimage() //显示40*40图片
{
	LCD_Clear(WHITE); //清屏  
	showqq();
	xianshi(); //显示信息 
	ref = 0;		
	//LCD_Display_Dir(L2R_U2D);		
}

static void sigint_handler(int sig)
{   
	my_quit();
    printf("-----@@@@@ sigint_handler  is over !\n"); 
}

void * thread_tft (void *arg) 
{
	Lcd_Init();   //tft初始化
	//Init_Key();
	//gui_init();
	
	BACK_COLOR = WHITE;
	POINT_COLOR = BLUE; 
	
	// piclib_init();				//piclib初始化	
	// printf("Init piclib\n");

	// printf("show jpg\n");
	// ai_load_picfile((uint8_t*)"/tft/test.jpg",0,0,240,320,0,T_JPG);//显示当前目录jpg图片
	// usleep(100*1000);
	// LCD_Clear(WHITE);
	// printf("show bmp\n");
	// ai_load_picfile((uint8_t*)"/tft/test.bmp",12,16,228,304,1,T_BMP);//显示当前目录bmp图片
	// usleep(100*1000);
	// LCD_Clear(WHITE);

	// printf("show gif\n");
	// LCD_Display_Dir(D2U_L2R);//横屏显示
	// ai_load_picfile((uint8_t*)"/tft/test.gif",0,0,lcddev.width,lcddev.height,1,T_GIF);//显示当前目录bmp图片
	// usleep(100*1000);
	// LCD_Display_Dir(DFT_SCAN_DIR);
	// LCD_Clear(WHITE);
	
	
	LCD_Display_Dir(D2U_L2R);
	time_t timer;//time_t就是long int 类型
	int times = 0;
    timer = time(NULL);
    printf("start time is: %ld\n", timer);

	for(int i = 0 ; i < 5 ; i++)
	{
		LCD_Clear(RED);times++;
		LCD_Clear(GREEN);times++;
		LCD_Clear(BLUE);times++;
		LCD_Clear(WHITE);times++; //刷屏测试
	}

	timer = time(NULL);
    printf("end time is:   %ld\n", timer);

	printf("Did you see that? demo!!\n");
	
	showimage();

	while(1)
	{							
		sleep(1);
    }
	//close(gpio_mmap_fd);
	if(gpio_fd_dc)
		close(gpio_fd_dc);
	if(gpio_fd_bl)
		close(gpio_fd_bl);
	SPI_Close();
	pthread_exit(NULL);
}


pthread_mutex_t mut;//声明互斥变量 

int main(int argc, char *argv[])
{ 
	pthread_t pthread_id[3];//线程ID
    pthread_mutex_init(&mut,NULL);

	signal(SIGINT, sigint_handler);//信号处理

	if (pthread_create(&pthread_id[0], NULL, thread_tft , NULL))
        printf("Create thread_tft error!\n");
	// if (pthread_create(&pthread_id[1], NULL, pwm_thread , NULL))
    //     printf("Create pwm_thread error!\n");
	// if (pthread_create(&pthread_id[2], NULL, oled_thread , NULL))
    //     printf("Create oled_thread error!\n");

	printf("ctrl + c to stop!\n"); 

    if(pthread_id[0] != 0) {                   
        pthread_join(pthread_id[0],NULL);
        printf("thread_tft %ld exit!\n",pthread_id[0]);
	}
	// if(pthread_id[1] != 0) {                   
    //     pthread_join(pthread_id[1],NULL);
    //     printf("pwm_thread %ld exit!\n",pthread_id[1]);
	// }
	// if(pthread_id[2] != 0) {                   
    //     pthread_join(pthread_id[2],NULL);
    //     printf("oled_thread %ld exit!\n",pthread_id[2]);
	// }
	
	return 0;
}