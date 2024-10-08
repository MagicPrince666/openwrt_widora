#include "pstwo.h"
#include "sys.h"

uint16_t Handkey;
uint8_t Comd[2]={0x01,0x42};	
uint8_t Data[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //

uint16_t MASK[]={
    PSB_SELECT,
    PSB_L3,
    PSB_R3 ,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1 ,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK
	};	//


void PS2_Init(void)
{
	if (gpio_mmap())
		printf("error\n");

	mt76x8_gpio_set_pin_direction(14, 0);
	mt76x8_gpio_set_pin_direction(15, 1);
	mt76x8_gpio_set_pin_direction(16, 1);
	mt76x8_gpio_set_pin_direction(17, 1);
	mt76x8_gpio_set_pin_value(14, 1);
	PS2_JOYPAD_CMND_1;
	PS2_JOYPAD_ATT_1;	
	PS2_JOYPAD_CLOCK_1;							  
}

//
void PS2_Cmd(uint8_t CMD)
{
	volatile uint16_t ref=0x01;
	Data[1] = 0;
	for(ref=0x01;ref<0x0100;ref<<=1)
	{
		if(ref&CMD)
		{
			PS2_JOYPAD_CMND_1;                   //
		}
		else PS2_JOYPAD_CMND_0;

		PS2_JOYPAD_CLOCK_1;                        //
		usleep(5);
		PS2_JOYPAD_CLOCK_0;
		usleep(5);
		PS2_JOYPAD_CLOCK_1;
		if(PS2_JOYPAD_DATA)
			Data[1] = ref|Data[1];
	}
	usleep(16);
}


uint8_t PS2_RedLight(void)
{
	PS2_JOYPAD_ATT_0;
	PS2_Cmd(Comd[0]);  
	PS2_Cmd(Comd[1]);  
	PS2_JOYPAD_ATT_1;
	if( Data[1] == 0X73)   return 0 ;
	else return 1;

}

void PS2_ReadData(void)
{
	volatile uint8_t byte=0;
	volatile uint16_t ref=0x01;

	PS2_JOYPAD_ATT_0;

	PS2_Cmd(Comd[0]);  
	PS2_Cmd(Comd[1]);  

	for(byte=2;byte<9;byte++)
	{
		for(ref=0x01;ref<0x100;ref<<=1)
		{
			PS2_JOYPAD_CLOCK_1;
			usleep(5);
			PS2_JOYPAD_CLOCK_0;
			usleep(5);
			PS2_JOYPAD_CLOCK_1;
		      if(PS2_JOYPAD_DATA)
		      Data[byte] = ref|Data[byte];
		}
        usleep(16);
	}
	PS2_JOYPAD_ATT_1;	
}


uint8_t PS2_DataKey()
{
	uint8_t index;

	PS2_ClearData();
	PS2_ReadData();

	Handkey=(Data[4]<<8)|Data[3];     
	for(index=0;index<16;index++)
	{	    
		if((Handkey&(1<<(MASK[index]-1)))==0)
		return index+1;
	}
	return 0;         
}


uint8_t PS2_AnologData(uint8_t button)
{
	return Data[button];
}


void PS2_ClearData()
{
	uint8_t a;
	for(a=0;a<9;a++)
		Data[a]=0x00;
}


/******************************************************
Function:    void PS2_Vibration(u8 motor1, u8 motor2)
Description: �ֱ��𶯺�����
Calls:		 void PS2_Cmd(u8 CMD);
Input: motor1:�Ҳ�С�𶯵�� 0x00�أ�������
	   motor2:�����𶯵�� 0x40~0xFF �������ֵԽ�� ��Խ��
******************************************************/
void PS2_Vibration(uint8_t motor1, uint8_t motor2)
{
	PS2_JOYPAD_ATT_0;
	usleep(16);
  	PS2_Cmd(0x01);  //
	PS2_Cmd(0x42);  //
	PS2_Cmd(0X00);
	PS2_Cmd(motor1);
	PS2_Cmd(motor2);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	usleep(16);  
}
//short poll
void PS2_ShortPoll(void)
{
	PS2_JOYPAD_ATT_0;
	usleep(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x42);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x00);
	PS2_JOYPAD_ATT_1;
	usleep(16);	
}

void PS2_EnterConfing(void)
{
  	PS2_JOYPAD_ATT_0;
	usleep(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01);
	PS2_Cmd(0x00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	usleep(16);
}


void PS2_TurnOnAnalogMode(void)
{
	PS2_JOYPAD_ATT_0;
	PS2_Cmd(0x01);  
	PS2_Cmd(0x44);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x01); //analog=0x01;digital=0x00  ������÷���ģʽ
	PS2_Cmd(0xEE); //Ox03�������ã�������ͨ��������MODE������ģʽ��
				   //0xEE������������ã���ͨ��������MODE������ģʽ��
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_Cmd(0X00);
	PS2_JOYPAD_ATT_1;
	usleep(16);
}

void PS2_VibrationMode(void)
{
	PS2_JOYPAD_ATT_0;
	usleep(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x4D);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0X01);
	PS2_JOYPAD_ATT_1;
	usleep(16);	
}
//
void PS2_ExitConfing(void)
{
    PS2_JOYPAD_ATT_0;
	usleep(16);
	PS2_Cmd(0x01);  
	PS2_Cmd(0x43);  
	PS2_Cmd(0X00);
	PS2_Cmd(0x00);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_Cmd(0x5A);
	PS2_JOYPAD_ATT_1;
	usleep(16);
}

void PS2_SetInit(void)
{
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_ShortPoll();
	PS2_EnterConfing();		
	PS2_TurnOnAnalogMode();	
	PS2_VibrationMode();	
	PS2_ExitConfing();	
}



