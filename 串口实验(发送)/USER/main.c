#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "key.h"
/**********************************
发送函数新增了
void SendChar(uint8_t Data)   //发送单个字符串
void SendString(char *str)    //发送字符串
void Send_Data(char *fmt, ...)  //发送一帧完整可变长数据，可以是仍以类型
第一个参数为数据类型，用法和printf一样，注意发送时不要有空格，第二个参数可以是任意类型，和第一个匹配使用
这三个函数在uart里面定义

***********************************/


int main(void)
{ 
   float c=111.26;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);		//延时初始化 
	uart_init(115200);	//串口初始化波特率为115200
	LED_Init();		  		//初始化与LED连接的硬件接口  
	while(1)
	{
    Send_Data("%f",c);
    //delay_ms(20);
  }
} 


