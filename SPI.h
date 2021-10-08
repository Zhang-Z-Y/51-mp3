#ifndef __SPI_H__
#define __SPI_H__

#include<intrins.h>
#include"reg52.h"

sbit MISO=P3^3;
sbit MOSI=P3^4;
sbit SCLK=P3^5;


#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long int

//–¥»Î
void SPIWrite(u8 dat);


//∂¡»°
u8 SPIRead();

void DelayMs(u16 ms);

void Delay(u16 a);
#endif