/*******************************************************************
* �ļ����� : arm.c
* ��       �� :
* ��       �� :
* ��       �� : 2023/7/19
* ��       �� : ���ļ�������arm.c��headers
*******************************************************************/


#include "arm.h"


/********************************************************************
* ��������      : Plier_Switch
* ��������      : ���ƻ�еǯ����
* ��       ��      : u8 mode          ǯ�ӿ���ģʽ
* ��       ��      : ��
*********************************************************************/
void Plier_Switch(u8 mode)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, !mode);
}



/********************************************************************
* ��������      : Mechanical_Arm_State
* ��������      : ���ƻ�е��״̬
* ��       ��      : u8 state         ��е��״̬
* ��       ��      : ��
*********************************************************************/
void Mechanical_Arm_State(u8 state)
{
    switch (state) {
        case Low:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 0);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 0);           // 00��е������
            break;
        case Stay:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);           // 11��е��̧��
            break;
        case Put1:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 0);           // 10��е��ת��1�ſ�λ
            break;
        case Put2:
            GPIO_WriteBit(GPIOD, GPIO_Pin_10, 0);
            GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);           // 01��е��ת��2�ſ�λ
            break;
        default:
            break;
    }
}

/********************************************************************
* ��������      : Mechanical_Arm_Control
* ��������      : �������ƻ�е��
* ��       ��      : char state           ��е��״̬
*            u32 ms               ��е��״̬�仯����ӳ�
* ��       ��      : ��
*********************************************************************/
u16 MA_1 = 1;                                 // ǯ��״̬��Ĭ�Ͽ�
void Mechanical_Arm_Control(char buffer, u8 state, u32 ms)
{
    if (buffer ==  '1' || state == 1)                        // ��е�۵�һ���˶�����
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
    } else if (buffer ==  '2' || state == 2)                 // ��е�۵ڶ����˶�����
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
    } else if (buffer ==  'A')                 // ǯ�ӿ���
    {
        MA_1 = !MA_1;                         // ǯ��״̬ȡ��
        Plier_Switch(MA_1);
    }else if (buffer ==  'B')                  // ��е������
    {
        Mechanical_Arm_State(Low);
    }else if (buffer ==  'C')                  // ��е��̧��
    {
        Mechanical_Arm_State(Stay);
    }else if (buffer ==  'D')                  // ��е��ת��1�ſ�λ
    {
        Mechanical_Arm_State(Put1);
    }else if (buffer ==  'E')                  // ��е��ת��2�ſ�λ
    {
        Mechanical_Arm_State(Put2);
    }
}
