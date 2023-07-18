/*******************************************************************
* �ļ����� : main.c
* ��       �� :
* ��       �� :
* ��       �� : 2023/7/14
* ��       �� : �������������ƴ���
*******************************************************************/
#include "debug.h"// ���� CH32V307 ��ͷ�ļ���C ��׼��Ԫ���delay()����
#include "lcd.h"
#include <string.h>
#include <stdarg.h>

/* ȫ�ֶ��� */
#define RXBUF_SIZE 1024     // DMA ��������С
#define size(a)   (sizeof(a) / sizeof(*(a)))
#define TURNDELAY 500       // ��ת�ӳ�
#define MECHINEARMTIME 1000 // ��е���˶����


/* ǯ�ӿ���ģʽö�� */
enum PLIER {Open, Close} Plier;

/* ��е��״̬ö�� */
enum ARM {Low, Stay, Put1, Put2} Arm;

/* PWM ���ģʽ���� */
#define PWM_MODE1   0
#define PWM_MODE2   1

/* ȫ������ */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE]={0};
u16 TURNTIME = 1250;        // ��ת90����Ҫ���ӳ�
u16 RUNTIME = 2500;         // ����1����Ҫ���ӳ�
u16 EACHCHANGE = 100;       // ÿ�α仯���ӳ�
u16 GLOBALSWITCH = 0;       // С�����Ƶ�ȫ�ֿ���
uint32_t angle = 0;         // ��������ʱС����ת�ĽǶ�
uint32_t distance = 0;      // ��������ʱС��ǰ���ľ���



/********************************************************************
* ��������      : USARTx_CFG
* ��������      : ���ڳ�ʼ��
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    //����ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* USART7 TX-->C2  RX-->C3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //RX����������
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;                    // ������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // ����λ 8
    USART_InitStructure.USART_StopBits = USART_StopBits_1;          // ֹͣλ 1
    USART_InitStructure.USART_Parity = USART_Parity_No;             // ��У��
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //ʹ�� RX �� TX

    USART_Init(UART7, &USART_InitStructure);
    DMA_Cmd(DMA2_Channel9, ENABLE);                                  //�������� DMA
    USART_Cmd(UART7, ENABLE);                                        //����UART
}



/********************************************************************
* ��������      : DMA_INIT
* ��������      : DMA ��ʼ��
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void DMA_INIT(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    // TX DMA ��ʼ��
    DMA_DeInit(DMA2_Channel8);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART7->DATAR);        // DMA �����ַ����ָ���Ӧ������
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;                   // DMA �ڴ��ַ��ָ���ͻ��������׵�ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���� : ���� ��Ϊ �յ㣬�� �ڴ� ->  ����
    DMA_InitStructure.DMA_BufferSize = 0;                                   // ��������С,��ҪDMA���͵����ݳ���,Ŀǰû�����ݿɷ�
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ����������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����������
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // ��������λ��8λ(Byte)
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ�����λ��8λ(Byte)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ��ͨģʽ�������������ѭ������
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ����
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // M2P,����M2M
    DMA_Init(DMA2_Channel8, &DMA_InitStructure);

    // RX DMA ��ʼ�������λ������Զ�����
    DMA_DeInit(DMA2_Channel9);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART7->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // ���ջ�����
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // ���� : ���� ��Ϊ Դ���� �ڴ� <- ����
    DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // ����������Ϊ RXBUF_SIZE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // ѭ��ģʽ�����ɻ��λ�����
    DMA_Init(DMA2_Channel9, &DMA_InitStructure);

}



/********************************************************************
* ��������      : uartWriteBLE
* ��������      : ������ģ�鷢������
* ��       ��      : char * data          Ҫ���͵����ݵ��׵�ַ
*            uint16_t num         ���ݳ���
* ��       ��      : RESET                ����ʧ��
*            SET                  ���ͳɹ�
*********************************************************************/
FlagStatus uartWriteBLE(char * data , uint16_t num)
{
    //���ϴη���δ��ɣ�����
    if(DMA_GetCurrDataCounter(DMA2_Channel8) != 0){
        return RESET;
    }

    DMA_ClearFlag(DMA2_FLAG_TC8);
    DMA_Cmd(DMA2_Channel8, DISABLE );           // �� DMA �����
    DMA2_Channel8->MADDR = (uint32_t)data;      // ���ͻ�����Ϊ data
    DMA_SetCurrDataCounter(DMA2_Channel8,num);  // ���û���������
    DMA_Cmd(DMA2_Channel8, ENABLE);             // �� DMA
    return SET;
}



