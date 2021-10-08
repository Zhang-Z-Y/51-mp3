#include<fat.h>
#include<Usart.h>

extern u8 xdata *Data;
extern u8 song_num;
extern struct DBR//DBR��Ϣ		//DBR����Ӳ��MBRװ�صĳ����
{
	u16 DBR_addr;				//DBR��������,���̵�0�������� 0X1C6~0X1C9
	u16 reverse_sector_num;		//����������,DBR������ƫ��0X0E~0X0F
	u8 sector_num;				//ÿ����ռ������Ŀ,DBR��ƫ��0X0D
	u16 fat1_addr;				 //FAT1��ʼ����,Ҳ����fat1���ַ,FAT1��ʼ����=DBR��������+����������
	u32 fat_sector_num;			//һ��FAT��������,Ҳ��ÿ��fat��ռ��������,��DBR��ƫ��0X24~0X27�õ�
	u16 root_cluster;			 //��ʼ�غ�,��Ŀ¼���ڴغ�,DBR��ƫ��0X2C~0X2F�õ�,һ��Ϊ2.
	u32 root_addr;				 //��Ŀ¼��ʼ����=FAT1����ʼ������+һ��FAT����������2+(��ʼ�غ�-2)xÿ�ص�������
}D1;

//u16 FindDBR()//��ȡDBR������
//{
//	u16 i=0;
//	Data=SDReadBlock(0);
//	for(i=0;i<512;i++)
//		UARTSendNum(Data[i]);
//	return (Data[0x1c7]<<8||Data[0x1c6]);
////	u16 DBR=0,i;
////	SDReadBlock(0);
////	for(i=0;i<0x1c6;i++)
////		SDRead();
////	DBR=SDRead();
////	DBR|=SDRead()<<8;
////	for(i=0;i<0x3B;i++)
////		SDRead();
////	SD_CS=1;
////	SPIWrite(0xff);
////	return DBR;
//}

extern struct Music //mp3�ļ���Ϣ
{
	u8 name[8];	   
	u32 cluster;
	u32 occupied_sectors;	
}M1[20];

void fatInit()//Fat��ʼ��
{

	Data=SDReadBlock(0);
	if(Data[0]==0xeb&&Data[1]==0x58&&Data[2]==0x90&&Data[3]==0x4d) //��MBR
		D1.DBR_addr=0;
	else{
		D1.DBR_addr=Data[0x1c7]<<8|Data[0x1c6];
	}
	UARTSendString("DBR");
	UARTSendNum(D1.DBR_addr);
//	UARTSendNum(D1.DBR_addr);
//	UARTSendString("DBR");
//	SDReadBlock(D1.DBR_addr);
//	UARTSendString("��һ��");
//	for(i=0;i<0x0d;i++)
//		UARTSendNum(SDRead());
//	D1.sector_num=SDRead();		  //0x0d
//	D1.reverse_sector_num=SDRead();	  //0x0e
//	D1.reverse_sector_num|=SDRead()<<8;  //0x0f
//	UARTSendString("�ڶ���");
//	for(i=0;i<14;i++)
//		SDRead();
//	D1.fat_sector_num=SDRead();	  //0x24
//	D1.fat_sector_num|=SDRead()<<8;	  
//	D1.fat_sector_num|=SDRead()<<16;
//	D1.fat_sector_num|=SDRead()<<24;
//	for(i=0;i<4;i++)
//		SDRead();
//	D1.root_cluster=SDRead();//0x2c
//	D1.root_cluster|=SDRead()<<8;
//
//	for(i=0;i<0x1d5;i++)
//		UARTSendNum(SDRead());
//	SD_CS=1;
//	D1.fat1_addr=D1.DBR_addr+D1.reverse_sector_num;
//	D1.root_addr=D1.fat1_addr+D1.fat_sector_num*2+(D1.root_cluster-2)*D1.sector_num;
//
//



	Data=SDReadBlock(D1.DBR_addr);//��ȡDBR����
	D1.reverse_sector_num=Data[0x0f]<<8|Data[0x0e];
	D1.sector_num=Data[0x0d];
	D1.fat_sector_num=Data[0x27]<<24|Data[0x26]<<16|Data[0x25]<<8|Data[0x24];
	D1.root_cluster=Data[0x2D]<<8|Data[0x2C];
	D1.fat1_addr=D1.DBR_addr+D1.reverse_sector_num;
	D1.root_addr=D1.fat1_addr+D1.fat_sector_num*2+512*(D1.root_cluster-2);

	UARTSendString("D1.sector_num");
	UARTSendNum(D1.sector_num);
	UARTSendString("D1.reverse_sector_num");
	UARTSendNum(D1.reverse_sector_num);
	UARTSendString("D1.fat_sector_num");
	UARTSendNum(D1.fat_sector_num);
	UARTSendString("D1.root_cluster");
	UARTSendNum(D1.root_cluster);
	UARTSendString("D1.fat1_addr");
	UARTSendNum(D1.fat1_addr);
	UARTSendString("D1.root_addr");
	UARTSendNum(D1.root_addr);


}


