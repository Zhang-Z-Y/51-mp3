#include<oled.h>
#include<oledfont.h>


void OLEDSetPos(u8 x,u8 y)	   //页地址
{
	OLEDCMD(0xb0+y);//目标显示位置页起始地址
	OLEDCMD(((x&0xf0)>>4)|0x10);//列起始地址高位
	OLEDCMD((x&0x0f)|0x01);//列起始地址低位	******
}

//SSD1306写入方法
void OLEDCMD(u8 dat)
{
	oled_dc=0;
	oled_cs=0;
	SPIWrite(dat);
	oled_cs=1;
	oled_dc=1;
}
void OLEDWrite(u8 dat)
{
	oled_dc=1;
	oled_cs=0;
	SPIWrite(dat);
	oled_cs=1;
	oled_dc=1;
}

//清屏
void OLEDClear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLEDCMD (0xb0+i);    //设置页地址（0~7）
		OLEDCMD (0x00);      //设置显示位置—列低地址
		OLEDCMD (0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLEDWrite(0); 
	} //更新显示
}



void OLEDInit()
{
//  重启电路
//当RES# 输入为LOW时，芯片初始化为下面的状态：
//1.显示关
//2.128 * 64显示模式
//3.正常的segment和显示数据列地址和行地址映射（SEG0映射到address 00h，COM0映射到address 00h）
//4.在串口中清除移位寄存器数据
//5.显示开始行设置为显示RAM地址0
//6.列地址计数设为0
//7.正常扫描COM输出方向
//8.对比度控制寄存器设为7Fh
//正常显示模式（等同于A4h命令）

//不懂,直接复制
	oled_res=1;
	DelayMs(100);
	oled_res=0;
	DelayMs(100);
	oled_res=1;

	OLEDCMD(0xAE);//--turn off oled panel
	OLEDCMD(0x00);//---set low column address
	OLEDCMD(0x10);//---set high column address
	OLEDCMD(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLEDCMD(0x81);//--set contrast control register
	OLEDCMD(0xCF); // Set SEG Output Current Brightness
	OLEDCMD(0xA1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLEDCMD(0xC8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLEDCMD(0xA6);//--set normal display
	OLEDCMD(0xA8);//--set multiplex ratio(1 to 64)
	OLEDCMD(0x3f);//--1/64 duty
	OLEDCMD(0xD3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLEDCMD(0x00);//-not offset
	OLEDCMD(0xd5);//--set display clock divide ratio/oscillator frequency
	OLEDCMD(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLEDCMD(0xD9);//--set pre-charge period
	OLEDCMD(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLEDCMD(0xDA);//--set com pins hardware configuration
	OLEDCMD(0x12);
	OLEDCMD(0xDB);//--set vcomh
	OLEDCMD(0x40);//Set VCOM Deselect Level
	OLEDCMD(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLEDCMD(0x02);//
	OLEDCMD(0x8D);//--set Charge Pump enable/disable
	OLEDCMD(0x14);//--set(0x10) disable
	OLEDCMD(0xA4);// Disable Entire Display On (0xa4/0xa5)
	OLEDCMD(0xA67);// Disable Inverse Display On (0xa6/a7) 
	OLEDCMD(0xAF);//--turn on oled panel
	OLEDCMD(0xAF); /*display ON*/ 
	OLEDClear();
	OLEDSetPos(0,0);	
}



void OLEDDisplayON()
{
	OLEDCMD(0x8d);
	OLEDCMD(0x14);
	OLEDCMD(0xaf);
}

void OLEDDisplayOFF()
{
	OLEDCMD(0x8d);
	OLEDCMD(0x10);
	OLEDCMD(0xae);
}

void OLEDShowChar(u8 x,u8 y,u8 chr)//显示字符
{
	u8 c=0,i=0;
	c=chr-' ';
	if(x>=127)
	{
		x=0;
		y=y+2;
	}
	OLEDSetPos(x,y);
	for(i=0;i<8;i++)
		OLEDWrite(F8X16[16*c+i]);
	OLEDSetPos(x,y+1);
	for(i=0;i<8;i++)
		OLEDWrite(F8X16[16*c+8+i]);	

}

void OLEDShowString(u8 x,u8 y,u8 *str)//显示字符串
{
	u8 a;
	for(a=0;str[a]!='\0';a++)
	{
  		OLEDShowChar(x,y,str[a]);
		x=x+8;
		if(x>120){x=0;y+=2;}
	}

}

void OLEDShowCN(u8 x,u8 y,u8 chr)//显示中文16x16
{
	u8 i=0;
	if(x>127)
	{
		x=0;
		y=y+2;
	}
	OLEDSetPos(x,y);
	for(i=0;i<16;i++)
		OLEDWrite(CN[2*chr][i]);
	OLEDSetPos(x,y+1);
	for(i=0;i<16;i++)
		OLEDWrite(CN[2*chr+1][i]);

}

void OLEDShowNum(u8 x,u8 y,u8 chr)
{
	while(chr)
	{
	OLEDShowChar(x,y,chr%10+48);
	x+=8;
	chr/=10;
	}		
}