/********************************************************************
* ��������      : uartWriteBLEstr
* ��������      : ������ģ�鷢���ַ���
* ��       ��      : char * str          Ҫ���͵����ݵ��׵�ַ
* ��       ��      : RESET               ����ʧ��
*            SET                 ���ͳɹ�
*********************************************************************/
FlagStatus uartWriteBLEstr(char * str)
{
    uint16_t num = 0;
    while(str[num])num++;           // �����ַ�������
    return uartWriteBLE(str,num);
}



/********************************************************************
* ��������      : uartReadBLE
* ��������      : �ӽ��ջ���������һ������
* ��       ��      : char * buffer        ������Ŷ������ݵĵ�ַ
*            uint16_t num         Ҫ�����ֽ���
* ��       ��      : int                  ����ʵ�ʶ������ֽ���
*********************************************************************/
uint16_t rxBufferReadPos = 0;       //���ջ�������ָ��
uint32_t uartReadBLE(char * buffer , uint16_t num)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9); //���� DMA ����β��λ��
    uint16_t i = 0;

    if (rxBufferReadPos == rxBufferEnd){
        // �����ݣ�����
        return 0;
    }

    while (rxBufferReadPos!=rxBufferEnd && i < num){
        buffer[i] = RxBuffer[rxBufferReadPos];
        i++;
        rxBufferReadPos++;
        if(rxBufferReadPos >= RXBUF_SIZE){
            // ����������������
            rxBufferReadPos = 0;
        }
    }
    return i;
}



/********************************************************************
* ��������      : uartReadByteBLE
* ��������      : �ӽ��ջ��������� 1 �ֽ�����
* ��       ��      : ��
* ��       ��      : char    ���ض���������(������Ҳ����0)
*********************************************************************/
char uartReadByteBLE()
{
    char ret;
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9);//���� DMA ����β��λ��
    if (rxBufferReadPos == rxBufferEnd){
        // �����ݣ�����
        return 0;
    }
    ret = RxBuffer[rxBufferReadPos];
    rxBufferReadPos++;
    if(rxBufferReadPos >= RXBUF_SIZE){
        // ����������������
        rxBufferReadPos = 0;
    }
    return ret;
}



/********************************************************************
* ��������      : uartAvailableBLE
* ��������      : ��ȡ�������пɶ����ݵ�����
* ��       ��      : ��
* ��       ��      : uint16_t    ���ؿɶ���������
*********************************************************************/
uint16_t uartAvailableBLE()
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9);//���� DMA ����β��λ��
    // ����ɶ��ֽ�
    if (rxBufferReadPos <= rxBufferEnd){
        return rxBufferEnd - rxBufferReadPos;
    }else{
        return rxBufferEnd +RXBUF_SIZE -rxBufferReadPos;
    }
}



/********************************************************************
* ��������      : str_Package
* ��������      : �ѿɱ�������е�����ת���ַ��浽buffer_s��
* ��       ��      : char* format         ��Ҫ��װ��buffer_s���ַ���
* ��       ��      : ��
*********************************************************************/
char buffer_s[100];
void str_Package(char* format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    vsnprintf(buffer_s, 100, format, vArgList);         // �ѿɱ�������е�����ת���ַ��浽buffer_s�У�ÿ����������','����
    va_end(vArgList);
}



/********************************************************************
* ��������      : High_Level_Measure
* ��������      : �������ոߵ�ƽ��ʱ��
* ��       ��      : ��
* ��       ��      : t             �ߵ�ƽ��ʱ��
*********************************************************************/
int32_t High_Level_Measure(void)
{
    uint32_t t = 0;
    while (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))     // �����ߵ�ƽ��ʱ��
    {
        t++;
        delay_us(10);
    }
    return t;
}



