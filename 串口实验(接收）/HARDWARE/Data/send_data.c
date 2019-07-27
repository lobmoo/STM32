#include <stdio.h>
#include "send_data.h"
#include <stdlib.h>
#include <string.h>
#define _head_1 0x23        //帧头'#'
#define _head_2 0x40        //帧头'@'
PUartHandle pUartHandle;
void Init_data_array(char *data)     //数组初始化
{
    int i = 0;
    while (data[i++] != '\0')
    {
        data[i] = 0;
    }
}

void pUartHandle_init(void) //结构体初始化函数    
{

    pUartHandle.step = 0;

    pUartHandle.tmpCnt = 0;

    pUartHandle.aRxBufIndex = 0;
}
/*
接收函数
第一个参数是接受到的字符，第二个为接受数组
接受数组需要用户自定义
且函数放在接受中断里面运行，保证实时性
*/
void Receive_Data(uint8_t str, char *Data)
{  
     
    switch (pUartHandle.step)
    {
    case 0:
      
        if (str == _head_1)   //帧头检验
        {

            pUartHandle.step++;
            pUartHandle.aRxBuf_1[pUartHandle.aRxBufIndex++] = str; /*?????*/
        }
        break;
    case 1:
        if (str == _head_2)  //第二个帧头检验
        {
            pUartHandle.step++;
            pUartHandle.aRxBuf_1[pUartHandle.aRxBufIndex++] = str;
           
        }
        else if (str == _head_1) //防止帧头与数据重复
            pUartHandle.step = 1;
        else
        {
            
            pUartHandle_init();
        }

        break;

    case 2:
        if (str == _head_1)
        {
            pUartHandle.step = 1;
            pUartHandle.aRxBufIndex = 1;
        }
        else
        {
           
            pUartHandle.tmpCnt =(int)str; //将数据长度赋值给TMPCNT
            pUartHandle.step++;
            pUartHandle.aRxBuf_1[pUartHandle.aRxBufIndex++] = str;                           /*开始存入数据*/
            if (((RX_BUF_1_LENGTH - pUartHandle.aRxBufIndex) < str) || (str == 0x00)) //防止数据溢出或者收到错误值
            {

                pUartHandle_init();
            }
        }
        break;

    case 3:
      
        pUartHandle.aRxBuf_1[pUartHandle.aRxBufIndex++] = str;
        pUartHandle.tmpCnt--;
        if (pUartHandle.aRxBufIndex >= RX_BUF_1_LENGTH)
        {
            pUartHandle_init();
        }
        if (pUartHandle.tmpCnt == 0) //说明接收完成
        {
            strcpy(Data, pUartHandle.aRxBuf_1);
            Init_data_array(pUartHandle.aRxBuf_1);  //每收完一帧数据后缓冲区清零。防止溢出
            pUartHandle_init();                       //初始化
        }

        break;

    default:
        pUartHandle_init();
        break;
    }
}
