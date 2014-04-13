/*
     APP_808 .h
*/
#ifndef  _APP_808
#define   _APP_808

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include <App_moduleConfig.h>
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>
#include  "gps.h"


// 1  : connect  not find      0:  not  find      2: connect find  
#define  USB_NOTCONNECT                        0
#define  USB_CONNECT_NOTFIND              1
#define  USB_FIND                                     2 



typedef  struct  _APP_QUE
{
   u32  write_num;
   u32  read_num;
   u8    sd_enable_flag;   // ���ͺ� 1  ���յ�Ӧ������λΪ 0 
   u8    re_send_flag;  //   ���·��ͱ�־λ
   u8    abnormal_counter;// ���ͺ���ȣ����ղ���Ӧ��
   u8    send_timer;


}APP_QUE;

extern APP_QUE  AppQue;
extern u8  Udisk_Test_workState;  //  Udisk ����״̬ 
extern u32  sec_num;


extern   rt_device_t   Udisk_dev;
extern   u8  Udisk_filename[30];  
extern   int  udisk_fd;   
extern   u16   AD_Volte;
extern   u16   AD_2through[2]; //  ����2 ·AD ����ֵ 

 //   ä������
#define MQ_INFO_SIZE 4096
extern uint8_t					MQ_rawinfo[MQ_INFO_SIZE];
extern struct rt_messagequeue	mq_MQBuBao;

//  ��Ϣ���� ˳��������

extern uint8_t					SD_rawinfo[MQ_INFO_SIZE];
extern struct rt_messagequeue	mq_BDsd;
extern LENGTH_BUF BDsd_tx;
extern LENGTH_BUF BDsd_rx;  
extern u8	BD_sequence_send_enable;	//	 ��������˳����



extern u8      Udisk_Find(void);
extern void    Protocol_app_init(void);
extern void  SensorPlus_caculateSpeed (void);
extern void  App_rxGsmData_SemRelease(u8* instr, u16 inlen,u8 link_num);

//          System  reset  related  
extern void  System_Reset(void);
extern  void  reset(void);  

extern void   UDisk_Write_Test(void);

extern  void  MainPower_Recover_process(void);
extern  void  MainPower_cut_process(void); 
extern  void  Recorder_init(u8 value); 
extern  void  Rcorder_Recover(void);

#endif