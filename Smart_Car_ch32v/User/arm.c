/*******************************************************************
* 文件名称 : arm.c
* 作       者 :
* 版       本 :
* 日       期 : 2023/7/19
* 描       述 : 此文件包含了arm.c的headers
*******************************************************************/


#include "arm.h"


/********************************************************************
* 函数名称      : Plier_Switch
* 函数功能      : 控制机械钳开关
* 输       入      : u8 mode          钳子开关模式
* 输       出      : 无
*********************************************************************/
void Plier_Switch(u8 mode)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, !mode);
}



/********************************************************************
* 函数名称      : Mechanical_Arm_State
* 函数功能      : 控制机械臂状态
* 输       入      : u8 state         机械臂状态
* 输       出      : 无
*********************************************************************/
void Mechanical_Arm_State(u8 state)
{
    switch (state) {
        case Low:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 0);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 0);           // 00机械臂落下
            break;
        case Stay:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);           // 11机械臂抬起
            break;
        case Put1:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 0);           // 10机械臂转到1号框位
            break;
        case Put2:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 0);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);           // 01机械臂转到2号框位
            break;
        default:
            break;
    }
}

/********************************************************************
* 函数名称      : Mechanical_Arm_Control
* 函数功能      : 蓝牙控制机械臂
* 输       入      : char state           机械臂状态
*            u32 ms               机械臂状态变化间的延迟
* 输       出      : 无
*********************************************************************/
u16 MA_1 = 1;                                 // 钳子状态，默认开
void Mechanical_Arm_Control(char buffer, u8 state, u32 ms)
{
    if (buffer ==  '1' || state == 1)                        // 机械臂第一种运动过程
    {
        Mechanical_Arm_State(Low);
        delay_ms(ms);
        Plier_Switch(Close);
        delay_ms(ms);
        Mechanical_Arm_State(Stay);
        delay_ms(ms);
        Mechanical_Arm_State(Put1);
        delay_ms(ms);
        Plier_Switch(Open);
        delay_ms(ms);
        Mechanical_Arm_State(Stay);
    } else if (buffer ==  '2' || state == 2)                 // 机械臂第二种运动过程
    {
        Mechanical_Arm_State(Low);
        delay_ms(ms);
        Plier_Switch(Close);
        delay_ms(ms);
        Mechanical_Arm_State(Stay);
        delay_ms(ms);
        Mechanical_Arm_State(Put2);
        delay_ms(ms);
        Plier_Switch(Open);
        delay_ms(ms);
        Mechanical_Arm_State(Stay);
    } else if (buffer ==  'A')                 // 钳子控制
    {
        MA_1 = !MA_1;                         // 钳子状态取反
        Plier_Switch(MA_1);
    }else if (buffer ==  'B')                  // 机械臂落下
    {
        Mechanical_Arm_State(Low);
    }else if (buffer ==  'C')                  // 机械臂抬起
    {
        Mechanical_Arm_State(Stay);
    }else if (buffer ==  'D')                  // 机械臂转到1号框位
    {
        Mechanical_Arm_State(Put1);
    }else if (buffer ==  'E')                  // 机械臂转到2号框位
    {
        Mechanical_Arm_State(Put2);
    }
}
