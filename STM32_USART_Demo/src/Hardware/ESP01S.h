#ifndef __ESP01S_H
#define __ESP01S_H

#include "stm32f10x.h"

#include "WebSocket.h"
#include "Serial.h"
#include "MyDMA.h"

/**
 * @brief 串口通讯初始化
 *
 */
void ESP01S_Init(void);

/**
 * @brief 发送一字节信息
 *
 * @param Byte 字节信息
 */
void ESP01S_SendByte(uint8_t Byte);

/**
 * @brief 发送一个字节数组
 *
 * @param Array 数组指针
 * @param Length 数组长度
 */
void ESP01S_SendArray(uint8_t *Array, uint16_t Length);

/**
 * @brief 发送一个字符串
 *
 * @param String 字符串首地址
 */
void ESP01S_SendString(char *String);

/**
 * @brief 重置缓冲区
 *
 */
void ResetData(void);

/**
 * @brief 发送WS握手信息
 *
 */
void SendWsAccept(void);

/**
 * @brief 发送经纬度信息
 *
 */
void SendLngLat(void);

#endif // !__ESP01S_H
