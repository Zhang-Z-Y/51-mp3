#include<SPI.h>

u8 spi_slow=0;
u16 spi_speed=400;

//Ð´Èë
//void SPIWrite(u8 dat)
//{
//	u8 n=8;//·¢ËÍ8Î»×Ö·û
//	u8 i;	
//	for(i=0;i<n;i++)
//	{
//		MOSI=dat>>n-1;
//		SCLK=1;
//		dat<<=1;
//		SCLK=0;	   
//	}
//	SCLK=0;
//}
void SPIWrite(u8 dat)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		SCLK=0;
		if(spi_slow)	Delay(spi_speed);
		MOSI=dat>>7;
		SCLK=1;
		if(spi_slow)	Delay(spi_speed);
		dat<<=1;
	}
	SCLK=0;
}



//¶ÁÈ¡
u8 SPIRead()
{
	u8 i;
	u8 dat=0;
	SCLK=0;
	for(i=0;i<8;i++)
	{
		dat<<=1;
		SCLK=1;
		if(spi_slow)	Delay(spi_speed);
		dat=MISO|dat;
		SCLK=0;		 
		if(spi_slow)	Delay(spi_speed);  
	}
	return dat;	
}

void DelayMs(u16 ms)
{                         
	u16 a;
	while(ms)
	{
		a=1800;
		while(a--);
		ms--;
	}
}

//void DelayMs(u16 ms)   //Îó²î 0us
//{
//    unsigned char a,b;
//	while(ms>0)
//	{
//	    for(b=4;b>0;b--)
//	        for(a=248;a>0;a--);
//	    _nop_();  //if Keil,require use intrins.h
//		ms--;
//	}
//}
//
void Delay(u16 a)
{
	while(a--);
}