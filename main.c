#include"reg52.h"
#include<fat.h>
#include<oled.h>
#include<Usart.h>
#include"vs1003.h"

extern u8 xdata *Data;
extern u8 SD_type;//SD卡类型,0为SD,1为SDHC,2为SD1.0,3为MMC,4为不支持
extern u8 spi_slow; 


u8 VOL=254;		 //0到254级调节
u8 song_num=0;
u8 song_now=0;
u32 music_cluster;	//当前播放所在的簇
u32 now_play_cluster_num=0;//已经播放了多少簇

struct DBR//DBR信息		//DBR是由硬盘MBR装载的程序段
{
	u16 DBR_addr;				//DBR的扇区号,磁盘的0号扇区的 0X1C6~0X1C9
	u16 reverse_sector_num;		//保留扇区数,DBR扇区，偏移0X0E~0X0F
	u8 sector_num;				//每簇所占扇区数目,DBR的偏移0X0D
	u16 fat1_addr;				 //FAT1起始扇区,也就是fat1表地址,FAT1起始扇区=DBR的扇区号+保留扇区数
	u32 fat_sector_num;			//一个FAT的扇区数,也是每个fat表占用扇区数,从DBR的偏移0X24~0X27得到
	u16 root_cluster;			 //起始簇号,根目录所在簇号,DBR的偏移0X2C~0X2F得到,一般为2.
	u32 root_addr;				 //根目录起始扇区=FAT1的起始扇区号+一个FAT的扇区数×2+(起始簇号-2)x每簇的扇区数
}D1;

//sbit IRIN=P3^2;	//红外传输

sbit Key_up=P2^1;
sbit key_down=P2^3;
sbit key_left=P2^4;
sbit key_right=P2^0;
sbit key_centre=P2^2;

#define KEY P2

struct Music //mp3文件信息
{
	u8 name[8];	   
	u32 cluster;
	u32 occupied_sectors;	
}M1[20];

//void ButtonInit()	//按键中断初始化
//{
//	IT0=1;
//	EX0=1;
//	EA=1;
//}
void KeyDetect()
{
	u8 a;
	KEY=0x1f;
	if((KEY&0x1f)!=0x1f)
	{
		DelayMs(10);
		if((KEY&0x1f)==0x0f)//左按键按下
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
			if(song_now==0)//第一首
				song_now=song_num-1;
			else
				song_now--;
			OLEDClear();			   //oled显示
			for(a=0;a<8;a++)
				OLEDShowChar(a*8,0,M1[song_now].name[a]);
			music_cluster=M1[song_now].cluster;
			Mp3SoftReset();
			now_play_cluster_num=0;
			OpenFile(music_cluster);
		}else if(((KEY&0x1f)==0x17))//下按键按下
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
			while(!VS_DREQ);
			if(VOL!=254)
				VOL=VOL+10;
			VolSetting();

		}else if((KEY&0x1f)==0x1B)//中间按下
		{
			DelayMs(100);
			do
			{
				KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
			DelayMs(100);
			//等待重新按下
			do
			{
				KEY=0x1f;
			}while((KEY&0x1f)!=0x1B);
			DelayMs(100);
			do
			{
				KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
		}else if((KEY&0x1f)==0x1D)//上按键
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
			while(!VS_DREQ);
			if(VOL!=0)
				VOL=VOL-10;
			VolSetting();
		}else if((KEY&0x1f)==0x1E)//右按键
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//等待松开按键
			if(song_now+1==song_num)//最后一首
				song_now=0;
			else
				song_now++;
			OLEDClear();			   //oled显示
			for(a=0;a<8;a++)
				OLEDShowChar(a*8,0,M1[song_now].name[a]);
			music_cluster=M1[song_now].cluster;
			Mp3SoftReset();
			now_play_cluster_num=0;
			OpenFile(music_cluster);
		}
	}	
}

void PlaySong(u8 num)
{
	u8 a;
	now_play_cluster_num=0;
	OLEDClear();
	for(a=0;a<8;a++)
		OLEDShowChar(a*8,0,M1[num].name[a]);  //oled显示
	Mp3SoftReset();
	music_cluster=M1[num].cluster;	
//	while(music_cluster!=0x0fffffff)//结束
	while(now_play_cluster_num<=M1[num].occupied_sectors)
	{
//		UARTSendString("Next");
//		UARTSendString("music_cluster");
//		UARTSendNum(music_cluster);
		OpenFile(music_cluster);
		Mp3PlayDat(Data);
//		music_cluster=CalculateNextCluster(music_cluster);
		music_cluster++;
		now_play_cluster_num++;
		KeyDetect();//按键检测

	}
	OpenFile(music_cluster);
	Mp3PlayDat(Data);	//将当前簇播放结束
	Mp3Play2048zero();
}