void OpenRoot()//�򿪸�Ŀ¼
{
	Data=SDReadBlock(D1.root_addr);
}



u32 CalculateCluster(u32 addr)	//����õ�ַ�Ĵغ�
{
	return (addr-D1.root_addr)/D1.sector_num+2;
}

u32 CalculateAddr(u32 Cluster)//�ôغż�����������
{
	return D1.root_addr+(Cluster-2)*D1.sector_num;			
}

u32 CalculateNextCluster(u32 Cluster)//����ôغ���Fat����Ϊ�ڼ������ĵڼ���,��������һ�ص�ֵ
{
	u8 i;
	u32 addr,next_cluster=0; 
//	UARTSendString("Cluster��fat1��λ��");
	addr=Cluster*4;  //fat��ƫ�Ƶ�ַ
//	UARTSendNum(addr);
//	UARTSendString("Cluster��Fat�ĵڼ�����");
//	UARTSendNum(D1.fat1_addr+addr/512);
//	while(next_cluster==0xff)	//c���ִ���	 ,�ٴζ�ȡ
//	{
		Data=SDReadBlock(D1.fat1_addr+addr/512);
		for(i=0;i<=4;i++)
		{
			next_cluster<<=8;
			next_cluster|=Data[(Cluster*4)%512+4-i];	
		}
//	}
	return next_cluster;		
}




//������
//u16 FindAndOpenTest()//���Ҳ���Test�ļ�
//{
//	u8 a,sector_now=1;
//	u32 addr;
//	Data=SDReadBlock(D1.root_addr+(sector_now-1)*512);
//	if(Data[a*32]==0xE5)   //�Ƿ�ɾ��
//		{
//			if(Data[0x08+a*32]==0x74&Data[0x09+a*32]==0x78&Data[0x0a+a*32]==0x74)//�ҵ�txt�ļ�
//			{
//			 addr=Data[0x15+a*32]<<24||Data[0x14+a*32]<<16||Data[0x1B+a*32]<<8||Data[0x1A+a*32];//��ȡ��ַ
//			}
//		}
//	Data=SDReadBlock(addr);//�򿪸��ļ�
//	return 0;
//  
//}








u8 FindMP3(u32 Cluster) //��ĳĿ¼��Ѱ��MP3�ļ�,����ȡ�ļ���,��ʼ�غźͶ�Ӧ����
{
	u8 a,i;
	u32 next_cluster,addr;
	addr=CalculateAddr(Cluster);
	UARTSendString("���ҵ�ַ");
	UARTSendNum(addr);
	
	Data=SDReadBlock(addr);
	for(a=0;a<16;a++)
	{
		if(song_num==19)	   //����20�׽�������
			return 1;
		if(Data[a*32]!=0xE5&&Data[a*32+0xb]!=0x0F)   //�Ƿ�ɾ�����ļ�����־
		{
			if((Data[0x08+a*32]==0x4D&&Data[0x09+a*32]==0x50&&Data[0x0a+a*32]==0x33)||(Data[0x08+a*32]==0x57&&Data[0x09+a*32]==0x41&&Data[0x0a+a*32]==0x56))//�ҵ�mp3�ļ� 
			{
				UARTSendString("�ҵ�����");
				M1[song_num].cluster=0;
				for(i=0;i<8;i++)
				{
					M1[song_num].name[i]=Data[i+a*32];
				}
				M1[song_num].cluster=(u32)Data[0x15+a*32]<<24|(u32)Data[0x14+a*32]<<16|(u32)Data[0x1B+a*32]<<8|(u32)Data[0x1A+a*32];
				UARTSendString("��Ϊ");
				UARTSendNum(M1[song_num].cluster);
				//ռ���˶�������
				M1[song_num].occupied_sectors=((u32)Data[0x1f+a*32]<<24|(u32)Data[0x1e+a*32]<<16|(u32)Data[0x1d+a*32]<<8|(u32)Data[0x1c+a*32])/512;

				song_num++;
			}
		}
	}
		//����������������
	next_cluster=CalculateNextCluster(Cluster);	
	if(next_cluster==0x0fffffff)	
		return 1;
	else{ 
		FindMP3(next_cluster);
		return 0;
		}
		
}

u8 OpenFile(u32 Cluster)//�򿪸ô�
{
	Data=SDReadBlock(CalculateAddr(Cluster));
	return 1;
}

u32 OpenNextCluster(u32 Cluster)//�򿪵�ǰ�ص���һ����
{
	u32 NextCluster=0;
	NextCluster=CalculateNextCluster(Cluster);
	if(NextCluster==0x0fffff)//Ϊ���һ��
		return 0;
	else 
	{
		Data=SDReadBlock(CalculateAddr(NextCluster));	  //�򿪸ôص�������
		return 1;
	}
}


//void OpenMusicFile(u8 i)	//��mp3��������
//{
//	Data=SDReadBlock(M1[i].addr);
//}

