#ifndef __MYDMA_H_
#define __MYDMA_H_

#include "stm32f10x.h"

/**
 * @brief DMA初始化
 *
 * @param AddrA 源地址
 * @param AddrB 目标地址
 * @param Size 转运次数
 */
void MyDMA_Init(uint32_t AddrA, uint32_t AddrB, uint16_t Size);

/**
 * @brief 开启转运
 *
 */
void MyDMA_Transfer(void);

#endif // !__MYDMA_H_
