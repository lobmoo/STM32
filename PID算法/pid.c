#include "pid.h"
#include "usart.h"
/* 私有类型定义 --------------------------------------------------------------*/
//定义PID结构体

/* 私有宏定义 ----------------------------------------------------------------*/
/*************************************/
//定义PID相关宏
// 这三个参数设定对电机运行影响非常大
/*************************************/
#define  P_DATA      30                                //P参数
#define  I_DATA      1                                //I参数
#define  D_DATA     0.5                              //D参数

 PID sPID;
 PID *sptr = &sPID;
 PID sPID1;
 PID *sptr1 = &sPID1;


/**************PID参数初始化********************************/
void IncPIDInit(double Proportion,double Integral,double Derivative) 
{
    sptr->LastError=0;            //Error[-1]
    sptr->PrevError=0;            //Error[-2]
    sptr->Proportion=Proportion;      //比例常数 Proportional Const
    sptr->Integral=Integral;        //积分常数  Integral Const
    sptr->Derivative=Derivative;      //微分常数 Derivative Const
    //sptr->SetPoint=SetPoint;           //设定目标Desired Value
	
}
/********************增量式PID控制设计************************************/
int IncPIDCalc(int iError) 
{	
  int iIncpid;                                 //当前误差
	
  iIncpid=(int)((sptr->Proportion * iError)                 //E[k]项
              -(sptr->Integral * sptr->LastError)     //E[k-1]项
              +(sptr->Derivative * sptr->PrevError));  //E[k-2]项
              
  sptr->PrevError=sptr->LastError;                    //存储误差，用于下次计算
  sptr->LastError=iError;
  return (int)(iIncpid);                                    //返回增量值
}
/********************位置式 PID 控制设计************************************/
int LocPIDCalc(int iError)
{
	send(0xF8,1,iError);
  int dError;
  sptr->SumError += iError; //积分
  dError = iError - sptr->LastError; //微分
  sptr->LastError = iError;
  return(sptr->Proportion * iError //比例项
  + sptr->Integral * sptr->SumError //积分项
  + sptr->Derivative * dError); //微分项
}



/**************PID参数初始化********************************/
void IncPIDInit1(double Proportion,double Integral,double Derivative) 
{
    sptr1->LastError=0;            //Error[-1]
    sptr1->PrevError=0;            //Error[-2]
    sptr1->Proportion=Proportion;      //比例常数 Proportional Const
    sptr1->Integral=Integral;        //积分常数  Integral Const
    sptr1->Derivative=Derivative;      //微分常数 Derivative Const
   // sptr1->SetPoint=100;           //设定目标Desired Value
	
}
/********************增量式PID控制设计************************************/
int IncPIDCalc1(int iError) 
{
  int iIncpid;                                 //当前误差
  iIncpid=(sptr1->Proportion * iError)                 //E[k]项
              -(sptr1->Integral * sptr1->LastError)     //E[k-1]项
              +(sptr1->Derivative * sptr1->PrevError);  //E[k-2]项
              
  sptr1->PrevError=sptr1->LastError;                    //存储误差，用于下次计算
  sptr1->LastError=iError;
  return(iIncpid);                                    //返回增量值
}
/********************位置式 PID 控制设计************************************/
int LocPIDCalc1(int iError)
{
  int dError;
	send (0xF9,1,iError);
  sptr1->SumError += iError; //积分
  dError = iError - sptr1->LastError; //微分
  sptr1->LastError = iError;
  return(sptr1->Proportion * iError //比例项
  + sptr1->Integral * sptr1->SumError //积分项
  + sptr1->Derivative * dError); //微分项
}

