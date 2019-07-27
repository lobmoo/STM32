#include "sys.h"
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "stdarg.h"
#include "usart.h"
#define USART3_MAX_RECV_LEN 200
#define USART3_MAX_SEND_LEN 200
#define _head_1 0x23        //帧头'#'
#define _head_2 0x40        //帧头'@'                                    //帧尾
int RX_Value = 0;
char send_str[256]; //发送数据缓冲区
//////////////////////////////////////////////////////////////////////////////////
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h" //ucos 使用
#endif

//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4探索者开发板
//串口1初始化
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/6/10
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
//////////////////////////////////////////////////////////////////////////////////
//串口发送缓存区
__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; //发送缓冲,最大USART3_MAX_SEND_LEN字节
#ifdef USART3_RX_EN
#endif								   //如果使能了接收
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; //接收缓冲,最大USART3_MAX_RECV_LEN个字节.

#define channel_number 3
unsigned char DataScope_OutPut_Buffer[42] = {0}; //串口发送缓冲区
u8 pstr[channel_number * 4 + 1];				 //
int state;
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
void _ttywrch(int x)
{
	x = x;
}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; //循环发送,直到发送完毕
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0; //接收状态标记

//初始化IO 串口1
//bound:波特率
void uart_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //使能USART1时钟

	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); //GPIOA10复用为USART1

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA9，PA10

	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;										//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART1, &USART_InitStructure);										//初始化串口1

	USART_Cmd(USART1, ENABLE); //使能串口1

	//USART_ClearFlag(USART1, USART_FLAG_TC);

#if EN_USART1_RX
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		  //串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器、

#endif

	//USART_ClearFlag(USART1, USART_FLAG_TC);
}
//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void usart2_init(u32 bound)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //使能USART2时钟

	//USART1_TX   PA.2 PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //PA2,PA3,复用功能,上拉输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		   //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		   //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		   //上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   //初始化PA2，PA3

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); //GPIOA3复用为USART2

	USART_InitStructure.USART_BaudRate = bound;										//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 发模式
	USART_Init(USART2, &USART_InitStructure);										//初始化串口
	USART_Cmd(USART2, ENABLE);														//使能串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);									//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		  //串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器、
}

void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  //使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3时钟

	USART_DeInit(USART3); //复位串口3

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); //GPIOB11复用为USART3
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); //GPIOB10复用为USART3

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10; //GPIOB11和GPIOB10初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			 //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			 //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			 //上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //初始化GPIOB11，和GPIOB10

	USART_InitStructure.USART_BaudRate = bound;										//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口3

	USART_Cmd(USART3, ENABLE); //使能串口

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //开启中断

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器
}

void USART1_IRQHandler(void) //串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART1); //(USART1->DR);	//读取接收到的数据

		if ((USART_RX_STA & 0x8000) == 0) //接收未完成
		{
			if (USART_RX_STA & 0x4000) //接收到了0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; //接收错误,重新开始
				else
					USART_RX_STA |= 0x8000; //接收完成了
			}
			else //还没收到0X0D
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; //接收数据错误,重新开始接收
				}
			}
		}
	}
#if SYSTEM_SUPPORT_OS //如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();
#endif
}

int UART_Value(void)
{
	int reg;
	if (RX_Value)
	{
		reg = RX_Value;
		RX_Value = 0;
	}
	return reg;
}

void USART2_IRQHandler(void) //串口1中断服务程序
{
	u8 Res;

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART2); //(USART2->DR);	//读取接收到的数据
		if (Res == '$')
		{
			state = channel_number * 4 + 1;
			state--;
		}
		else
		{
			if (state > 0)
				pstr[(channel_number * 4 + 1) - state] = Res;
			state--;
		}
	}
}
int receive(int data[])
{
	union {
		unsigned char float_byte[sizeof(int)];

		int value;
	} f_b;
	int i;
	int cnt = channel_number;
	if (state == 0)
	{
		for (i = 0; i < cnt; ++i)
		{
			f_b.float_byte[0] = pstr[4 * i + 1];
			f_b.float_byte[1] = pstr[4 * i + 2];
			f_b.float_byte[2] = pstr[4 * i + 3];
			f_b.float_byte[3] = pstr[4 * i + 4];
			data[i] = f_b.value;
		}
		state = channel_number * 4 + 1;
		return 1;
	}
	else
		for (i = 0; i < cnt; ++i)
			data[i] = 0;
	return 0;
}

