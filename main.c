#include"reg52.h"
#include<fat.h>
#include<oled.h>
#include<Usart.h>
#include"vs1003.h"

extern u8 xdata *Data;
extern u8 SD_type;//SD������,0ΪSD,1ΪSDHC,2ΪSD1.0,3ΪMMC,4Ϊ��֧��
extern u8 spi_slow; 


u8 VOL=254;		 //0��254������
u8 song_num=0;
u8 song_now=0;
u32 music_cluster;	//��ǰ�������ڵĴ�
u32 now_play_cluster_num=0;//�Ѿ������˶��ٴ�

struct DBR//DBR��Ϣ		//DBR����Ӳ��MBRװ�صĳ����
{
	u16 DBR_addr;				//DBR��������,���̵�0�������� 0X1C6~0X1C9
	u16 reverse_sector_num;		//����������,DBR������ƫ��0X0E~0X0F
	u8 sector_num;				//ÿ����ռ������Ŀ,DBR��ƫ��0X0D
	u16 fat1_addr;				 //FAT1��ʼ����,Ҳ����fat1���ַ,FAT1��ʼ����=DBR��������+����������
	u32 fat_sector_num;			//һ��FAT��������,Ҳ��ÿ��fat��ռ��������,��DBR��ƫ��0X24~0X27�õ�
	u16 root_cluster;			 //��ʼ�غ�,��Ŀ¼���ڴغ�,DBR��ƫ��0X2C~0X2F�õ�,һ��Ϊ2.
	u32 root_addr;				 //��Ŀ¼��ʼ����=FAT1����ʼ������+һ��FAT����������2+(��ʼ�غ�-2)xÿ�ص�������
}D1;

//sbit IRIN=P3^2;	//���⴫��

sbit Key_up=P2^1;
sbit key_down=P2^3;
sbit key_left=P2^4;
sbit key_right=P2^0;
sbit key_centre=P2^2;

#define KEY P2

struct Music //mp3�ļ���Ϣ
{
	u8 name[8];	   
	u32 cluster;
	u32 occupied_sectors;	
}M1[20];

//void ButtonInit()	//�����жϳ�ʼ��
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
		if((KEY&0x1f)==0x0f)//�󰴼�����
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
			if(song_now==0)//��һ��
				song_now=song_num-1;
			else
				song_now--;
			OLEDClear();			   //oled��ʾ
			for(a=0;a<8;a++)
				OLEDShowChar(a*8,0,M1[song_now].name[a]);
			music_cluster=M1[song_now].cluster;
			Mp3SoftReset();
			now_play_cluster_num=0;
			OpenFile(music_cluster);
		}else if(((KEY&0x1f)==0x17))//�°�������
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
			while(!VS_DREQ);
			if(VOL!=254)
				VOL=VOL+10;
			VolSetting();

		}else if((KEY&0x1f)==0x1B)//�м䰴��
		{
			DelayMs(100);
			do
			{
				KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
			DelayMs(100);
			//�ȴ����°���
			do
			{
				KEY=0x1f;
			}while((KEY&0x1f)!=0x1B);
			DelayMs(100);
			do
			{
				KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
		}else if((KEY&0x1f)==0x1D)//�ϰ���
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
			while(!VS_DREQ);
			if(VOL!=0)
				VOL=VOL-10;
			VolSetting();
		}else if((KEY&0x1f)==0x1E)//�Ұ���
		{
			do
			{
			KEY=0x1f;
			}while(KEY!=0x1f);//�ȴ��ɿ�����
			if(song_now+1==song_num)//���һ��
				song_now=0;
			else
				song_now++;
			OLEDClear();			   //oled��ʾ
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
		OLEDShowChar(a*8,0,M1[num].name[a]);  //oled��ʾ
	Mp3SoftReset();
	music_cluster=M1[num].cluster;	
//	while(music_cluster!=0x0fffffff)//����
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
		KeyDetect();//�������

	}
	OpenFile(music_cluster);
	Mp3PlayDat(Data);	//����ǰ�ز��Ž���
	Mp3Play2048zero();
}



