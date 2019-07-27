#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "send_data.h"
/****************************
新增了send_data.c文件
具体用法请看源文件
本次实验用的是串口3
用串口三接收数据。

****************************/
extern char data[20];
//extern PUartHandle pUartHandle;
int main(void)
{ 
  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	uart_init(115200);	//串口初始化波特率为115200
  usart3_init(115200);
	LED_Init();		  		//初始化与LED连接的硬件接口  
  //pUartHandle_init();
	while(1)
	{
     
  
   printf("%s\r\n", data);
   
   delay_us(1);
  }
} 


