#ifndef __FAT_H__
#define __FAT_H__

#include<sd.h>

//extern u8 xdata Block[512];
//extern u8 xdata *Data;


//u16 FindDBR();
void fatInit();
void OpenRoot();
u32 CalculateCluster(u32 addr);
u32 CalculateAddr(u32 Cluster);
u32 CalculateNextCluster(u32 Cluster);
u8 OpenFile(u32 Cluster);
u16 FindAndOpenTest();
u8 FindMP3(u32 Cluster);
u32 OpenNextCluster(u32 addr);

#endif