//void PlaySongHalf()		  //��ǰ֪����һ�ļ���6��,���Ժ�vs1003ֱ��
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
//		SDReadBlockHalf(addr);//SD���ݿ�
//		while(i>0)			  
//		{	
////			UARTSendString("Here");		
//			while(VS_DREQ==1)
//			{
////				UARTSendString("Send");
//				VS_XCS=1;
//				VS_XDCS=0; //1003����Ƭѡ��
//				for(a=0;a<256;a++)	 //��ȡ32�ֽ�
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
//		SDRead();		  //16λcrc
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
	UARTSendString("���ڳ�ʼ�����");
	OLEDInit();
	OLEDClear();
	UARTSendString("oled��ʼ�����");
	DelayMs(1);
	OLEDShowString(0,0,"Loading...");
	UARTSendString("��ʼ��ʼ��SD��");	
	spi_slow=1;

	DelayMs(100);	 //����ʡ!!!!!!!!!!!!!!!
	SDInit();
	if(SD_type==2)
		UARTSendString("SD1");
	else if(SD_type==3)
		UARTSendString("SDmmc");
	else 
	{
		UARTSendString("ʧ��");
		OLEDShowString(0,0,"SDcard error");
		while(1);
	}
	spi_slow=0;
	UARTSendString("Fat��ʼ����ʼ");
	fatInit();
	
	UARTSendString("Fat��ʼ�����");
	UARTSendString("vs1003Reset");
	Mp3Reset();//vs1003Ӳ��λ
	Mp3SoftReset();	
//	SinTest();
	UARTSendString("����������");
	FindMP3(2);
	UARTSendString("�������,������ĿΪ:");
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
	UARTSendString("��ʼ����...");
	//��������
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
//	if(IRIN==0)//���յ���ȷ�ź�
//	{
//		DelayMs(5);
//		error=300;
//		while(IRIN==1||error==0)	error--;//�ȴ���ʼ���ȥ
//		for(j=0;j<4;j++)
//		{
//			for(i=0;i<8;i++)
//			{
//				a=0;
//				while(IRIN==0);
//				while(IRIN==1) a++;	//��ʼ����1��ʱ��
//				dat[k]<<=1; 
//				if(a>70)//Ϊ1
//					dat[k]|=1;
//				else
//					dat[k]|=0;
//			}
//		}		
//	}
//	
//
//	if(dat[3]==0x44)//������ͣ��
//	{
//
//	}else if(dat[3]==0)	//��һ��0x40
//	{
//		if(song_now==0)//��һ��
//			song_now=song_num-1;
//		else
//			song_now--;
//		OLEDClear();			   //oled��ʾ
//		for(a=0;a<8;a++)
//			OLEDShowChar(a*8,0,M1[song_now].name[a]);
//		music_cluster=M1[song_now].cluster;
//		Mp3SoftReset();
//		OpenFile(music_cluster);
//
//					
//	}else if(dat[3]==1)	//��һ��0x43
//	{
//		if(song_now+1==song_num)//���һ��
//			song_now=0;
//		else
//			song_now++;
//		OLEDClear();			   //oled��ʾ
//		for(a=0;a<8;a++)
//			OLEDShowChar(a*8,0,M1[song_now].name[a]);
//		music_cluster=M1[song_now].cluster;
//		Mp3SoftReset();
//		OpenFile(music_cluster);
//	}else if(dat[3]==2)	//����С0x15
//	{
//		if(VOL!=250)
//			VOL=VOL+10;
//		VolSetting();
//	}else if(dat[3]==3)	//������0x09
//	{
//		if(VOL!=0)
//			VOL=VOL-10;
//		VolSetting();
//	}
////}
//u8 ButtonScan()
//{
//		button_up=1;
//		if(button_control==1)//�ϰ���������
//			return 1;
//		button_down=1;
//		if(button_control==1)//�°���������
//			return 2;
//		button_left=1;
//		if(button_control==1)//�󰴼�������
//			return 3;
//		button_right=1;//�Ұ���������
//		if(button_control==1)
//			return 4;
//		button_centre=1;//�м䰴��������
//		if(button_control==1)
//			return 5;
//}
//
//void ButtonInt0() interrupt 0
//{
//	DelayMs(1);	//����
//	ButtonScan();
//			
//	
//}
 