/********************************************************************
* ��������      : Angle_Measure
* ��������      : ���ոߵ�ƽ��ʱ�䲢ת��Ϊ�Ƕ�
* ��       ��      : ��
* ��       ��      : angle        �Ƕ�
*********************************************************************/
int32_t Angle_Measure(void)
{

    uint32_t angle = High_Level_Measure();
    if (angle == 0)                                             // �����ź������0
        return 0;
    else
        return angle - 180;                                     // ��������ʱС����ת�ĽǶȣ�t=0 Ϊ -180��
}



/********************************************************************
* ��������      : Distance_Measure
* ��������      : ���ոߵ�ƽ��ʱ�䲢ת��Ϊ����
* ��       ��      : ��
* ��       ��      : distance     ����
*********************************************************************/
int32_t Distance_Measure(void)
{
    uint32_t distance = High_Level_Measure();                   // ��������ʱС��ǰ���ľ���
    return distance;
}



/********************************************************************
* ��������      : Refuse_Classification
* ��������      : ��������
* ��       ��      : ��
* ��       ��      : sort         ����
*********************************************************************/
int32_t Refuse_Classification(void)
{
    u8 sort = 0;
    if (!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1))
        sort = 1;
    else
        sort = 2;
    return sort;
}



/********************************************************************
* ��������      : EXTI_INT_INIT
* ��������      : ��ʼ���ⲿ�ж�
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void EXTI_INT_INIT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure={0};
    EXTI_InitTypeDef EXTI_InitStructure={0};
    NVIC_InitTypeDef NVIC_InitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line=EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //��Ϊ�ߵ�ƽ����������
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        //�����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line=EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //��Ϊ�ߵ�ƽ�����½���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        //�����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}



/********************************************************************
* ��������      : GPIO_INIT
* ��������      : GPIO ��ʼ��
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void GPIO_INIT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);       // B0, B1��������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_WriteBit(GPIOA, GPIO_Pin_1, 1);        // ����״̬��ʼ��Ϊ�ߵ�ƽ
    GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
    GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);
}



/********************************************************************
* ��������      : Car_Run
* ��������      : С��ֹͣ
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void Car_Run()
{
    GPIO_WriteBit(GPIOE, GPIO_Pin_12, 0);       // ����
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, 1);
}



/********************************************************************
* ��������      : Car_Stop
* ��������      : С��ֹͣ
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void Car_Stop()
{
    GPIO_WriteBit(GPIOE, GPIO_Pin_12, 1);       // ����
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, 0);
}



/********************************************************************
* ��������      : Car_Front_Run
* ��������      : С��ǰ��
* ��       ��      : u32 ms           �źű仯��С���˶�����ӳ�
* ��       ��      : ��
*********************************************************************/
void Car_Front_Run(u32 ms)
{
    Car_Stop();
    Delay_Ms(ms);
    GPIO_WriteBit(GPIOE, GPIO_Pin_13, 0);
    GPIO_WriteBit(GPIOE, GPIO_Pin_14, 0);       // 00Ϊǰ���ź�
    Delay_Ms(ms);
    Car_Run();
}



/********************************************************************
* ��������      : Car_Back_Run
* ��������      : С������
* ��       ��      : u32 ms           �źű仯��С���˶�����ӳ�
* ��       ��      : ��
*********************************************************************/
void Car_Back_Run(u32 ms)
{
    Car_Stop();
    Delay_Ms(ms);
    GPIO_WriteBit(GPIOE, GPIO_Pin_13, 0);
    GPIO_WriteBit(GPIOE, GPIO_Pin_14, 1);       // 01Ϊ�����ź�
    Delay_Ms(ms);
    Car_Run();
}



/********************************************************************
* ��������      : Car_Turn_Right
* ��������      : С��ԭ����ת
* ��       ��      : u32 ms           �źű仯��С���˶�����ӳ�
* ��       ��      : ��
*********************************************************************/
void Car_Turn_Right(u32 ms)
{
    Car_Stop();
    Delay_Ms(ms);
    GPIO_WriteBit(GPIOE, GPIO_Pin_13, 1);
    GPIO_WriteBit(GPIOE, GPIO_Pin_14, 0);       // 10Ϊ��ת�ź�
    Delay_Ms(ms);
    Car_Run();
}



