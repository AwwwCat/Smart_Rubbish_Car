/*******************************************************************
* 文件名称 : main.c
* 作       者 :
* 版       本 :
* 日       期 : 2023/7/14
* 描       述 : 智能垃圾车控制代码
*******************************************************************/
#include "debug.h"// 包含 CH32V307 的头文件，C 标准单元库和delay()函数
#include "lcd.h"
#include "arm.h"
#include "car.h"
#include <string.h>
#include <stdarg.h>

/* 全局定义 */
#define RXBUF_SIZE 1024     // DMA 缓冲器大小
#define size(a)   (sizeof(a) / sizeof(*(a)))
#define TURNDELAY 500       // 旋转延迟
#define MECHINEARMTIME 1000 // 机械臂运动间隔

/* PWM 输出模式定义 */
#define PWM_MODE1   0
#define PWM_MODE2   1

/* 全局数据 */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE]={0};
u16 TURNTIME = 1600;        // 旋转90度需要的延迟
u16 RUNTIME = 2500;         // 行走1米需要的延迟
u16 EACHCHANGE = 100;       // 每次变化的延迟
u16 GLOBALSWITCH = 0;       // 小车控制的全局开关
uint32_t angle = 0;         // 发现垃圾时小车旋转的角度
uint32_t distance = 0;      // 发现垃圾时小车前进的距离



/********************************************************************
* 函数名称      : USARTx_CFG
* 函数功能      : 串口初始化
* 输       入      : 无
* 输       出      : 无
*********************************************************************/
void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    //开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* USART7 TX-->C2  RX-->C3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //RX，输入上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;                    // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;     // 数据位 8
    USART_InitStructure.USART_StopBits = USART_StopBits_1;          // 停止位 1
    USART_InitStructure.USART_Parity = USART_Parity_No;             // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //使能 RX 和 TX

    USART_Init(UART7, &USART_InitStructure);
    DMA_Cmd(DMA2_Channel9, ENABLE);                                  //开启接收 DMA
    USART_Cmd(UART7, ENABLE);                                        //开启UART
}



/********************************************************************
* 函数名称      : DMA_INIT
* 函数功能      : DMA 初始化
* 输       入      : 无
* 输       出      : 无
*********************************************************************/
void DMA_INIT(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    // TX DMA 初始化
    DMA_DeInit(DMA2_Channel8);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART7->DATAR);        // DMA 外设基址，需指向对应的外设
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;                   // DMA 内存基址，指向发送缓冲区的首地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // 方向 : 外设 作为 终点，即 内存 ->  外设
    DMA_InitStructure.DMA_BufferSize = 0;                                   // 缓冲区大小,即要DMA发送的数据长度,目前没有数据可发
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设地址自增，禁用
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存地址自增，启用
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据位宽，8位(Byte)
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 内存数据位宽，8位(Byte)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // 普通模式，发完结束，不循环发送
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // 优先级最高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // M2P,禁用M2M
    DMA_Init(DMA2_Channel8, &DMA_InitStructure);

    // RX DMA 初始化，环形缓冲区自动接收
    DMA_DeInit(DMA2_Channel9);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART7->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // 接收缓冲区
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // 方向 : 外设 作为 源，即 内存 <- 外设
    DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // 缓冲区长度为 RXBUF_SIZE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // 循环模式，构成环形缓冲区
    DMA_Init(DMA2_Channel9, &DMA_InitStructure);

}



/********************************************************************
* 函数名称      : uartWriteBLE
* 函数功能      : 向蓝牙模组发送数据
* 输       入      : char * data          要发送的数据的首地址
*            uint16_t num         数据长度
* 输       出      : RESET                发送失败
*            SET                  发送成功
*********************************************************************/
FlagStatus uartWriteBLE(char * data , uint16_t num)
{
    //如上次发送未完成，返回
    if(DMA_GetCurrDataCounter(DMA2_Channel8) != 0){
        return RESET;
    }

    DMA_ClearFlag(DMA2_FLAG_TC8);
    DMA_Cmd(DMA2_Channel8, DISABLE );           // 关 DMA 后操作
    DMA2_Channel8->MADDR = (uint32_t)data;      // 发送缓冲区为 data
    DMA_SetCurrDataCounter(DMA2_Channel8,num);  // 设置缓冲区长度
    DMA_Cmd(DMA2_Channel8, ENABLE);             // 开 DMA
    return SET;
}



/********************************************************************
* 函数名称      : uartWriteBLEstr
* 函数功能      : 向蓝牙模组发送字符串
* 输       入      : char * str          要发送的数据的首地址
* 输       出      : RESET               发送失败
*            SET                 发送成功
*********************************************************************/
FlagStatus uartWriteBLEstr(char * str)
{
    uint16_t num = 0;
    while(str[num])num++;           // 计算字符串长度
    return uartWriteBLE(str,num);
}