void send(u8 channel, int cnt, ...)
{
	union {
		u8 float_byte[sizeof(u16)];
		u16 value;
	} f_b;
	int i;
	const int size = 2;
	u8 str[size * cnt + 5];
	va_list ap;
	va_start(ap, cnt);
	str[0] = 0xAA;
	str[1] = 0xAA;
	str[2] = channel;
	str[3] = cnt * size;
	str[size * cnt + 4] = 0;
	for (i = 0; i < cnt; ++i)
	{
		f_b.value = va_arg(ap, int);
		str[size * i + 4] = f_b.float_byte[1];
		str[size * i + 5] = f_b.float_byte[0];
	}
	va_end(ap);
	for (i = 0; i < size * cnt + 4; i++)
	{
		str[size * cnt + 4] = (str[size * cnt + 4] + str[i]) & 0xff; //?????
		while ((USART1->SR & 0X40) == 0)
			; //循环发送,直到发送完毕
		USART1->DR = (u8)str[i];
	}
	while ((USART1->SR & 0X40) == 0)
		; //循环发送,直到发送完毕
	USART1->DR = (u8)str[size * cnt + 4];
}

//通过判断接收连续2个字符之间的时间差不大于100ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过100ms,则认为不是1次连续数据.也就是超过100ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
u16 USART3_RX_STA = 0;
void USART3_IRQHandler(void)
{
	u8 res;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收到数据
	{

		res = USART_ReceiveData(USART3);
		if ((USART3_RX_STA & (1 << 15)) == 0) //接收完的一批数据,还没有被处理,则不再接收其他数据
		{
			if (USART3_RX_STA < USART3_MAX_RECV_LEN) //还可以接收数据
			{
				TIM_SetCounter(TIM7, 0); //计数器清空
				if (USART3_RX_STA == 0)
					TIM_Cmd(TIM7, ENABLE);			  //使能定时器7
				USART3_RX_BUF[USART3_RX_STA++] = res; //记录接收到的值
			}
			else
			{
				USART3_RX_STA |= 1 << 15; //强制标记接收完成
			}
		}
	}
}
//初始化IO 串口3
//bound:波特率

//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u3_printf(char *fmt, ...)
{
	u16 i, j;
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char *)USART3_TX_BUF, fmt, ap);
	va_end(ap);
	i = strlen((const char *)USART3_TX_BUF); //此次发送数据的长度
	for (j = 0; j < i; j++)					 //循环发送数据
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
			;											   //等待上次传输完成
		USART_SendData(USART3, (uint8_t)USART3_TX_BUF[j]); //发送数据到串口3
	}
}
/*

stm32串口发送字符函数

*/
void SendChar(uint8_t Data)

{

USART_SendData (USART1 ,Data);

while(USART_GetFlagStatus (USART1 ,USART_FLAG_TC )!=SET );

}


/*
stm32串口发送字符串函数
*/

void SendString(char *str)
{
 while(*str)
{
  SendChar(*str);
  str++;
}
}

void Send_Data(char *fmt, ...) 
{
    va_list ap; //初始化指向可变参数列表的指针
    int len=0;          //计算数据长度
    va_start(ap, fmt);         
    vsprintf(send_str, fmt, ap);
    len=strlen(send_str);
    char _data[len+4]; //定义数组来按照协议存放数据；（算上帧头帧尾的长度+数据校验位）
    _data[0] = _head_1;
    _data[1] =_head_2;
    _data[2] =len;
    for (int i = 0; i < len; i++)
    {
        _data[i+3]=send_str[i];

    }
    _data[len+3]='\0';
    SendString(_data);  //这里调用sendstring；
    va_end(ap);                
}
#endif
