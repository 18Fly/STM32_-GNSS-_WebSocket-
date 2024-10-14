#ifndef __WEBSOCKET_H_
#define __WEBSOCKET_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "MyDMA.h"

/**
 * @brief 返回WS握手数据
 *
 * @param Key 验证Key
 * @param Buf 响应数据
 */
void repeatWSRaw(char *Key, char *Buf);

#endif // !__WEBSOCKET_H_
