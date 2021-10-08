#ifndef _OLED_H__
#define _OLED_H__

#include "spi.h"

sbit oled_cs=P1^2; //Ƭѡ
sbit oled_dc=P1^1; //����/�������
sbit oled_res=P1^0;//��λ

void OLEDCMD(u8 dat);
void OLEDWrite(u8 dat);
void OLEDSetPos(u8 x,u8 y);
void OLEDInit();
void OLEDClear(void);
void OLEDDisplayON();
void OLEDDisplayOFF();
void OLEDShowChar(u8 x,u8 y,u8 chr);
void OLEDShowString(u8 x,u8 y,u8 *str);
void OLEDShowCN(u8 x,u8 y,u8 chr);
void OLEDShowNum(u8 x,u8 y,u8 chr);

#endif
