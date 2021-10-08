#include"vs1003.h"
#include<Usart.h>
#include"musicdata.c"

extern u8 VOL;

void Mp3WriteRegister(u8 addr,u16 cmd)
{
	VS_DREQ=1;
	while(!VS_DREQ);
	VS_XCS=0;		  //SCI
	SPIWrite(0x02);//д�Ĵ�������
	SPIWrite(addr);
	SPIWrite((u8)(cmd>>8));
	SPIWrite((u8)(cmd&0x0f));
	VS_XCS=1;	
	while(!VS_DREQ)	UARTSendString("wait");
}

u8 Mp3Reset()//Ӳ��λ
{
	VS_XRST=0;
	DelayMs(10);
	SPIWrite(0xff);//����SPI����
	VS_XCS=1;
	VS_XDCS=1;
	VS_XRST=1;
	while(VS_DREQ==0);

	return 1;
}

void Mp3SoftReset()	//��λ
{
	VS_XCS=0;
	Mp3WriteRegister(0,0x824);
	Mp3WriteRegister(0x3,0x9800);//����ʱ��
	Mp3WriteRegister(0x5,0xbb81);//������48k
	Mp3WriteRegister(0x2,0x55);//����
	Mp3WriteRegister(0xb,0xffff);//���� ,�߰�λΪ������
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

void Mp3PlayDat(u8 *dat)	   //��������,����Data��512�ֽڵ�����,ÿ�η���32�ֽ�,
{
	u16 i=0,a=0;
	while(i<512)
	{
		if(VS_DREQ==1)		//���DREQ����DREQΪ��ʱ����һ��32���ֽڵ�����
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

void VolSetting()//��������
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
		if(VS_DREQ==1)		//���DREQ����DREQΪ��ʱ����һ��32���ֽڵ�����
			{	
				
				for(a=0;a<32;a++)
				{
					
					SPIWrite(MusicData[i]);
					i++;
					
				}
					
			}else
				UARTSendString("���ֵȴ�");	
	}
	VS_XDCS=1;
	VS_XCS=1;
		
}