#include"reg52.h"
#include"SPI.h"
#include"string.h"

void UARTSendString(char *s)
{
	u16 i;
	ES=0;
	for(i=0;i<strlen(s);i++)
	{
		SBUF=s[i];
		while(!TI);
		TI=0;
	}
	SBUF=0x0d;
	while(!TI);
	TI=0;
	SBUF=0x0a;
	while(!TI);
	TI=0;
	ES=1;

}

void u32tostr(unsigned long dat,char *str) 
{
	u8 temp[20];
	u8 i=0,j=0;
	while(dat)
	{
		temp[i]=dat%10+48;
		i++;
		dat/=10;
	}
	j=i;
	for(i=0;i<j;i++)
		str[i]=temp[j-i-1];
	if(!i)
		str[i++]='0';
	str[i]=0;
}

//发送数值
void UARTSendNum(unsigned long dat)
{
 char temp[20];
 u32tostr(dat,temp);
 UARTSendString(temp);
}

void UsartInit()			  //串口调试
{
	SCON=0X50;			//设置为工作方式1
	TMOD=0X20;			//设置计数器工作方式2
	PCON=0X80;			//波特率加倍
	TH1=0XF3;//	0XE6			//计数器初始值设置，注意波特率是4800的
	TL1=0XF3;//	 0XE6
	ES=1;						//打开接收中断
	EA=1;
	TI=0;
	RI=0;						//打开总中断
	TR1=1;					//打开计数器
}