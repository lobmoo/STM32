
#include "SPTA.h"
#include "led.h"
#include "usart.h"
#include <stdlib.h>
#include "test.h"
#include "pid.h"
#include "delay.h"
  extern int pid; 
  int VelAccumulator=0; 		
  int ActualAcceleration=0;	
  int VelAdd=0; 				
  int ActualVelocity=0;		
  int PosAccumulator=0;  //位置累加
  int PosAdd=0;           //位置增加值
  int ActualPosition=0;    //实际位置
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
          ActualAcceleration=pid;	
       if (ActualAcceleration!=0) 
			 {				 
         VelAccumulator+=ActualAcceleration;    
         VelAdd = VelAccumulator >> 13; 
          VelAccumulator -= VelAdd << 13;   //???????????  				 
            if(VelAdd>0)
							 ActualVelocity++;
						else if(VelAdd<0)
							 ActualVelocity--;
			if(ActualVelocity>5000)
			  	 ActualVelocity=5000;
			 if(ActualVelocity<-5000)
				  ActualVelocity=-5000;
					}
			  else
				{
				VelAccumulator=0;
				VelAdd =0;	
				ActualVelocity=0;	
				}
				PosAccumulator+=ActualVelocity;   //ActualVelocity; 
			  	PosAdd = PosAccumulator >> 13;    //?????????????
			    PosAccumulator -= PosAdd << 13;   //???????????
				if(PosAdd!=0) 
			{	 
       if(PosAdd>0)				
			 {
				 PBout(4)=0;
         	ActualPosition++;	
           if(ActualPosition>500)
            ActualPosition=500;  						 
			 }
			 else
			 {
			   PBout(4)=1; 
				 ActualPosition--;
        if(ActualPosition<-500)
            ActualPosition=-500;  				 
			 }
			  PBout(5)=~PBout(5);
		 }
				if (PosAdd==0)
					 ActualPosition=0;

}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位

}
