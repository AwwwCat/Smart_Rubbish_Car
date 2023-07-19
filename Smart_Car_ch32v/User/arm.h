/*******************************************************************
* 文件名称 : arm.h
* 作       者 :
* 版       本 :
* 日       期 : 2023/7/19
* 描       述 : 此文件包含了机械臂的headers
*******************************************************************/

#ifndef USER_ARM_H_
#define USER_ARM_H_

#include "debug.h"

#define MECHINEARMTIME 1000 // 机械臂运动间隔

#define  delay_ms(x)  Delay_Ms(x)
#define  delay_us(x)  Delay_Us(x)


/* 钳子开关模式*/
#define Open 0
#define Close 1

/* 机械臂状态 */
#define Low 0
#define Stay 1
#define Put1 2
#define Put2 3

void Plier_Switch(u8 mode);
void Mechanical_Arm_State(u8 state);
void Mechanical_Arm_Control(char buffer, u8 state, u32 ms);


#endif /* USER_ARM_H_ */
