/*******************************************************************
* 文件名称 : car.h
* 作       者 :
* 版       本 :
* 日       期 : 2023/7/19
* 描       述 : 此文件包含了小车的headers
*******************************************************************/

#ifndef USER_CAR_H_
#define USER_CAR_H_

#include "debug.h"

#define TURNDELAY 500       // 旋转延迟

void Car_Run();
void Car_Stop();
void Car_Front_Run(u32 ms);
void Car_Back_Run(u32 ms);
void Car_Turn_Right(u32 ms);
void Car_Turn_Left(u32 ms);
void Echo_Switch(u8 mode);
void Car_Control(char state);


#endif /* USER_CAR_H_ */
