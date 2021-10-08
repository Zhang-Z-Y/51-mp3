#include"vs1003.h"
#include<Usart.h>
#include"musicdata.c"

extern u8 VOL;

void Mp3WriteRegister(u8 addr,u16 cmd)
{
	VS_DREQ=1;
	while(!VS_DREQ);
	VS_XCS=0;		  //SCI
	SPIWrite(0x02);//写寄存器命令
	SPIWrite(addr);
	SPIWrite((u8)(cmd>>8));
	SPIWrite((u8)(cmd&0x0f));
	VS_XCS=1;	
	while(!VS_DREQ)	UARTSendString("wait");
}

u8 Mp3Reset()//硬复位
{
	VS_XRST=0;
	DelayMs(10);
	SPIWrite(0xff);//启动SPI传输
	VS_XCS=1;
	VS_XDCS=1;
	VS_XRST=1;
	while(VS_DREQ==0);

	return 1;
}

void Mp3SoftReset()	//软复位
{
	VS_XCS=0;
	Mp3WriteRegister(0,0x824);
	Mp3WriteRegister(0x3,0x9800);//设置时钟
	Mp3WriteRegister(0x5,0xbb81);//采样率48k
	Mp3WriteRegister(0x2,0x55);//重音
	Mp3WriteRegister(0xb,0xffff);//音量 ,高八位为左声道
	DelayMs(1);
	VS_XCS=1;
	while(!VS_DREQ);
	VS_XDCS=0;
	SPIWrite(0);
	SPIWrite(0);
	SPIWrite(0);
	SPIWrite(0);
	VS_XDCS=1;
}

void Mp3PlayDat(u8 *dat)	   //播放音乐,发送Data中512字节的数据,每次发送32字节,
{
	u16 i=0,a=0;
	while(i<512)
	{
		if(VS_DREQ==1)		//检测DREQ，当DREQ为高时发下一个32个字节的数据
		{	
			VS_XDCS=0;
			VS_XCS=1;
			for(a=0;a<32;a++)
			{
				
				SPIWrite(dat[i]);
				i++;
				
			}
			VS_XDCS=1;
			VS_XCS=1;	
		}

	}
}


void Mp3Play2048zero()
{
	u16 i=0,a=0;
	for(i=0;i<64;i++)
	{
		if(!VS_DREQ)
		{
			VS_XDCS=0;
			for(a=0;a<32;a++)
			{
				SPIWrite(0);				
			}
			VS_XDCS=1;
			VS_XCS=1;		
		}
	}
}

void VolSetting()//音量设置
{
		Mp3WriteRegister(0x0b,((u32)VOL)<<8+VOL);
		VS_XDCS=0;
}

void Test()
{	
	u16 i=0,iCount,a;
	iCount = sizeof(MusicData);
	VS_XDCS=0;
	VS_XCS=1;
	while(i<iCount)
	{
		if(VS_DREQ==1)		//检测DREQ，当DREQ为高时发下一个32个字节的数据
			{	
				
				for(a=0;a<32;a++)
				{
					
					SPIWrite(MusicData[i]);
					i++;
					
				}
					
			}else
				UARTSendString("出现等待");	
	}
	VS_XDCS=1;
	VS_XCS=1;
		
}