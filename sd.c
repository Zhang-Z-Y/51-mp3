#include<sd.h>
#include<Usart.h>

u8 xdata Block[512];//�ٶȽϿ�
u8 xdata *Data;
u8 SD_type;//SD������,0ΪSD,1ΪSDHC,2ΪSD1.0,3ΪMMC,4Ϊ��֧��


//SD����ָ��  cmd ָ��  argu ����/��ַ  CRC У����
//��SPI����ģʽ��,����ҪCRCУ��,ֻ��Ҫ��CMD0����CRC
u8 SDCmd(u8 cmd,u32 argu,u8 crc)
{
	u8 resp,i;
	SPIWrite(cmd|0x40);	//��ߵ���λΪ0 1�����������������Լ������ʼ��ʽ
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
			return 0;//��Ӧ��ʱ
	}while(resp==0xff);
	return resp;
}

//SD����ʼ��
u8 SDInit()
{
	u8 i;
	u8 resp=0;
	SCLK = 1;
	SD_CS=1;
	MOSI = 1;
	MISO = 1;
	// ��ʼ��ʱ��
	for (i = 0; i < 10; i++) 
		SDWrite(0xff);
	SCLK = 0;
	//��λSD��
//	DelayMs(1);
	i=30;
//	SD_CS=1;
	SPIWrite(0xff);	
	SD_CS=0;
	do
	{
		resp=SDCmd(CMD0,0,0x95);		
		i--;
		if(i==0) return 0;  //��λʧ��
		UARTSendNum(resp);
	}while(resp!=0x01);
	SD_CS=1;

	
//	SD_CS=1;//��Ƭѡ
//	for(i=0;i<250;i++)
//		SPIWrite(0xff);//74��������
//	SD_CS=0;//��Ƭѡ

//	while(resp!=0x01)//���SD���Ƿ�Ӧ��,Ӧ���ź�0x01
//	{
//	SDCmd(CMD0,0,0x95);//����ʼ����ҪCRC
//	resp=SPIRead();
//	i++;
//	if(i>200)//200����Ӧ��,����0
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
	//��ʼ��
	UARTSendString("CMD8��ӦΪ");
	UARTSendNum(resp);


	if(resp==0x01)	//����Ƿ�ΪSD2.0
	{
	UARTSendString("ΪSD2.0");
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
			return 0;//ACMD�����ʼ��δ���
		}while(resp!=0);
			
		//����Ƿ�ΪSDHC,���ڴ���������4G
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
			SD_type=1; //SDHCΪ����Ѱַ
		else
			SD_type=0;	
	}

	if(resp==0x05)	//���SD���Ƿ�ΪSD1.0��MMC	 ==0x05
	{
//		SD_CS=1;
		SPIWrite(0xff);
		SD_CS=0;
		i=30;
		SDCmd(CMD55,0x0,0xff);
		resp=SDCmd(ACMD41,0,0xff);
		if(resp<=1)		 //41ָ�����
		{
			SD_type=2;	//SD1.0
			while(resp&0x01!=0)
			{
			SDCmd(CMD55,0x0,0xff);
			resp=SDCmd(ACMD41,0,0xff);
			}
			SD_CS=1;
		}else{		//MMC��,��cmd1���г�ʼ��
		i=30;
		do
		{
			i--;
			resp=SDCmd(CMD1,0x0,0xff);
			if(i==0)
				return 0;//MMD��ʼ��ʧ��
		}while(resp!=0);
			if(!resp)
				SD_type=3;//SD_mmc	
			else
				SD_type=4;//��֧��	
		}
		SD_CS=1;
		SPIWrite(0xff);

	}
	if(0!=SDCmd(16,0x200,0xff))
	{
		SD_type=4;//�ÿ���������Ϊ512�ֽ�
		return 0;
	}
		
 
//	//sd������
//	SD_CS=1;
//	SPIWrite(0xff);
//	SD_CS=0;
//	resp=0x00;
//	i=0;
//	while(resp!=0x01)//���SD���Ƿ�Ӧ��,Ӧ���ź�0x01
//	{
//	SDCmd(CMD1,0,0xff); //����ΪSPIģʽ
//	resp=SPIRead();
//	i++;
//	if(i>200)//200����Ӧ��,����2
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

//��ȡ��,���������׵�ַ	**********��ȡ������,89c52ram������,�������ֶ���ȡ512���ֽں�2�ֽڵ�crc	, ������CS,8��ʱ��
u8 *SDReadBlock(u32 addr)
{
	u16 i;
	u8 tmp=0xff;
	SPIWrite(0xff);
	SD_CS=0;	
	if(SD_type!=1)
		addr<<=9;//addr=addr*512 �����ַתΪ�ֽڵ�ַ,Ҳ������addr<<=9
//	SD_CS=1;
//	SPIWrite(0xff);
//	SD_CS=0;
//	SDCmd(CMD17,addr,0xff);//��ȡָ�� 
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
		Block[i]=SDRead();//��������
	}
	SDRead();
	SDRead();	//16λ��crcУ����
	SD_CS=1;
	SPIWrite(0xff);
	return Block;		
}

////д��,��ָ���512���ֽ�д��addr��
//void SDWriteBlock(u32 addr,u8 *dat)
//{
//
//}


void SDReadBlockHalf(u32 addr)	 //�ֶ�����
{
	u16 i;
	u8 tmp=0xff;
	SPIWrite(0xff);
	SD_CS=0;	
	if(SD_type!=1)
		addr<<=9;//addr=addr*512 �����ַתΪ�ֽڵ�ַ,Ҳ������addr<<=9
	SDCmd(CMD17,addr,0xff);//��ȡָ�� 
	i=30;
	tmp=0;
	while(tmp!=0xfe)
	{
		tmp=SDRead();
	}
}