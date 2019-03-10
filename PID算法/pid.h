#ifndef _PID_H
#define _PID_H
#include "sys.h"
typedef struct 
{

  __IO double      SetPoint;                                 //设定目标 Desired Value
  __IO double     SumError;                                 //误差累计
  __IO double   Proportion;                               //比例常数 Proportional Const
  __IO double   Integral;                                 //积分常数 Integral Const
  __IO double   Derivative;                               //微分常数 Derivative Const
  __IO double      LastError;                                //Error[-1]
  __IO double      PrevError;                                //Error[-2]
}PID;

void IncPIDInit(double Proportion,double Integral,double Derivative)  ;
int IncPIDCalc(int iError);
int LocPIDCalc(int iError);

void IncPIDInit1(double Proportion,double Integral,double Derivative)  ;
int IncPIDCalc1(int iError);
int LocPIDCalc1(int iError);
#endif
