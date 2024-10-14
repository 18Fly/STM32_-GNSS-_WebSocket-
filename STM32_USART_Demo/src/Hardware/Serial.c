#include "Serial.h"

Serial_RxData RxData = {0};

struct ReceiveBuffer Buffer = {0};

uint8_t Receive_Temp;

void Serial_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 9600;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
    uint8_t timeCount = 0;
    USART_SendData(USART3, Byte);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET && timeCount++ < 0xFF);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i, timeCount = 0;
    for (i = 0; i < Length; i++) {
        Serial_SendByte(Array[i]);
    }

    /* 等待发送完成 */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET && timeCount++ < 0xff);
}

void Serial_SendString(char *String)
{
    uint8_t timeCount = 0;
    while (*String != 0) {
        Serial_SendByte(*String++);
    }
    /* 等待发送完成 */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET && timeCount++ < 0xff);
}

uint8_t Serial_GetSmallNum(uint32_t Base, uint8_t Index)
{
    uint16_t tmp = 10;
    while (--Index) {
        tmp *= 10;
    }
    return Base / tmp % 10;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    while (Length--) {
        Serial_SendByte(Serial_GetSmallNum(Number, Length) + '0');
    }
}

int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);
    return ch;
}

void Serial_SPrintf(char *format, ...)
{
    char String[100];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(String, format, argptr);
    va_end(argptr);
    Serial_SendString(String);
}

void ResetRxData(void)
{
    while (--Buffer.RxIndex) {
        Buffer.RxBuffer[Buffer.RxIndex] = 0;
    }
    Buffer.RxBuffer[Buffer.RxIndex] = 0;
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        Receive_Temp = USART_ReceiveData(USART3);

        if (Receive_Temp == '$') {
            Buffer.IFUpdate = true;
        }
        if (Receive_Temp == '\r') {
            Buffer.IFUpdate                 = false;
            Buffer.RxBuffer[Buffer.RxIndex] = 0;
            RxData.receiveFlag              = true;
        }
        if (Buffer.IFUpdate) {
            Buffer.RxBuffer[Buffer.RxIndex++] = Receive_Temp;
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        USART_ClearFlag(USART3, USART_FLAG_RXNE);
    }
    if (USART_GetFlagStatus(USART3, USART_IT_ORE) != RESET)
    // 需要用USART_GetFlagStatus函数来检查ORE溢出中断
    {
        USART_ClearFlag(USART3, USART_FLAG_ORE); // 清除ORE标志位
        USART_ReceiveData(USART3);               // 抛弃接收到的数据
    }
}