/********************************************************************
* 函数名称      : uartReadBLE
* 函数功能      : 从接收缓冲区读出一组数据
* 输       入      : char * buffer        用来存放读出数据的地址
*            uint16_t num         要读的字节数
* 输       出      : int                  返回实际读出的字节数
*********************************************************************/
uint16_t rxBufferReadPos = 0;       //接收缓冲区读指针
uint32_t uartReadBLE(char * buffer , uint16_t num)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9); //计算 DMA 数据尾的位置
    uint16_t i = 0;

    if (rxBufferReadPos == rxBufferEnd){
        // 无数据，返回
        return 0;
    }

    while (rxBufferReadPos!=rxBufferEnd && i < num){
        buffer[i] = RxBuffer[rxBufferReadPos];
        i++;
        rxBufferReadPos++;
        if(rxBufferReadPos >= RXBUF_SIZE){
            // 超出缓冲区，回零
            rxBufferReadPos = 0;
        }
    }
    return i;
}



/********************************************************************
* 函数名称      : uartReadByteBLE
* 函数功能      : 从接收缓冲区读出 1 字节数据
* 输       入      : 无
* 输       出      : char    返回读出的数据(无数据也返回0)
*********************************************************************/
char uartReadByteBLE()
{
    char ret;
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9);//计算 DMA 数据尾的位置
    if (rxBufferReadPos == rxBufferEnd){
        // 无数据，返回
        return 0;
    }
    ret = RxBuffer[rxBufferReadPos];
    rxBufferReadPos++;
    if(rxBufferReadPos >= RXBUF_SIZE){
        // 超出缓冲区，回零
        rxBufferReadPos = 0;
    }
    return ret;
}



/********************************************************************
* 函数名称      : uartAvailableBLE
* 函数功能      : 获取缓冲区中可读数据的数量
* 输       入      : 无
* 输       出      : uint16_t    返回可读数据数量
*********************************************************************/
uint16_t uartAvailableBLE()
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel9);//计算 DMA 数据尾的位置
    // 计算可读字节
    if (rxBufferReadPos <= rxBufferEnd){
        return rxBufferEnd - rxBufferReadPos;
    }else{
        return rxBufferEnd +RXBUF_SIZE -rxBufferReadPos;
    }
}



/********************************************************************
* 函数名称      : str_Package
* 函数功能      : 把可变参数表中的数据转成字符存到buffer_s中
* 输       入      : char* format         需要封装进buffer_s的字符串
* 输       出      : 无
*********************************************************************/
char buffer_s[100];
void str_Package(char* format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    vsnprintf(buffer_s, 100, format, vArgList);         // 把可变参数表中的数据转成字符存到buffer_s中，每个参数间用','隔开
    va_end(vArgList);
}



/********************************************************************
* 函数名称      : High_Level_Measure
* 函数功能      : 测量接收高电平的时间
* 输       入      : 无
* 输       出      : t             高电平的时长
*********************************************************************/
int32_t High_Level_Measure(void)
{
    uint32_t t = 0;
    while (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))     // 测量高电平的时间
    {
        t++;
        delay_us(10);
    }
    return t;
}



/********************************************************************
* 函数名称      : Angle_Measure
* 函数功能      : 接收高电平的时间并转换为角度
* 输       入      : 无
* 输       出      : angle        角度
*********************************************************************/
int32_t Angle_Measure(void)
{

    uint32_t angle = High_Level_Measure();
    if (angle == 0)                                             // 若无信号则输出0
        return 0;
    else
        return angle - 180;                                     // 发现垃圾时小车旋转的角度，t=0 为 -180度
}



/********************************************************************
* 函数名称      : Distance_Measure
* 函数功能      : 接收高电平的时间并转换为距离
* 输       入      : 无
* 输       出      : distance     距离
*********************************************************************/
int32_t Distance_Measure(void)
{
    uint32_t distance = High_Level_Measure();                   // 发现垃圾时小车前进的距离
    return distance;
}



/********************************************************************
* 函数名称      : Refuse_Classification
* 函数功能      : 垃圾分类
* 输       入      : 无
* 输       出      : sort         分类
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
* 函数名称      : EXTI_INT_INIT
* 函数功能      : 初始化外部中断
* 输       入      : 无
* 输       出      : 无
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
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //按为高电平，用上升沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        //子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line=EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //按为高电平，用下降沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        //子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}



/********************************************************************
* 函数名称      : GPIO_INIT
* 函数功能      : GPIO 初始化
* 输       入      : 无
* 输       出      : 无
*********************************************************************/
void GPIO_INIT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);       // B0, B1浮空输入
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

    GPIO_WriteBit(GPIOA, GPIO_Pin_1, 1);        // 引脚状态初始化为高电平
    GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1);
    GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1);
}



