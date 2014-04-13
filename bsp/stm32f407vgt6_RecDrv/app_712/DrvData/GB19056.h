/*
     GB19056.h
*/
#ifndef  GB_19056
#define  GB_19056
#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//数字转换成字符串
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>
//#include "usbh_usr.h"

//     DRV  CMD
  //  采集命令
#define  GB_SAMPLE_00H    0x00
#define  GB_SAMPLE_01H    0x01
#define  GB_SAMPLE_02H    0x02
#define  GB_SAMPLE_03H    0x03
#define  GB_SAMPLE_04H    0x04
#define  GB_SAMPLE_05H    0x05
#define  GB_SAMPLE_06H    0x06
#define  GB_SAMPLE_07H    0x07
#define  GB_SAMPLE_08H    0x08
#define  GB_SAMPLE_09H    0x09
#define  GB_SAMPLE_10H    0x10
#define  GB_SAMPLE_11H    0x11
#define  GB_SAMPLE_12H    0x12
#define  GB_SAMPLE_13H    0x13
#define  GB_SAMPLE_14H    0x14
#define  GB_SAMPLE_15H    0x15
   //   设置命令
#define  GB_SET_82H      0x82
#define  GB_SET_83H      0x83
#define  GB_SET_84H      0x84
#define  GB_SET_82H      0x82
#define  GB_SET_C2H      0x82
#define  GB_SET_C3H      0x83
#define  GB_SET_C4H      0x84















typedef  struct  _GB_STRKT
{
   u8  workstate;  // 记录仪串口工作模式  1:enable  2:disable
   u8  RX_CMD;   //  接收命令字
   u8  TX_CMD;   //  发送命令字
   u8  RX_FCS;   //   接收校验  
   u8  TX_FCS;   //   发送校验
   u8  rx_buf[128]; //接收字符
   u16  rx_infoLen; //信息块长度
   u8  rx_wr; // 
   u8  rx_flag;

   //---------usb -----------
   u8  usb_exacute_output;  // 使能输出     0 : disable     1:  enable    2:  usb output   working 
   u8  usb_out_selectID; //  idle:  FF    0x00~0x15     0xFE :out put all        0xFB : output  recommond info  
                           /*
                                                        Note:  Recommond info  include  below : 
                                                           0x00-0x05 + 10 (latest)
                                                */
   u8 Usbfilename[30]; 
   u8 usb_write_step;  //  写入操作步骤						   


						   
 }GB_STRKT;


extern GB_STRKT GB19056;
extern struct rt_semaphore GB_RX_sem;  //  gb  接收AA 75





#endif
