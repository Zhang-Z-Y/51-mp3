#include<fat.h>
#include<Usart.h>

extern u8 xdata *Data;
extern u8 song_num;
extern struct DBR//DBR信息		//DBR是由硬盘MBR装载的程序段
{
	u16 DBR_addr;				//DBR的扇区号,磁盘的0号扇区的 0X1C6~0X1C9
	u16 reverse_sector_num;		//保留扇区数,DBR扇区，偏移0X0E~0X0F
	u8 sector_num;				//每簇所占扇区数目,DBR的偏移0X0D
	u16 fat1_addr;				 //FAT1起始扇区,也就是fat1表地址,FAT1起始扇区=DBR的扇区号+保留扇区数
	u32 fat_sector_num;			//一个FAT的扇区数,也是每个fat表占用扇区数,从DBR的偏移0X24~0X27得到
	u16 root_cluster;			 //起始簇号,根目录所在簇号,DBR的偏移0X2C~0X2F得到,一般为2.
	u32 root_addr;				 //根目录起始扇区=FAT1的起始扇区号+一个FAT的扇区数×2+(起始簇号-2)x每簇的扇区数
}D1;

//u16 FindDBR()//获取DBR扇区号
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

extern struct Music //mp3文件信息
{
	u8 name[8];	   
	u32 cluster;
	u32 occupied_sectors;	
}M1[20];

void fatInit()//Fat初始化
{

	Data=SDReadBlock(0);
	if(Data[0]==0xeb&&Data[1]==0x58&&Data[2]==0x90&&Data[3]==0x4d) //无MBR
		D1.DBR_addr=0;
	else{
		D1.DBR_addr=Data[0x1c7]<<8|Data[0x1c6];
	}
	UARTSendString("DBR");
	UARTSendNum(D1.DBR_addr);
//	UARTSendNum(D1.DBR_addr);
//	UARTSendString("DBR");
//	SDReadBlock(D1.DBR_addr);
//	UARTSendString("第一段");
//	for(i=0;i<0x0d;i++)
//		UARTSendNum(SDRead());
//	D1.sector_num=SDRead();		  //0x0d
//	D1.reverse_sector_num=SDRead();	  //0x0e
//	D1.reverse_sector_num|=SDRead()<<8;  //0x0f
//	UARTSendString("第二段");
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



	Data=SDReadBlock(D1.DBR_addr);//读取DBR数据
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


void OpenRoot()//打开根目录
{
	Data=SDReadBlock(D1.root_addr);
}



u32 CalculateCluster(u32 addr)	//计算该地址的簇号
{
	return (addr-D1.root_addr)/D1.sector_num+2;
}

u32 CalculateAddr(u32 Cluster)//用簇号计算所在扇区
{
	return D1.root_addr+(Cluster-2)*D1.sector_num;			
}

u32 CalculateNextCluster(u32 Cluster)//计算该簇号在Fat表中为第几扇区的第几个,并返回下一簇的值
{
	u8 i;
	u32 addr,next_cluster=0; 
//	UARTSendString("Cluster在fat1的位置");
	addr=Cluster*4;  //fat表偏移地址
//	UARTSendNum(addr);
//	UARTSendString("Cluster在Fat的第几分区");
//	UARTSendNum(D1.fat1_addr+addr/512);
//	while(next_cluster==0xff)	//c出现错误	 ,再次读取
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




//测试用
//u16 FindAndOpenTest()//查找并打开Test文件
//{
//	u8 a,sector_now=1;
//	u32 addr;
//	Data=SDReadBlock(D1.root_addr+(sector_now-1)*512);
//	if(Data[a*32]==0xE5)   //是否被删除
//		{
//			if(Data[0x08+a*32]==0x74&Data[0x09+a*32]==0x78&Data[0x0a+a*32]==0x74)//找到txt文件
//			{
//			 addr=Data[0x15+a*32]<<24||Data[0x14+a*32]<<16||Data[0x1B+a*32]<<8||Data[0x1A+a*32];//获取地址
//			}
//		}
//	Data=SDReadBlock(addr);//打开该文件
//	return 0;
//  
//}








u8 FindMP3(u32 Cluster) //在某目录中寻找MP3文件,并读取文件名,起始簇号和对应长度
{
	u8 a,i;
	u32 next_cluster,addr;
	addr=CalculateAddr(Cluster);
	UARTSendString("所找地址");
	UARTSendNum(addr);
	
	Data=SDReadBlock(addr);
	for(a=0;a<16;a++)
	{
		if(song_num==19)	   //超过20首结束搜索
			return 1;
		if(Data[a*32]!=0xE5&&Data[a*32+0xb]!=0x0F)   //是否被删除或长文件名标志
		{
			if((Data[0x08+a*32]==0x4D&&Data[0x09+a*32]==0x50&&Data[0x0a+a*32]==0x33)||(Data[0x08+a*32]==0x57&&Data[0x09+a*32]==0x41&&Data[0x0a+a*32]==0x56))//找到mp3文件 
			{
				UARTSendString("找到音乐");
				M1[song_num].cluster=0;
				for(i=0;i<8;i++)
				{
					M1[song_num].name[i]=Data[i+a*32];
				}
				M1[song_num].cluster=(u32)Data[0x15+a*32]<<24|(u32)Data[0x14+a*32]<<16|(u32)Data[0x1B+a*32]<<8|(u32)Data[0x1A+a*32];
				UARTSendString("簇为");
				UARTSendNum(M1[song_num].cluster);
				//占用了多少扇区
				M1[song_num].occupied_sectors=((u32)Data[0x1f+a*32]<<24|(u32)Data[0x1e+a*32]<<16|(u32)Data[0x1d+a*32]<<8|(u32)Data[0x1c+a*32])/512;

				song_num++;
			}
		}
	}
		//整个扇区搜索结束
	next_cluster=CalculateNextCluster(Cluster);	
	if(next_cluster==0x0fffffff)	
		return 1;
	else{ 
		FindMP3(next_cluster);
		return 0;
		}
		
}

u8 OpenFile(u32 Cluster)//打开该簇
{
	Data=SDReadBlock(CalculateAddr(Cluster));
	return 1;
}

u32 OpenNextCluster(u32 Cluster)//打开当前簇的下一个簇
{
	u32 NextCluster=0;
	NextCluster=CalculateNextCluster(Cluster);
	if(NextCluster==0x0fffff)//为最后一簇
		return 0;
	else 
	{
		Data=SDReadBlock(CalculateAddr(NextCluster));	  //打开该簇的首扇区
		return 1;
	}
}


//void OpenMusicFile(u8 i)	//打开mp3的首扇区
//{
//	Data=SDReadBlock(M1[i].addr);
//}