//void PlaySongHalf()		  //提前知道有一文件在6簇,尝试和vs1003直连
//{
//	u16 i,addr,a;
//	Mp3SoftReset();	
//	addr=CalculateAddr(M1[0].cluster);
//	UARTSendString("music_cluster_addr");
//	UARTSendNum(addr);
////	addr=8196;
//	while(1)
//	{
//		i=16;
//		SDReadBlockHalf(addr);//SD数据开
//		while(i>0)			  
//		{	
////			UARTSendString("Here");		
//			while(VS_DREQ==1)
//			{
////				UARTSendString("Send");
//				VS_XCS=1;
//				VS_XDCS=0; //1003数据片选开
//				for(a=0;a<256;a++)	 //读取32字节
//				{
//					SCLK=1;
//					MOSI=MISO;
//					SCLK=0;
//				}
//				VS_XDCS=1;
//				i--;
//			}
//		}
//		VS_XDCS=1;
//		addr++;
//		SDRead();		  //16位crc
//		SDRead();
//		SD_CS=1;
//		SPIWrite(0xff);
//	}
//}




void main()
{
//	u8 flag;

	UsartInit();
	DelayMs(1);
//	ButtonInit();		   
	UARTSendString("串口初始化完成");
	OLEDInit();
	OLEDClear();
	UARTSendString("oled初始化完成");
	DelayMs(1);
	OLEDShowString(0,0,"Loading...");
	UARTSendString("开始初始化SD卡");	
	spi_slow=1;

	DelayMs(100);	 //不能省!!!!!!!!!!!!!!!
	SDInit();
	if(SD_type==2)
		UARTSendString("SD1");
	else if(SD_type==3)
		UARTSendString("SDmmc");
	else 
	{
		UARTSendString("失败");
		OLEDShowString(0,0,"SDcard error");
		while(1);
	}
	spi_slow=0;
	UARTSendString("Fat初始化开始");
	fatInit();
	
	UARTSendString("Fat初始化完成");
	UARTSendString("vs1003Reset");
	Mp3Reset();//vs1003硬复位
	Mp3SoftReset();	
//	SinTest();
	UARTSendString("搜索音乐中");
	FindMP3(2);
	UARTSendString("搜索完成,歌曲数目为:");
	UARTSendNum(song_num);
	OLEDShowString(0,0,"find ");
	OLEDShowNum(40,0,song_num);
	OLEDShowString(48,0," song");
	DelayMs(250);
	DelayMs(250);
	DelayMs(250);
	DelayMs(250);
	if(song_num==0)
	{
		OLEDClear();
		OLEDShowString(0,0,"NO song");
		while(1);
	}	
	UARTSendString("开始播放...");
	//音量尝试
	DelayMs(10);
	spi_slow=1;
	VS_XCS=0;
	VS_XDCS=1;
	SPIWrite(0x03);
	SPIWrite(0x00);
	UARTSendNum(SPIRead());
	UARTSendNum(SPIRead());
	VS_XCS=1;
	while(!VS_DREQ)	UARTSendString("wait");
	spi_slow=0;
//	PlaySongHalf();

	for(song_now=0;song_now<song_num;song_now++)
		PlaySong(song_now);


	while(1);
		
}

//void ReadIR() interrupt 0
//{
//	u8 i,j,k,dat[4]=0;
//	u16 a,error;
//	DelayMs(7);
//	if(IRIN==0)//接收到正确信号
//	{
//		DelayMs(5);
//		error=300;
//		while(IRIN==1||error==0)	error--;//等待起始码过去
//		for(j=0;j<4;j++)
//		{
//			for(i=0;i<8;i++)
//			{
//				a=0;
//				while(IRIN==0);
//				while(IRIN==1) a++;	//开始计算1的时间
//				dat[k]<<=1; 
//				if(a>70)//为1
//					dat[k]|=1;
//				else
//					dat[k]|=0;
//			}
//		}		
//	}
//	
//
//	if(dat[3]==0x44)//播放暂停键
//	{
//
//	}else if(dat[3]==0)	//上一首0x40
//	{
//		if(song_now==0)//第一首
//			song_now=song_num-1;
//		else
//			song_now--;
//		OLEDClear();			   //oled显示
//		for(a=0;a<8;a++)
//			OLEDShowChar(a*8,0,M1[song_now].name[a]);
//		music_cluster=M1[song_now].cluster;
//		Mp3SoftReset();
//		OpenFile(music_cluster);
//
//					
//	}else if(dat[3]==1)	//下一首0x43
//	{
//		if(song_now+1==song_num)//最后一首
//			song_now=0;
//		else
//			song_now++;
//		OLEDClear();			   //oled显示
//		for(a=0;a<8;a++)
//			OLEDShowChar(a*8,0,M1[song_now].name[a]);
//		music_cluster=M1[song_now].cluster;
//		Mp3SoftReset();
//		OpenFile(music_cluster);
//	}else if(dat[3]==2)	//音量小0x15
//	{
//		if(VOL!=250)
//			VOL=VOL+10;
//		VolSetting();
//	}else if(dat[3]==3)	//音量大0x09
//	{
//		if(VOL!=0)
//			VOL=VOL-10;
//		VolSetting();
//	}
////}
//u8 ButtonScan()
//{
//		button_up=1;
//		if(button_control==1)//上按键被按下
//			return 1;
//		button_down=1;
//		if(button_control==1)//下按键被按下
//			return 2;
//		button_left=1;
//		if(button_control==1)//左按键被按下
//			return 3;
//		button_right=1;//右按键被按下
//		if(button_control==1)
//			return 4;
//		button_centre=1;//中间按键被按下
//		if(button_control==1)
//			return 5;
//}
//
//void ButtonInt0() interrupt 0
//{
//	DelayMs(1);	//消抖
//	ButtonScan();
//			
//	
//}
 