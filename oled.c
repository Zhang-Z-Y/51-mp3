#include<oled.h>
#include<oledfont.h>


void OLEDSetPos(u8 x,u8 y)	   //ҳ��ַ
{
	OLEDCMD(0xb0+y);//Ŀ����ʾλ��ҳ��ʼ��ַ
	OLEDCMD(((x&0xf0)>>4)|0x10);//����ʼ��ַ��λ
	OLEDCMD((x&0x0f)|0x01);//����ʼ��ַ��λ	******
}

//SSD1306д�뷽��
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

//����
void OLEDClear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLEDCMD (0xb0+i);    //����ҳ��ַ��0~7��
		OLEDCMD (0x00);      //������ʾλ�á��е͵�ַ
		OLEDCMD (0x10);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)OLEDWrite(0); 
	} //������ʾ
}



void OLEDInit()
{
//  ������·
//��RES# ����ΪLOWʱ��оƬ��ʼ��Ϊ�����״̬��
//1.��ʾ��
//2.128 * 64��ʾģʽ
//3.������segment����ʾ�����е�ַ���е�ַӳ�䣨SEG0ӳ�䵽address 00h��COM0ӳ�䵽address 00h��
//4.�ڴ����������λ�Ĵ�������
//5.��ʾ��ʼ������Ϊ��ʾRAM��ַ0
//6.�е�ַ������Ϊ0
//7.����ɨ��COM�������
//8.�Աȶȿ��ƼĴ�����Ϊ7Fh
//������ʾģʽ����ͬ��A4h���

//����,ֱ�Ӹ���
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
	OLEDCMD(0xA1);//--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
	OLEDCMD(0xC8);//Set COM/Row Scan Direction   0xc0���·��� 0xc8����
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

void OLEDShowChar(u8 x,u8 y,u8 chr)//��ʾ�ַ�
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

void OLEDShowString(u8 x,u8 y,u8 *str)//��ʾ�ַ���
{
	u8 a;
	for(a=0;str[a]!='\0';a++)
	{
  		OLEDShowChar(x,y,str[a]);
		x=x+8;
		if(x>120){x=0;y+=2;}
	}

}

void OLEDShowCN(u8 x,u8 y,u8 chr)//��ʾ����16x16
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