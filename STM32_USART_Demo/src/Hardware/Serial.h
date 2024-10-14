#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include "Delay.h"

#define boolean unsigned char
#define true    1
#define false   0

typedef struct RxData {
    char Longitude[12]; // 经度
    char Latitude[12];  // 纬度
    uint8_t receiveFlag;
} Serial_RxData;

struct ReceiveBuffer {
    uint8_t RxBuffer[128]; // 接收缓冲区
    uint8_t RxIndex;       // 接收缓冲区索引
    boolean IFUpdate;      // 是否继续接收数据
};

/**
 * @brief 串口通讯初始化
 *
 */
void Serial_Init(void);

/**
 * @brief 发送一字节信息
 *
 * @param Byte 字节信息
 */
void Serial_SendByte(uint8_t Byte);

/**
 * @brief 发送一个字节数组
 *
 * @param Array 数组指针
 * @param Length 数组长度
 */
void Serial_SendArray(uint8_t *Array, uint16_t Length);

/**
 * @brief 发送一个字符串
 *
 * @param String 字符串首地址
 */
void Serial_SendString(char *String);

/**
 * @brief 将数字转换为ASCII字符串并发送
 *
 * @param Number 数字 四字节
 * @param Length 数字长度
 */
void Serial_SendNumber(uint32_t Number, uint8_t Length);

/**
 * @brief 根据Index取对应的位的数值
 *
 * @param Base 数
 * @param Index 位
 * @return uint8_t 对应的值
 */
uint8_t Serial_GetSmallNum(uint32_t Base, uint8_t Index);

/**
 * @brief 重写函数fputc使printf输出到串口
 *
 * @param ch 字符
 * @param f 文件指针
 * @return int 返回字符
 */
int fputc(int ch, FILE *f);

/**
 * @brief 二次封装的sprintf
 *
 * @param format 待处理的字符串
 * @param ... 可变参数
 */
void Serial_SPrintf(char *format, ...);

/**
 * @brief 重置RxData缓冲区
 *
 */
void ResetRxData(void);

#endif // !__SERIAL_H