/********************************************************************
* 函数名称      : Turntime_Change
* 函数功能      : 通过蓝牙调整延迟进行旋转速度测试
* 输       入      : char buffer          要发送的数据的首地址
* 输       出      : 无
*********************************************************************/
void Turntime_Change(char buffer)
{
    if (buffer == 'A')                // Addition 旋转时间加上变化
    {
        TURNTIME += EACHCHANGE;
    } else if (buffer == 'S')         // Subtraction 旋转时间减去变化
    {
        if (TURNTIME - EACHCHANGE > 0)
            TURNTIME -= EACHCHANGE;
    } else if (buffer == 'M')         // Multiplication 变化乘以10
    {
        if (EACHCHANGE < 10000)
            EACHCHANGE *= 10;
    } else if (buffer == 'D')         // Division 变化除以10
    {
        if (EACHCHANGE > 1)
            EACHCHANGE /= 10;
    } else if (buffer == 'T')         // Test 测试小车一定延迟旋转角度
    {
        Car_Turn_Left(TURNDELAY);
        Delay_Ms(TURNTIME);
        Car_Stop();
    }
}



/********************************************************************
* 函数名称      : Turn_Angle
* 函数功能      : 小车转固定角度
* 输       入      : angle            角度，单位度（°）
* 输       出      : 无
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
* 函数名称      : Run_Distance
* 函数功能      : 小车行走固定距离
* 输       入      : distance         距离，单位厘米（cm）
* 输       出      : 无
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
        Delay_Ms(distance * RUNTIME / 100);
    }
    Car_Stop();
}



/********************************************************************
* 函数名称      : Globalswitch_Change
* 函数功能      : 改变
* 输       入      : char state
* 输       出      : 无
*********************************************************************/
void Globalswitch_Change(char state)
{
    if (state == 'I')                         // 蓝牙控制全局开关
    {
        GLOBALSWITCH = 1;
    } else if (state == 'O')
    {
        GLOBALSWITCH = 0;
    }
}



/********************************************************************
* 函数名称      : main
* 函数功能      : 主程序
* 输       入      : 无
* 输       出      : 无
*********************************************************************/
int main(void)
{
    /**********************初始化**********************/
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

    GPIO_WriteBit(GPIOC, GPIO_Pin_13,SET);              // 蓝牙使能
    /********************初始化结束**********************/

    Delay_Ms(1000);

    while (1){
        Delay_Ms(400);
        lcd_set_color(BLACK,RED);
        lcd_show_string(30, 32, 16, "TURNTIME   : %8d", TURNTIME);
        lcd_show_string(30, 48, 16, "Each Change: %8d", EACHCHANGE);
        int num = uartAvailableBLE();
        char buffer_r[1024]={"\0"};                     // 读取蓝牙字符串
        if (num > 0 ){
            uartReadBLE(buffer_r , num);
            lcd_set_color(BLACK,GREEN);                 // 将读取的字符串显示在屏幕上
            lcd_show_string(30, 208, 16,"Receive    : %8s", buffer_r);
            printf("Received:%s\r\n",buffer_r);         // 将收到的字符串发送给串口
        }

        Globalswitch_Change(buffer_r[1]);


        if (GLOBALSWITCH)
        {
            Car_Front_Run(TURNDELAY);
            u8 pick = High_Level_Measure();                 // 获取高电平，若有垃圾则则接收到高电平，反之为0
            if (pick != 0)                                  // 若不为0，则有垃圾
            {
                Echo_Switch(Close);
                u16 angle = 0;
                while (!angle)                              // 当角度为零，即未接收到角度信号时，再次接收
                    angle = Angle_Measure();
                Turn_Angle(angle);
                u16 distance = 0;
                while (!distance)                           // 当距离为零，即未接收到距离信号时，再次接收
                    distance = Distance_Measure();
                u8 refuse = Refuse_Classification();        // 获取分类信息
                switch (refuse)                             // 将字符串封装进buffer_s
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
                uartWriteBLEstr(buffer_s);                   //将字符串穿给蓝牙
                Run_Distance(distance);
                Mechanical_Arm_Control('0', refuse, MECHINEARMTIME);
                Turn_Angle(-angle - 90);
                Echo_Switch(Open);
                Car_Front_Run(TURNDELAY);
            }

//            Car_Control(buffer_r[2]);
//            Mechanical_Arm_Control(buffer_r[3], 0, MECHINEARMTIME);
//            Turntime_Change(buffer_r[4]);
        }
    }
}



/********************************************************************
* 函数名称      : EXTI3_IRQHandler
* 函数功能      : 中断服务程序的函数
* 输       入      : 无
* 输       出      : 无
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
    EXTI_ClearFlag(EXTI_Line3); // 置中断标志位为零
}



/********************************************************************
* 函数名称      : EXTI15_10_IRQHandler
* 函数功能      : 中断服务程序的函数
* 输       入      : 无
* 输       出      : 无
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

    EXTI_ClearFlag(EXTI_Line15); // 置中断标志位为零
}

