#include <stdio.h>
#include "pstwo.h"

int main(int argc, char **argv){

	uint8_t key = 0;
	uint8_t l_lx = 0,l_ly = 0,l_rx = 0,l_ry = 0;
	uint8_t lx,ly,rx,ry;
	int speed = 0;
	PS2_Init();
	PS2_SetInit();

	while(1)
	{
		key = PS2_DataKey();
		if(key != 0)               
		{
			printf("key = %d\r\n",key);
			
			if(key == 12)
			{
				PS2_Vibration(0xFF,0x00);
				usleep(500000);
			}
			else if(key == 11)
			{
				PS2_Vibration(0x00,0xFF); 
				usleep(500000);
			}
			else
				PS2_Vibration(0x00,0x00); 
			
			switch(key)
			{
				case 5:;
				//case 7:pwm_config(pwm1, 1500000, 20000000, 1);break;
				//case 6:pwm_config(pwm1, 2000000, 20000000, 1);break;
				//case 8:pwm_config(pwm1, 1000000, 20000000, 1);break;
			}

		}
		
		lx = PS2_AnologData(PSS_LX);
		ly = PS2_AnologData(PSS_LY);
		rx = PS2_AnologData(PSS_RX);
		ry = PS2_AnologData(PSS_RY);

		if(ry < 120)
		{
			speed = (128 - ry)/128 * 1000000;
			//gpio_set_value(F1C100S_GPIOE3,1);
			//gpio_set_value(F1C100S_GPIOE4,0);
			//pwm_config(pwm0, speed, 1000000, 1);
		}
		else if(ry > 135)
		{
			speed = (ry -128)/128 * 1000000;
			//gpio_set_value(F1C100S_GPIOE3,0);
			//gpio_set_value(F1C100S_GPIOE4,1);
			//pwm_config(pwm0, speed, 1000000, 1);
		}
		else
		{
			//gpio_set_value(F1C100S_GPIOE3,1);
			//gpio_set_value(F1C100S_GPIOE4,1);
			//pwm_config(pwm0, 0, 1000000, 1);
		}

		if(l_lx != lx)
		{
			l_lx = lx;
			printf("LX = %d\r\n",lx);
		}
		if(l_ly != ly)
		{
			l_ly = ly;
			printf("LY = %d\r\n",ly);	
		}
		if(l_rx != rx)
		{
			l_rx = rx;
			printf("RX = %d\r\n",rx);
		}
		if(l_ry != ry)
		{
			l_ry = ry;
			printf("RY = %d\r\n",ry);
		}

		usleep(1000);	
	}

	return 0;
}