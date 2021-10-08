#ifndef __VS1003_H_
#define __VS1003_H_


#include<fat.h>

sbit VS_XRST=P3^7;
sbit VS_DREQ=P1^5;
sbit VS_XCS=P1^4;
sbit VS_XDCS=P1^3;

void Mp3WriteRegister(u8 addr,u16 cmd);
u8 Mp3Reset();//”≤∏¥Œª
void Mp3SoftReset();	//»Ì∏¥Œª
void Mp3PlayDat(u8 *dat);
void Mp3Play2048zero();
void VolSetting();
void Test();
#endif