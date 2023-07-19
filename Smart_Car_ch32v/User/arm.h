/*******************************************************************
* �ļ����� : arm.h
* ��       �� :
* ��       �� :
* ��       �� : 2023/7/19
* ��       �� : ���ļ������˻�е�۵�headers
*******************************************************************/

#ifndef USER_ARM_H_
#define USER_ARM_H_

#include "debug.h"

#define MECHINEARMTIME 1000 // ��е���˶����

#define  delay_ms(x)  Delay_Ms(x)
#define  delay_us(x)  Delay_Us(x)


/* ǯ�ӿ���ģʽ*/
#define Open 0
#define Close 1

/* ��е��״̬ */
#define Low 0
#define Stay 1
#define Put1 2
#define Put2 3

void Plier_Switch(u8 mode);
void Mechanical_Arm_State(u8 state);
void Mechanical_Arm_Control(char buffer, u8 state, u32 ms);


#endif /* USER_ARM_H_ */
