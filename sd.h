#ifndef __SD_H__
#define __SD_H__

#include"reg52.h"

#include<SPI.h>



sbit SD_CS=P3^6;

#define	CMD0	0X00  //复位SD卡,重置所有卡到空闲状态
#define	CMD1	0X01  //设置SD卡到ACTIVATE模式
#define	CMD8	0X08  //发送接口状态命令
#define	CMD17	0X11  //读取扇区命令
#define	CMD55	0X37  //告诉SD卡下一个命令式卡应用命令,也就是下一条为ACMD命令
#define	ACMD41	0X29  //发送卡容量支持信息



u8 SDCmd(u8 cmd,u32 argu,u8 crc);
u8 SDInit();
void SDWrite(u8 dat);
u8 SDRead();
u8 *SDReadBlock(u32 addr);
void SDReadBlockHalf(u32 addr);

#endif