/********************************************************************
* ��������      : Car_Turn_Left
* ��������      : С��ԭ����ת
* ��       ��      : u32 ms           �źű仯��С���˶�����ӳ�
* ��       ��      : ��
*********************************************************************/
void Car_Turn_Left(u32 ms)
{
    Car_Stop();
    Delay_Ms(ms);
    GPIO_WriteBit(GPIOE, GPIO_Pin_13, 1);
    GPIO_WriteBit(GPIOE, GPIO_Pin_14, 1);       // 11Ϊ��ת�ź�
    Delay_Ms(ms);
    Car_Run();
}



/********************************************************************
* ��������      : Echo_Switch
* ��������      : С��ԭ����ת
* ��       ��      : u8 mode          ���������ģ�鿪��
* ��       ��      : ��
*********************************************************************/
void Echo_Switch(u8 mode)
{
    GPIO_WriteBit(GPIOD, GPIO_Pin_9, mode);
}



/********************************************************************
* ��������      : Plier_Switch
* ��������      : ���ƻ�еǯ����
* ��       ��      : u8 mode          ǯ�ӿ���ģʽ
* ��       ��      : ��
*********************************************************************/
void Plier_Switch(u8 mode)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, mode);
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



/********************************************************************
* ��������      : Car_Control
* ��������      : ��������С��
* ��       ��      : char buffer              Ҫ���͵�����
* ��       ��      : ��
*********************************************************************/
void Car_Control(char state)
{
    if (state == 'F')                     //����ǰ���ź�ʱС��ǰ��
    {
        Car_Front_Run(TURNDELAY);
    } else if (state == 'B')              //���ܺ����ź�ʱС������
    {
        Car_Back_Run(TURNDELAY);
    } else if (state == 'L')              //������ת�ź�ʱС����ת
    {
        Car_Turn_Left(TURNDELAY);
    } else if (state == 'R')              //������ת�ź�ʱС����ת
    {
        Car_Turn_Right(TURNDELAY);
    } else if (state == 'S')              //����ֹͣ�ź�ʱС��ֹͣ
    {
        Car_Stop();
    }
}



/********************************************************************
* ��������      : Turntime_Change
* ��������      : ͨ�����������ӳٽ�����ת�ٶȲ���
* ��       ��      : char buffer          Ҫ���͵����ݵ��׵�ַ
* ��       ��      : ��
*********************************************************************/
void Turntime_Change(char buffer)
{
    if (buffer == 'A')                // Addition ��תʱ����ϱ仯
    {
        TURNTIME += EACHCHANGE;
    } else if (buffer == 'S')         // Subtraction ��תʱ���ȥ�仯
    {
        if (TURNTIME - EACHCHANGE > 0)
            TURNTIME -= EACHCHANGE;
    } else if (buffer == 'M')         // Multiplication �仯����10
    {
        if (EACHCHANGE < 10000)
            EACHCHANGE *= 10;
    } else if (buffer == 'D')         // Division �仯����10
    {
        if (EACHCHANGE > 1)
            EACHCHANGE /= 10;
    } else if (buffer == 'T')         // Test ����С��һ���ӳ���ת�Ƕ�
    {
        Car_Turn_Left(TURNDELAY);
        Delay_Ms(TURNTIME);
        Car_Stop();
    }
}



/********************************************************************
* ��������      : Turn_Angle
* ��������      : С��ת�̶��Ƕ�
* ��       ��      : angle            �Ƕȣ���λ�ȣ��㣩
* ��       ��      : ��
*********************************************************************/
void Turn_Angle(u16 angle)
{
    if (angle < 0)
    {
        Car_Turn_Left(TURNDELAY);
        Delay_Ms((-angle) * TURNTIME / 90);
    }
    else if (angle > 0)
    {
        Car_Turn_Right(TURNDELAY);
        Delay_Ms(angle * TURNTIME / 90);
    }
    Car_Stop();
}



/********************************************************************
* ��������      : Run_Distance
* ��������      : С�����߹̶�����
* ��       ��      : distance         ���룬��λ���ף�cm��
* ��       ��      : ��
*********************************************************************/
void Run_Distance(u16 distance)
{
    if (distance < 0)
    {
        Car_Back_Run(TURNDELAY);
        Delay_Ms((-distance) * RUNTIME / 100);
    }
    else if (distance > 0)
    {
        Car_Front_Run(TURNDELAY);
        Delay_Ms(-distance * RUNTIME / 100);
    }
    Car_Stop();
}



