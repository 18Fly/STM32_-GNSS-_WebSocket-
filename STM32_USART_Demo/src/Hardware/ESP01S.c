#include "ESP01S.h"

uint8_t ESP01S_Receive_Temp[1024];
uint8_t Token[24];
uint8_t Receive_Index;
boolean FinishSign;
boolean Send_Flag;
uint8_t SendBuf[129] = {0};
uint8_t SendBuf2[24] = {0x81, 0x16};

extern Serial_RxData RxData;
extern uint8_t SendCount;

void ESP01S_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void ESP01S_SendByte(uint8_t Byte)
{
    uint8_t timeCount = 0;
    USART_SendData(USART1, Byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET && timeCount++ < 0xFF);
}

void ESP01S_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i, timeCount = 0;
    for (i = 0; i < Length; i++) {
        ESP01S_SendByte(Array[i]);
    }

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET && timeCount++ < 0xff);
}

void ESP01S_SendString(char *String)
{
    unsigned int k = 0, timeCount = 0;
    do {
        ESP01S_SendByte(String[k++]);
    } while (String[k] != 0);

    /* 等待发送完成 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET && timeCount++ < 0xff);
}

void ResetData(void)
{
    ESP01S_Receive_Temp[Receive_Index] = 0;
    while (Receive_Index > 0) {
        ESP01S_Receive_Temp[--Receive_Index] = 0;
    }
}

void SendWsAccept(void)
{
    if (FinishSign) {
        memset(SendBuf, 0, 129);
        repeatWSRaw((char *)Token, (char *)SendBuf);
        // Delay_ms(350);
        ESP01S_SendString("AT+CIPSEND=0,129\r\n");
        Delay_us(500);
        ESP01S_SendArray(SendBuf, 129);
        // Delay_ms(300);
        Send_Flag  = true;
        FinishSign = false;
        // USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }
}

void SendLngLat(void)
{
    if (Send_Flag && SendCount > 2) {
        memset((char *)SendBuf2 + 2, 0, 22);
        strcat((char *)SendBuf2, (char *)RxData.Latitude);
        strcat((char *)SendBuf2, ",");
        strcat((char *)SendBuf2, (char *)RxData.Longitude);
        // Delay_ms(350);
        ESP01S_SendString("AT+CIPSEND=0,24\r\n");
        Delay_us(500);
        ESP01S_SendArray(SendBuf2, 24);
        SendCount = 0;
        // Delay_ms(300);
    }
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
        ESP01S_Receive_Temp[Receive_Index++] = USART_ReceiveData(USART1);

        if (ESP01S_Receive_Temp[Receive_Index - 1] == ' ' && ESP01S_Receive_Temp[Receive_Index - 2] == ':' && ESP01S_Receive_Temp[Receive_Index - 3] == 'y' && ESP01S_Receive_Temp[Receive_Index - 4] == 'e' && ESP01S_Receive_Temp[Receive_Index - 5] == 'K') {
            ResetData();
        }

        if (ESP01S_Receive_Temp[Receive_Index - 1] == '\r' && ESP01S_Receive_Temp[Receive_Index - 2] == '=') {
            MyDMA_Init((uint32_t)ESP01S_Receive_Temp, (uint32_t)Token, 24);
            MyDMA_Transfer();
            FinishSign = true;
            ResetData();
        }

        if (Receive_Index > 200) {
            ResetData();
        }

        USART_ClearITPendingBit(USART1, USART_FLAG_RXNE);
        USART_ClearFlag(USART1, USART_FLAG_RXNE);
    }

    if (USART_GetFlagStatus(USART1, USART_IT_ORE) != RESET)
    // 需要用USART_GetFlagStatus函数来检查ORE溢出中断
    {
        USART_ClearFlag(USART1, USART_FLAG_ORE); // 清除ORE标志位
        USART_ReceiveData(USART1);               // 抛弃接收到的数据
    }
}
