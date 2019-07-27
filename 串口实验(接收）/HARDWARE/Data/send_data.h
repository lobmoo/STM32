#ifndef _SEND_DATA_H
#define _SEND_DATA_H
#include <stdio.h>
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
#define RX_BUF_1_LENGTH 256 //接收长度

typedef struct pos //状态指示
{
    int step;
    uint16_t tmpCnt;
    int aRxBufIndex;
    char aRxBuf_1[RX_BUF_1_LENGTH];
}PUartHandle;
void Init_data_array(char *data);
void pUartHandle_init(void);
void Receive_Data(uint8_t str, char *Data);
#endif