/********************************************************************
* ��������      : Globalswitch_Change
* ��������      : �ı�
* ��       ��      : char state
* ��       ��      : ��
*********************************************************************/
void Globalswitch_Change(char state)
{
    if (state == 'I')                         // ��������ȫ�ֿ���
    {
        GLOBALSWITCH = 1;
    } else if (state == 'O')
    {
        GLOBALSWITCH = 0;
    }
}



/********************************************************************
* ��������      : main
* ��������      : ������
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
int main(void)
{
    /**********************��ʼ��**********************/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n",SystemCoreClock);

    DMA_INIT();
    USARTx_CFG();
    USART_DMACmd(UART7,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);

    lcd_init();
    lcd_set_color(BLACK,WHITE);
    lcd_show_string(50, 0, 32,"Smart Car");

    delay_ms(100);

    EXTI_INT_INIT();
    GPIO_INIT();

    GPIO_WriteBit(GPIOC, GPIO_Pin_13,SET);              // ����ʹ��
    /********************��ʼ������**********************/

    Delay_Ms(1000);

    while (1){
        Delay_Ms(400);
        lcd_set_color(BLACK,RED);
        lcd_show_string(30, 32, 16, "TURNTIME   : %8d", TURNTIME);
        lcd_show_string(30, 48, 16, "Each Change: %8d", EACHCHANGE);
        int num = uartAvailableBLE();
        char buffer_r[1024]={"\0"};                     // ��ȡ�����ַ���
        if (num > 0 ){
            uartReadBLE(buffer_r , num);
            lcd_set_color(BLACK,GREEN);                 // ����ȡ���ַ�����ʾ����Ļ��
            lcd_show_string(30, 208, 16,"Receive    : %8s", buffer_r);
            printf("Received:%s\r\n",buffer_r);         // ���յ����ַ������͸�����
        }

        u16 angle = Angle_Measure();                    // ��ȡ�Ƕȣ������������нǶȣ���֮Ϊ0
        if (angle != 0)                                 // ����Ϊ0����������
        {
            Echo_Switch(Close);
            Turn_Angle(angle);
            u16 distance = 0;
            while (!distance)                           // ������Ϊ�㣬��δ���յ������ź�ʱ���ٴν���
                distance = Distance_Measure();
            u8 refuse = Refuse_Classification();        // ��ȡ������Ϣ
            switch (refuse)                             // ���ַ�����װ��buffer_s
            {
                case 1:
                    str_Package("Classification:    red\r\n");
                    break;
                case 2:
                    str_Package("Classification:  green\r\n");
                    break;
                default:
                    str_Package("Classification:  other\r\n");
                    break;
            }
            uartWriteBLEstr(buffer_s);                   //���ַ�����������
            Run_Distance(distance);
            Mechanical_Arm_Control('0', refuse, MECHINEARMTIME);
            Turn_Angle(-angle - 90);
            Echo_Switch(Open);
            Car_Front_Run(TURNDELAY);
        }

        Globalswitch_Change(buffer_r[1]);

        if (GLOBALSWITCH)
        {
            Car_Control(buffer_r[2]);
            Mechanical_Arm_Control(buffer_r[3], 0, MECHINEARMTIME);
            Turntime_Change(buffer_r[4]);
        }
    }
}



/********************************************************************
* ��������      : EXTI3_IRQHandler
* ��������      : �жϷ������ĺ���
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void)
{
    if (GLOBALSWITCH)
    {
        Echo_Switch(Close);
        Car_Turn_Left(TURNDELAY);
        Delay_Ms(TURNTIME);
        Car_Front_Run(TURNDELAY);
        Echo_Switch(Open);
    }
    EXTI_ClearFlag(EXTI_Line3); // ���жϱ�־λΪ��
}



/********************************************************************
* ��������      : EXTI15_10_IRQHandler
* ��������      : �жϷ������ĺ���
* ��       ��      : ��
* ��       ��      : ��
*********************************************************************/
void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void)
{
    if (GLOBALSWITCH)
    {
        Echo_Switch(Close);
        Car_Turn_Right(TURNDELAY);
        Delay_Ms(TURNTIME);
        Car_Front_Run(TURNDELAY);
        Echo_Switch(Open);
    }

    EXTI_ClearFlag(EXTI_Line15); // ���жϱ�־λΪ��
}

