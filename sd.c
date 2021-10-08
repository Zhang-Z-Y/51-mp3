#include<sd.h>
#include<Usart.h>

u8 xdata Block[512];//速度较快
u8 xdata *Data;
u8 SD_type;//SD卡类型,0为SD,1为SDHC,2为SD1.0,3为MMC,4为不支持


//SD发送指令  cmd 指令  argu 参数/地址  CRC 校验码
//在SPI驱动模式下,不需要CRC校验,只需要对CMD0进行CRC
u8 SDCmd(u8 cmd,u32 argu,u8 crc)
{
	u8 resp,i;
	SPIWrite(cmd|0x40);	//最高的两位为0 1，这个是所有命令中约定的起始方式
	SPIWrite((u8)(argu>>24));
	SPIWrite((u8)(argu>>16&0xff));
	SPIWrite((u8)(argu>>8&0xff));
	SPIWrite((u8)argu&0xff);
	SPIWrite(crc);
	
	i=30;
	do{
		resp=SPIRead();
		i--;
		if(i==0)
			return 0;//响应超时
	}while(resp==0xff);
	return resp;
}

//SD卡初始化
u8 SDInit()
{
	u8 i;
	u8 resp=0;
	SCLK = 1;
	SD_CS=1;
	MOSI = 1;
	MISO = 1;
	// 初始化时钟
	for (i = 0; i < 10; i++) 
		SDWrite(0xff);
	SCLK = 0;
	//复位SD卡
//	DelayMs(1);
	i=30;
//	SD_CS=1;
	SPIWrite(0xff);	
	SD_CS=0;
	do
	{
		resp=SDCmd(CMD0,0,0x95);		
		i--;
		if(i==0) return 0;  //复位失败
		UARTSendNum(resp);
	}while(resp!=0x01);
	SD_CS=1;

	
//	SD_CS=1;//关片选
//	for(i=0;i<250;i++)
//		SPIWrite(0xff);//74以上脉冲
//	SD_CS=0;//开片选

//	while(resp!=0x01)//检测SD卡是否应答,应答信号0x01
//	{
//	SDCmd(CMD0,0,0x95);//仅初始化需要CRC
//	resp=SPIRead();
//	i++;
//	if(i>200)//200次无应答,返回0
//		return 0;
//	}

//	DelayMs(1);
//	SD_CS=1;
	SPIWrite(0xff);
	SD_CS=0;
	resp=SDCmd(CMD8,0x1aa,0x87);
	SPIRead();
	SPIRead();
	SPIRead();
	SD_CS=1;
	//初始化
	UARTSendString("CMD8回应为");
	UARTSendNum(resp);


	if(resp==0x01)	//检测是否为SD2.0
	{
	UARTSendString("为SD2.0");
		i=30;
		do
		{
//		SD_CS=1;
//		SPIWrite(0xff);
		SD_CS=0;
		SDCmd(CMD55,0,0xff);
		resp=SDCmd(ACMD41,0,0xff);		  //0x40000000
		SD_CS=1;
		i--;
		if(i==0)
			return 0;//ACMD命令初始化未完成
		}while(resp!=0);
			
		//检查是否为SDHC,即内存容量大于4G
		SPIWrite(0xff);
		SD_CS=0;
		if(0==SDCmd(58,0,0xff))
		{
		resp=SPIRead();
		SPIRead();
		SPIRead();
		SPIRead();
		}
		SD_CS=1;
		UARTSendNum(resp);
		if(resp&0x40)
			SD_type=1; //SDHC为扇区寻址
		else
			SD_type=0;	
	}

	if(resp==0x05)	//检测SD卡是否为SD1.0或MMC	 ==0x05
	{
//		SD_CS=1;
		SPIWrite(0xff);
		SD_CS=0;
		i=30;
		SDCmd(CMD55,0x0,0xff);
		resp=SDCmd(ACMD41,0,0xff);
		if(resp<=1)		 //41指令被接受
		{
			SD_type=2;	//SD1.0
			while(resp&0x01!=0)
			{
			SDCmd(CMD55,0x0,0xff);
			resp=SDCmd(ACMD41,0,0xff);
			}
			SD_CS=1;
		}else{		//MMC卡,用cmd1进行初始化
		i=30;
		do
		{
			i--;
			resp=SDCmd(CMD1,0x0,0xff);
			if(i==0)
				return 0;//MMD初始化失败
		}while(resp!=0);
			if(!resp)
				SD_type=3;//SD_mmc	
			else
				SD_type=4;//不支持	
		}
		SD_CS=1;
		SPIWrite(0xff);

	}
	if(0!=SDCmd(16,0x200,0xff))
	{
		SD_type=4;//该卡扇区并不为512字节
		return 0;
	}
		
 
//	//sd卡设置
//	SD_CS=1;
//	SPIWrite(0xff);
//	SD_CS=0;
//	resp=0x00;
//	i=0;
//	while(resp!=0x01)//检测SD卡是否应答,应答信号0x01
//	{
//	SDCmd(CMD1,0,0xff); //设置为SPI模式
//	resp=SPIRead();
//	i++;
//	if(i>200)//200次无应答,返回2
//		return 2;
//	}
//	SD_CS=1;
//	SPIWrite(0xff);
	return 1;
}



void SDWrite(u8 dat)
{
	SCLK=0;
	SD_CS=0;
	SPIWrite(dat);
	SD_CS=1;
}


u8 SDRead()
{
	u8 dat=0;
	SCLK=0;
	SD_CS=0;
	dat=SPIRead();
	SD_CS=1;
	return dat;	
}

//读取块,返回数组首地址	**********读取块命令,89c52ram不够用,后面需手动读取512个字节和2字节的crc	, 并拉高CS,8个时钟
u8 *SDReadBlock(u32 addr)
{
	u16 i;
	u8 tmp=0xff;
	SPIWrite(0xff);
	SD_CS=0;	
	if(SD_type!=1)
		addr<<=9;//addr=addr*512 将块地址转为字节地址,也可以用addr<<=9
//	SD_CS=1;
//	SPIWrite(0xff);
//	SD_CS=0;
//	SDCmd(CMD17,addr,0xff);//读取指令 
//	i=30;
//	tmp=0;
//	while(tmp!=0xfe)
//	{
//		tmp=SDRead();
//
//	}
	i=30;
		tmp=SDCmd(CMD17,addr,0xFF); //CMD17
	while(tmp!=0xfe)
	{
		tmp=SDRead();
		i--;
		if(i==0)
		{
		return 0;
		}
	}
	for(i=0;i<512;i++)
	{
		Block[i]=SDRead();//读出数据
	}
	SDRead();
	SDRead();	//16位的crc校验码
	SD_CS=1;
	SPIWrite(0xff);
	return Block;		
}

////写块,将指向的512个字节写入addr中
//void SDWriteBlock(u32 addr,u8 *dat)
//{
//
//}


void SDReadBlockHalf(u32 addr)	 //手动读块
{
	u16 i;
	u8 tmp=0xff;
	SPIWrite(0xff);
	SD_CS=0;	
	if(SD_type!=1)
		addr<<=9;//addr=addr*512 将块地址转为字节地址,也可以用addr<<=9
	SDCmd(CMD17,addr,0xff);//读取指令 
	i=30;
	tmp=0;
	while(tmp!=0xfe)
	{
		tmp=SDRead();
	}
}