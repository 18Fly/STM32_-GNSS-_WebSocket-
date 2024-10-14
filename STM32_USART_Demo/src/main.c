#include "stm32f10x.h"
#include "OLED.h"
#include "ESP01S.h"

extern Serial_RxData RxData;

extern struct ReceiveBuffer Buffer;

extern boolean Send_Flag;
uint8_t SendCount;

struct {
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
} UTCTime = {0};

int main(void)
{
    OLED_Init();
    Serial_Init();
    ESP01S_Init();

    Delay_s(2);
    ESP01S_SendString("AT+CIPMUX=1\r\n");
    Delay_ms(500);
    ESP01S_SendString("AT+CIPSERVER=1,2568\r\n");
    // Delay_ms(100);

    OLED_ShowString(1, 1, "G N S S");
    OLED_ShowString(2, 1, "E:");
    OLED_ShowString(3, 1, "W:");
    OLED_ShowString(4, 1, "UTC:");
    OLED_ShowChar(4, 8, ':');
    OLED_ShowChar(4, 11, ':');

    while (1) {
        SendWsAccept();
        // OLED_ShowHexNum(1, 12, Send_Flag, 2);
        // Delay_s(1);
        if (RxData.receiveFlag) {
            if (Buffer.RxBuffer[5] == 'A' && Buffer.RxBuffer[4] == 'G') {
                RxData.Latitude[0] = Buffer.RxBuffer[18];
                RxData.Latitude[1] = Buffer.RxBuffer[19];
                RxData.Latitude[2] = Buffer.RxBuffer[22];
                RxData.Latitude[3] = Buffer.RxBuffer[20];
                RxData.Latitude[4] = Buffer.RxBuffer[21];
                RxData.Latitude[5] = Buffer.RxBuffer[23];
                RxData.Latitude[6] = Buffer.RxBuffer[24];
                RxData.Latitude[7] = Buffer.RxBuffer[25];
                RxData.Latitude[8] = Buffer.RxBuffer[26];
                RxData.Latitude[9] = Buffer.RxBuffer[27];

                RxData.Longitude[0]  = Buffer.RxBuffer[31];
                RxData.Longitude[1]  = Buffer.RxBuffer[32];
                RxData.Longitude[2]  = Buffer.RxBuffer[33];
                RxData.Longitude[3]  = Buffer.RxBuffer[36];
                RxData.Longitude[4]  = Buffer.RxBuffer[34];
                RxData.Longitude[5]  = Buffer.RxBuffer[35];
                RxData.Longitude[6]  = Buffer.RxBuffer[37];
                RxData.Longitude[7]  = Buffer.RxBuffer[38];
                RxData.Longitude[8]  = Buffer.RxBuffer[39];
                RxData.Longitude[9]  = Buffer.RxBuffer[40];
                RxData.Longitude[10] = Buffer.RxBuffer[41];

                UTCTime.Hour = (Buffer.RxBuffer[7] - '0') * 10 + (Buffer.RxBuffer[8] - '0');
                UTCTime.Hour += 8;
                if (UTCTime.Hour > 24)
                    UTCTime.Hour -= 24;
                UTCTime.Minute = (Buffer.RxBuffer[9] - '0') * 10 + (Buffer.RxBuffer[10] - '0');
                UTCTime.Second = (Buffer.RxBuffer[11] - '0') * 10 + (Buffer.RxBuffer[12] - '0');
                OLED_ShowString(2, 3, RxData.Longitude);
                OLED_ShowString(3, 3, RxData.Latitude);
                OLED_ShowNum(4, 6, UTCTime.Hour, 2);
                OLED_ShowNum(4, 9, UTCTime.Minute, 2);
                OLED_ShowNum(4, 12, UTCTime.Second, 2);
                SendCount++;
            }
            SendLngLat();
            RxData.receiveFlag = false;
            ResetRxData();
        }
    }
}
