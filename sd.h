#ifndef __SD_H__
#define __SD_H__

#include"reg52.h"

#include<SPI.h>



sbit SD_CS=P3^6;

#define	CMD0	0X00  //��λSD��,�������п�������״̬
#define	CMD1	0X01  //����SD����ACTIVATEģʽ
#define	CMD8	0X08  //���ͽӿ�״̬����
#define	CMD17	0X11  //��ȡ��������
#define	CMD55	0X37  //����SD����һ������ʽ��Ӧ������,Ҳ������һ��ΪACMD����
#define	ACMD41	0X29  //���Ϳ�����֧����Ϣ



u8 SDCmd(u8 cmd,u32 argu,u8 crc);
u8 SDInit();
void SDWrite(u8 dat);
u8 SDRead();
u8 *SDReadBlock(u32 addr);
void SDReadBlockHalf(u32 addr);

#endif