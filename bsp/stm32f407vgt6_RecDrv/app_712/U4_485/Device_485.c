/*
    App Gsm uart
*/

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



 //---------------变量声明-------------------------------------------
    //第四个字节  0x81: 320x240      0x82 : 640 x480
 u8   Take_photo[10]={0x40,0x40,0x61,0x81,0x02,0X00,0X00,0X02,0X0D,0X0A}; ;   //----  报警拍照命令
 u8   Fectch_photo[10]={0x40,0x40,0x62,0x81,0x02,0X00,0XFF,0XFF,0X0D,0X0A};;   //----- 报警取图命令 


 u8    _485_CameraData_Enable=0;// 有图片数据过来   1: data come  0:   no data 
 u8 	 _485_content[600];
 u16	 _485_content_wr=0;


 
static  u16	 PackageLen=0;//记录每次接收的byte数  
 u8  OpenDoor_StartTakeFlag=0; // 车开关开始拍照状态 ，	开始拍照时	set   1   拍照结束后  0 
 u8   Opendoor_transFLAG=0x02;	   //	车门打开拍照后是否上传标志位  

#ifdef  LCD_5inch
  LARGELCD     DwinLCD;     
#endif

/* 定时器的控制块 */
 static rt_timer_t timer_485; 
 static u8 One_second_Counter_485=0; 
//----------- _485 rx-----
ALIGN(RT_ALIGN_SIZE)
u8    _485_dev_rx[_485_dev_SIZE];       
u16  _485dev_wr=0;      




 struct rt_device  Device_485;
#define                                      _485_MsgQueStack_SIZE  512
static uint8_t					 _485_MsgQueStack[_485_MsgQueStack_SIZE];
static struct rt_messagequeue	 _485_MsgQue;  
MSG_Q_TYPE  _485_MsgQue_sruct;    

_485REC_Struct 	 _485_RXstatus;	  


#ifdef LCD_5inch
void  DwinLCD_Send(void)
{
     u8  i=0;
     u8  send[300];	 
	 
       memset(DwinLCD.Txinfo,0,sizeof(DwinLCD.Txinfo));
     	DwinLCD.TxInfolen=0;
       DwinLCD.Txinfo[0]=0xAA;
 	DwinLCD.Txinfo[1]=0x55;
 	
	switch (DwinLCD.Type)
	{   
	    case  LCD_SETTIME:    //  授时  
	                          /*  AA 55 0A 80 1F 5A 12 09 01 03 11 53 01     -----授时
                                        注： 12 09 01 03 11 53 01 表示  12年 09月01日 星期3 11:53:01
	                          */
	                        DwinLCD.Txinfo[2]=0x0A; 
			          DwinLCD.Txinfo[3]=0x80;
				   DwinLCD.Txinfo[4]=0x1F;  //  寄存器 
			          DwinLCD.Txinfo[5]=0x5A;	  
				   DwinLCD.Txinfo[6]=((((time_now.year))/10)<<4)+(((time_now.year))%10);  	   	
				   DwinLCD.Txinfo[7]=((time_now.month/10)<<4)+(time_now.month%10);  
				   DwinLCD.Txinfo[8]=((time_now.day/10)<<4)+(time_now.day%10); 
				   DwinLCD.Txinfo[9]=0x01; 
				   DwinLCD.Txinfo[10]=((time_now.hour/10)<<4)+(time_now.hour%10);
			          DwinLCD.Txinfo[11]=((time_now.min/10)<<4)+(time_now.min%10);
				   DwinLCD.Txinfo[12]=((time_now.sec/10)<<4)+(time_now.sec%10);       
				    rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,13);		
				  break;	  
	    case  LCD_SETSPD:   //   速度方向
	                        // AA 55 07 82 00 01 01 30  00 F3 
	                        DwinLCD.Txinfo[2]=0x05; 
                               DwinLCD.Txinfo[3]=0x82;
				   DwinLCD.Txinfo[4]=0x10;
 	                        DwinLCD.Txinfo[5]=0x00;
 	                        DwinLCD.Txinfo[6]=((Speed_gps/10)>>8);     // 速度
 	                        DwinLCD.Txinfo[7]=(u8)(Speed_gps/10);  
				  // DwinLCD.Txinfo[8]=(GPS_direction>>8);	;   //  方向 
 	                       // DwinLCD.Txinfo[9]=GPS_direction; 			
				   rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,8);				
			   
			         break;
	    case  LCD_SDTXT:  
			        //  UART4_output_LongData("\xAA\x55\x05\x82\x10\x00\xFF\xFF",8  );   //  clear screen      
				    WatchDog_Feed();	
			          DwinLCD.Txinfo[2]=5+DwinLCD.TXT_contentLen;   // 长度 
                               DwinLCD.Txinfo[3]=0x82;
				   DwinLCD.Txinfo[4]=0x00;  // LCD ID  
 	                        DwinLCD.Txinfo[5]=0x05;
				   memcpy(DwinLCD.Txinfo+6,DwinLCD.TXT_content,DwinLCD.TXT_contentLen);  
				   DwinLCD.Txinfo[DwinLCD.TXT_contentLen+6]=0xFF;
				   DwinLCD.Txinfo[DwinLCD.TXT_contentLen+7]=0xFF;	
				   //UART4_output_LongData((const char*)DwinLCD.Txinfo,8+DwinLCD.TxInfolen);    
				   DwinLCD.TxInfolen=DwinLCD.TXT_contentLen; 
				   rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,8+DwinLCD.TxInfolen); 	
				    //   rt_thread_delay(8);
				   WatchDog_Feed();
				    delay_ms(20);
					
  
			         break;	   
	    case  LCD_GPSNUM:  //  GPS 信息   AA 55 05 82 10 01 00 03  最大为20
	                        DwinLCD.Txinfo[2]=5;   // 长度 
                               DwinLCD.Txinfo[3]=0x82;
				   DwinLCD.Txinfo[4]=0x10;  // LCD ID  
 	                        DwinLCD.Txinfo[5]=0x01; 
				   DwinLCD.Txinfo[6]=0x00;  //Data
				   DwinLCD.Txinfo[7]=Satelite_num; 

				    rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,8); 	

			         break;
	   case    LCD_GSMNUM://   GSM 信息  AA 55 05 82 10 02 00 09  最大为31
	                        DwinLCD.Txinfo[2]=5;   // 长度 
                               DwinLCD.Txinfo[3]=0x82;
				   DwinLCD.Txinfo[4]=0x10;  // LCD ID  
 	                        DwinLCD.Txinfo[5]=0x02;  
				   DwinLCD.Txinfo[6]=0x00;  //Data
				   DwinLCD.Txinfo[7]=ModuleSQ;   

				    rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,8); 	  
		                break;
	  case   LCD_VechInfo://  发送车辆信息要拼文本信息

                               DwinLCD.Txinfo[2]=5;   // 长度 
                               DwinLCD.Txinfo[3]=0x82;
				   DwinLCD.Txinfo[4]=0x37;  // LCD ID   
 	                        DwinLCD.Txinfo[5]=0x00;  
				                          //Data
				   memset(send,0,sizeof((const char*)send));                 
				    sprintf(send,"\r\nSocket: %d.%d.%d.%d : %d\r\n",RemoteIP_main[0],RemoteIP_main[1],RemoteIP_main[2],RemoteIP_main[3],RemotePort_main);
				   memcpy(DwinLCD.Txinfo+6,send,strlen(send));	
				   DwinLCD.TxInfolen=6+strlen(send);   

				    memset(send,0,sizeof((const char*)send));      
				   sprintf(send,"\r\nAPN:   %s\r\n",APN_String); 
                               memcpy(DwinLCD.Txinfo+ DwinLCD.TxInfolen,send,strlen(send));  	 
				   DwinLCD.TxInfolen+=strlen(send);      
				    memset(send,0,sizeof((const char*)send));   
				   sprintf(send,"\r\n车辆入网ID: "); 
				   memcpy(DwinLCD.Txinfo+ DwinLCD.TxInfolen,send,strlen(send));
				   DwinLCD.TxInfolen+=strlen(send);   
				   memcpy(DwinLCD.Txinfo+ DwinLCD.TxInfolen,IMSI_CODE+3,12);   //---- 
				   DwinLCD.TxInfolen+=12;     
				   memset(send,0,sizeof((const char*)send));   
				   sprintf(send,"\r\n\r\n车牌号:   %s",JT808Conf_struct.Vechicle_Info.Vech_Num,strlen((const char*)(JT808Conf_struct.Vechicle_Info.Vech_Num)));
				    memcpy(DwinLCD.Txinfo+ DwinLCD.TxInfolen,send,strlen(send));	
				   DwinLCD.TxInfolen+=strlen(send);    
				   // memset(send,0,sizeof((const char*)send));   
				    //sprintf(send,"\r\n车辆类型: %s",JT808Conf_struct.Vechicle_Info.Vech_Type,strlen((const char*)(JT808Conf_struct.Vechicle_Info.Vech_Type)));
				   // memcpy(DwinLCD.Txinfo+ DwinLCD.TxInfolen,send,strlen(send));	  
				  // DwinLCD.TxInfolen+=strlen(send);    

				   DwinLCD.Txinfo[2]=DwinLCD.TxInfolen-3;  //   补填  
				    rt_device_write(&Device_485,0,( const char*)DwinLCD.Txinfo,DwinLCD.TxInfolen);   	

				         break;
	   default :
	   	                return;

	}
       //-------------------------
   DwinLCD.Type=LCD_IDLE;          
   DwinLCD.TxInfolen=0;  // 信息长度	     
	//------------------------
}
#endif

#ifdef LCD_5inch
 void DwinLCD_init(void) 
 {
   DwinLCD.CounterTimer=0;
   DwinLCD.Enable_Flag=1;      //   初始化Enable
   DwinLCD.Type=LCD_IDLE;          //  速度方向
   DwinLCD.TxInfolen=0;  // 信息长度
   memset(DwinLCD.Txinfo,0,sizeof(DwinLCD.Txinfo));  

 }
#endif

void  DwinLCD_DispTrigger(void)
{
                     #ifdef  LCD_5inch                                              
				if((time_now.sec==30)&&(DwinLCD.Type==LCD_IDLE))  
					     DwinLCD.Type=LCD_SETTIME;   
				else
				if(((time_now.sec%10)==6)&&(DwinLCD.Type==LCD_IDLE))  
				           DwinLCD.Type=LCD_GPSNUM;
			       else
				if((time_now.sec%10==4)&&(DwinLCD.Type==LCD_IDLE))    
  				 	     DwinLCD.Type=LCD_GSMNUM; 
			      if((time_now.sec%5==0)&&(DwinLCD.Type==LCD_IDLE))     
				        DwinLCD.Type=LCD_SETSPD;       
                             #endif  
}

void  DwinLCD_work_Enable(void)
{ 
     DwinLCD.Enable_Flag=1; 
}
void  DwinLCD_work_Disable(void)
{ 
      DwinLCD.Enable_Flag=0; 
}

void  DwinLCD_Timer(void)
{
    #ifdef LCD_5inch
    if(DwinLCD.Enable_Flag==1)  
    {
          DwinLCD.CounterTimer++;
	   if(	 DwinLCD.CounterTimer>=8)
	   	{
                      DwinLCD_Send();  
			 DwinLCD.CounterTimer=0;	    
 	   	}

    }	
    #endif	
}

void  DwinLCD_Data_Process(void) 
{
     u16   ADD_ID=0,i=0; 
     u16   ContentLen=0;// 实体信息长度
     u8    dwin_reg[50],endtailFlag=0;
     
         //AA 55 08 83 30 00 02 1E FF FF 00               
         //      08 是长度       83  : 指令  33 00 : 地址   02 1E FF FF 00: 内容 

       if(!DwinLCD.Process_Enable)
   	      return   ;   
	 DwinLCD.Process_Enable=0; // clear  

     if(DwinLCD.RxInfolen>=3)
     	{
                 ContentLen=DwinLCD.RxInfolen-3;
                 ADD_ID=((u16)DwinLCD.RxInfo[4]<<8)+(u16)DwinLCD.RxInfo[5];   

             //   Get useful  infomation    
                  memset(dwin_reg,0,sizeof(dwin_reg));
		    for(i=0;i<ContentLen;i++)				  
		    	{
		    	     if(DwinLCD.RxInfo[6+i]==0xFF)
			     {		
			         switch(endtailFlag)
				   {   case 0:							   
			                            endtailFlag=1;  // 有一个FF
							break;
					case  1:
						       endtailFlag=2; // 有2个 FF 结束了
						       break;
					default:
                                                       return;   // 异常情况返回不处理
				  }	 
		    	      }		
			    else		 
		    	        dwin_reg[i]=DwinLCD.RxInfo[6+i]; 
			    //----------------------------------	
                                  if(endtailFlag==2)
					break;
		    	}
			 
            switch(ADD_ID)
            	{
                  case 0x3000:// 车牌号输入   AA 55 0E 83 30 00 05 BD F2 41 37 37 38 38 FF FF 00
                                      /* 
                                            输入津A67823输出如下：
                                                         AA 55 0E 83 30 00 05 BD F2 41 36 37 38 32 33 FF FF
                                      */
                                      
                                 /*   
                                       //------------------------------------------------------------ 
					   memset((u8*)&JT808Conf_struct.Vechicle_Info.Vech_Num,0,sizeof(JT808Conf_struct.Vechicle_Info.Vech_Num));	//clear	
					  // memcpy(JT808Conf_struct.Vechicle_Info.Vech_Num,dwin_reg,strlen(dwin_reg));
					   memcpy(JT808Conf_struct.Vechicle_Info.Vech_Num,dwin_reg,7); // 国标规定车牌长度7个字节
					   Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));    
					    //------------------------------------------------------------				   
                                       rt_kprintf("\r\n Dwin设置车牌号:%12s",JT808Conf_struct.Vechicle_Info.Vech_Num); 
					  */  				   
						
				         break;
		    case 0x3200: // APN        AA 55 14 83 32 00 08 77 77 77 2E 62 61 69 64 75 2E 63 6F 6D FF FF 00
                                  /*    memset(APN_String,0,sizeof(APN_String));					  
					  memcpy(APN_String,(char*)dwin_reg,strlen(dwin_reg));  
					  memset((u8*)SysConf_struct.APN_str,0,sizeof(APN_String));	
					  memcpy((u8*)SysConf_struct.APN_str,(char*)dwin_reg,strlen(dwin_reg));    
					 Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));


                                     DataLink_APN_Set(APN_String,1); 
                                     */
			               break;
		    case  0x3300:// IP	        AA 55 12 83 33 00 07 31 39 32 2E 31 36 38 2E 31 2E 31 32 FF FF	   
                                    //-------------------------------------------------------------------------
				      /*  i = str2ipport((char*)dwin_reg, RemoteIP_main, &RemotePort_main);
					  if (i <= 4) break;;
					   
					  memset(dwin_reg,0,sizeof(dwin_reg));
					  IP_Str((char*)dwin_reg, *( u32 * ) RemoteIP_main);		   
					  strcat((char*)dwin_reg, " :"); 	  
					  sprintf((char*)dwin_reg+strlen((const char*)dwin_reg), "%u\r\n", RemotePort_main);  
				         memcpy((char*)SysConf_struct.IP_Main,RemoteIP_main,4);
					  SysConf_struct.Port_main=RemotePort_main;
					 Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));

				        DataLink_MainSocket_set(RemoteIP_main,RemotePort_main,1);
						 DataLink_EndFlag=1; //AT_End();  
					*/	 
                                  //--------------------------------------------------------------------------

                                    break;
		    case  0x0999:// 拍照  AA 55 06 83 09 99 01 00 03
                                   //   DEV_regist.Enable_sd=1; // set 发送注册标志位
                                     // rt_kprintf("\r\n 5 寸屏手动发送注册\r\n");
					 break;
		    case  0x0004://车辆信息 AA 55 06 83 00 04 01 00 02
		                         //                       AA 55 06 83 00 04 01 00 02 
                                     DwinLCD.Type=LCD_VechInfo;  
			               break;
		    case   0x3600:  //一键拨号
                                          Speak_ON;
						if(DispContent==2) 
						 rt_kprintf("\r\n   5 寸屏 一键拨号");  
					
	 					  memset(JT808Conf_struct.LISTEN_Num,0,sizeof(JT808Conf_struct.LISTEN_Num));
	 					  memcpy(JT808Conf_struct.LISTEN_Num,"13051953513",11);  										  
                                            Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));   
                                            CallState=CallState_rdytoDialLis;  // 准备开始拨打监听号码 

				    
				       break;
		     //---------------    Add  later --------------------------------------------			   
		     case   0x4100:  //   入网ID 
		                              //AA 55 12 83 41 00 07 30 31 33 36 30 32 30 35 39 31 39 31 FF FF 
                              
				      break;
		    case    0x4114:  //  车辆类型  ---货运
		                             //   AA 55 06 83 41 14 01 00 06 
                                           				// 车辆类型

			             break;
		     case    0x4118:  //  车辆类型  ---危险
		                              // AA 55 06 83 41 18 01 00 07
			             break;	
                   case    0x4120:  //  车辆类型  ---客运
                                            // AA 55 06 83 41 20 01 00 08
			             break;
		     case    0x4122:  //  车辆类型  ---出租 
		                              //  AA 55 06 83 41 22 01 00 09
			             break;
		      //-------------------------------------------				 
		     case    0x4110:  //  速度类型  ---GPS
		                              //  AA 55 06 83 41 10 01 00 04 
                                            JT808Conf_struct.Speed_GetType=1; 
					         JT808Conf_struct.DF_K_adjustState=1;		
					         ModuleStatus|=Status_Pcheck;
						  				//  存储
				                 Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
	                                   //  rt_kprintf("\r\n 5 寸屏手动速度:GPS\r\n");
			             break;				 
		       case    0x4112:  //  速度类型  ---传感器
		                                //AA 55 06 83 41 12 01 00 05 
                                             JT808Conf_struct.Speed_GetType=0;  
					          JT808Conf_struct.DF_K_adjustState=0;	
					          ModuleStatus&=~Status_Pcheck;
				                 //  存储
				                 Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
                                             // rt_kprintf("\r\n 5 寸屏手动速度:传感器\r\n");		  
			             break;	
			//-------------------------------------------	
			case    0x4105:  //  颜色  ---黑
			                         //  AA 55 06 83 41 05 01 00 01 
			                         Menu_color_num=3;
                                              JT808Conf_struct.Vechicle_Info.Dev_Color=Menu_color_num;
						    Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));			 
                                              // rt_kprintf("\r\n 5 寸屏手动颜色: 黑\r\n");		
						 break;
			case    0x4107:  //  颜色---黄
			                         //AA 55 06 83 41 07 01 00 02   
			                         Menu_color_num=2;
                                              JT808Conf_struct.Vechicle_Info.Dev_Color=Menu_color_num;
						    Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));			 
                                             //  rt_kprintf("\r\n 5 寸屏手动颜色: 黄\r\n");		

			             break;	
			case    0x4108:  //  颜色 ---蓝 
			                         // AA 55 06 83 41 08 01 00 03
			                         Menu_color_num=1;
                                              JT808Conf_struct.Vechicle_Info.Dev_Color=Menu_color_num;
						    Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));			 
                                             // rt_kprintf("\r\n 5 寸屏手动颜色: 蓝 \r\n");		 

			             break;		
			//----------------------------------------
			case   0x4128://   鉴权
			                      // AA 55 06 83 41 28 01 00 12
                                            DEV_Login.Operate_enable=1;			   
			                       DEV_Login.Enable_sd=1; 
						  //  rt_kprintf("\r\n 5 寸屏手动发送鉴权\r\n");		   
			             break;
			case   0x4124://   注册
			                      //  AA 55 06 83 41 24 01 00 00 
                                           DEV_regist.Enable_sd=1; // set 发送注册标志位
                                           // rt_kprintf("\r\n 5 寸屏手动发送注册\r\n");
						//  要发送短信	
						 SMS_send_enable();
			             break;
			case   0x4126: // 清空鉴权			 
                                            // AA 55 06 83 41 26 01 00 11
							
				     break;		 
		    default:
				        break;

            	}


     	}
     else
	 	return;  
}


//=====================================================

void  Photo_TakeCMD_Update(u8 CameraNum)
{
      Take_photo[4]=CameraNum;    // Set take  Camra  Num
      Take_photo[5]=0x00;
}

void  Photo_FetchCMD_Update(u8 CameraNum)
{
     Fectch_photo[4]=CameraNum; // Set  fetch  Camra  Num 
     Fectch_photo[5]=0x00;
}

void _485_delay_us(u16 j)
{
u8 i;
  while(j--)
  	{
  	i=3;
  	while(i--);
  	}  
}
void _485_delay_ms(u16 j)
{
  while(j--)
  	{
         _485_delay_us(1000);   
  	} 
   
}


/* write one character to serial, must not trigger interrupt */
 void rt_hw_485_putc(const char c)
{
	/*
		to be polite with serial console add a line feed
		to the carriage return character
	*/
	//USART_SendData(USART2,  c); 
	//while (!(USART2->SR & USART_FLAG_TXE));  
	//USART2->DR = (c & 0x1FF);  
	USART_SendData(USART2,  c);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){} 
}

void rt_hw_485_output(const char *str)
{
	/* empty console output */
	TX_485const; 
	_485_delay_ms(1);
		//--------  add by  nathanlnw ---------
       while (*str)
	{
		rt_hw_485_putc (*str++);
	}
       //--------  add by  nathanlnw  --------	
       _485_delay_ms(3); 
       RX_485const; 	 
}

void rt_hw_485_Output_Data(const char *Instr, unsigned int len)  
{
        unsigned int  Info_len485=0;

	Info_len485=len;
    	/* empty console output */
	TX_485const; 
	_485_delay_ms(1);
		//--------  add by  nathanlnw ---------
       while (Info_len485)
	{
		rt_hw_485_putc (*Instr++); 
		Info_len485--;
	}
       //--------  add by  nathanlnw  --------	
       _485_delay_ms(3); 
       RX_485const; 	 
}

static u8  CHKendTake_ReadyToSend(void)
{ /*  结束多路拍照，同时触发上报处理   */


    if(Max_CameraNum==Camera_Number) 
    {
		  /*
			  Taking End, Start Transfering 
		   */
		   MultiTake.Taking=0;	// Taking  State  Over	   
		   
		   //------ 判断车门开关拍照是否上传状态 ------
		   if((OpenDoor_StartTakeFlag==1)&&(Opendoor_transFLAG==0))
			{
				 MultiTake_End();	// 车门开关不上传
				 OpenDoor_StartTakeFlag=0;
			} 
		   else
				Check_MultiTakeResult_b4Trans();

		 return true;  
     }
	else
		 return false;
}


void  _485_RxHandler(u8 data)
{
    //      Large   LCD   Data 
 
      _485_dev_rx[_485dev_wr++]=data;
	

     switch(_485_RXstatus._485_receiveflag)
     	{
	     case  IDLE_485:
		 	                  if((0xAA!=_485_dev_rx[0])&&(0x40!=_485_dev_rx[0]))
					    {	  //    判断 第一个字节是否合法，否则直接清除	
					          _485dev_wr=0;
						   break;	  
		 	                  }								  
					   switch (_485dev_wr)	 
					     	{   
                                            case  2 :   if((0x55!=_485_dev_rx[1])&&(0x40!=_485_dev_rx[1])) 
										_485dev_wr=0; 												
							             break;
					     	  case   3:
							     	      //----------  Check  LCD  --------------------------
							            if((0xAA==_485_dev_rx[0])&&(0x55==_485_dev_rx[1]))  
							            {     //AA 55 08 83 30 00 02 1E FF FF 00                     08 是长度(包含1BYTE 指令2 BYTES  地址)
									    _485_RXstatus._485_RxLen=_485_dev_rx[2];
									    _485_RXstatus._485_receiveflag=LARGE_LCD; 			
							            }	
								     else
								      if(0x40!=_485_dev_rx[0])	
									   _485dev_wr=0; 	
							        	break;
						 case    8:      //        -------  Camera  Data  -----------
								     if((0x40==_485_dev_rx[0])&&(0x40==_485_dev_rx[1])&&(0x63==_485_dev_rx[2])) 
								     {     //  40  40  63  FF  FF  00  02           00  02  是长度  (小端)
		                                                       _485_RXstatus._485_RxLen=((u16)_485_dev_rx[6]<<8)+(u16)_485_dev_rx[5];
									     //----------------------------------  						   
									     _485dev_wr=0;  //clear now  , the bytes  receiving  later  is  pic data 	
									     _485_RXstatus._485_receiveflag=CAMERA_Related;  //  return Idle
								     }	  
								     else
									 	_485dev_wr=0;   // clear
								     break;	 
					     	}
                                       
						
			               break;
	   case  LARGE_LCD:
                                        if(_485dev_wr>=(_485_RXstatus._485_RxLen+3))  //  AA  55  08    
                                        	{
                                        	        //  send msg_queue
                                        	        _485_MsgQue_sruct.info=_485_dev_rx;
							 _485_MsgQue_sruct.len=_485dev_wr;	
							 // rt_mq_send( &_485_MsgQue, (void*)&_485_MsgQue_sruct, _485_MsgQue_sruct.len+ 2 ); 
							 //-------------------------- 
                                                  memset(DwinLCD.RxInfo,0,sizeof((const char*)DwinLCD.RxInfo));
							 memcpy(DwinLCD.RxInfo,_485_dev_rx,_485_RXstatus._485_RxLen); 
							 DwinLCD.RxInfolen=_485_RXstatus._485_RxLen;
							 DwinLCD.Process_Enable=1;      
							 //-------------------------- 
							_485dev_wr=0;  // clear	  
							_485_RXstatus._485_receiveflag=IDLE_485;   
                                        	}					
					   break;  
	   case   CAMERA_Related:
	   	                             if(_485dev_wr>=_485_RXstatus._485_RxLen) 
	   	                             {
                                                  memset(_485_content,0,sizeof(_485_content));
							 //-------------------------------------------------- 					  
							 memcpy(_485_content,_485_dev_rx,_485_RXstatus._485_RxLen); 
							 _485_content_wr=_485_RXstatus._485_RxLen; // Packet info len 
                                                   _485_CameraData_Enable=1;   // 图片数据过来了
                                                  //---------------------------------------------------
							 _485dev_wr=0; // clear 
							 _485_RXstatus._485_receiveflag=IDLE_485;
	   	                             } 
	                              break; 
	   default:
	   	                       _485dev_wr=0;  
					  _485_RXstatus._485_receiveflag=	IDLE_485; 
					  break ;
     } 

}


void  Pic_Data_Process(void) 
{
      u8  tmp[40]; 
      u8   pic_buf[600];
	    u16 i=0;
  

           // 1.    Judge   last  package  
            PackageLen=_485_content_wr;
             if(PackageLen<PageSIZE)
			  last_package=1;    	
	    else   
	      if((last_package==0)&&(_485_content[510]==0xFF)&&(_485_content[511]==0xD9))
		  {
		            last_package=1;        
		  }          
	   //  2.    Block ++	    
	      CameraState.block_counter++;
	  //   3.    Check   first   packet
            if(CameraState.create_Flag==1)    // 如果是第一包则创建文件 hhmmss_x.jpg
              {                  
                     CameraState.create_Flag=0; // clear 
					 memset(tmp,0,sizeof(tmp));
					 memset(PictureName,0,sizeof(PictureName));
					 //-----------  创建图片文件处理  -------------					 
					 /*
						 每张图片占32个page    其中第1个page 为图片索引，后边127个Page为图片内容	
						 SST25 开辟个区域做图片缓存 
					 */
                                   DF_Read_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);					 
					 //pic_current_page=(pic_write<<5)+PicStart_offset; //计算图片起始page 
					if(Camera_Number==1) 
					{
					     pic_current_page=PicStart_offset; //起始page 固定为缓存起始地址
					     //擦除一个64K的区域用于图片存储  
					  
					     Api_DFdirectory_Delete(camera_1);
					  
					}
					else
					if(Camera_Number==2) 
					{
						 pic_current_page=PicStart_offset2; //起始page 固定为缓存起始地址
						 //擦除一个64K的区域用于图片存储  
					   
					     Api_DFdirectory_Delete(camera_2);
					   ; 	 
					}	
					else
					if(Camera_Number==3) 
					{
						 pic_current_page=PicStart_offset3; //起始page 固定为缓存起始地址
						 //擦除一个64K的区域用于图片存储  
					     Api_DFdirectory_Delete(camera_3);   	 
					}
					else
					if(Camera_Number==4) 
					{
					     pic_current_page=PicStart_offset4; //起始page 固定为缓存起始地址
						 //擦除一个64K的区域用于图片存储  
					     Api_DFdirectory_Delete(camera_3);	   
					}	
					 DF_delay_ms(200);  
					 WatchDog_Feed(); 
					 pic_current_page++;  // 图片内容从 第二个page 开始 第一个Page 存储的是图片索引 
					 pic_PageIn_offset=0; // 页内偏移清空 
					 pic_size=0; // 清除图片大小
					 //------------------------------------------					 
					 memset(PictureName,0,sizeof(PictureName));    
					 sprintf((char*)PictureName,"%d%d%d-%d.jpg",time_now.hour,time_now.min,time_now.sec,Camera_Number);  	
                  if(DispContent==2) 
					 rt_kprintf("\r\n              创建图片名称: %s \r\n                  图片地址: %d ",PictureName,pic_write);   
					 WatchDog_Feed();  
        
		              //----- TF -------		         
				/*if(Udisk_Test_workState==1)	 	
		              {  
                               // creat_file(PictureName); //TF卡创建文件 
                                 rt_kprintf("\r\n   udisk  创建文件成功!");
                                 udisk_fd=open((const char*)PictureName, O_RDWR|O_CREAT, 0);  // 创建U 盘文件
                                 rt_kprintf(" \r\n udiskfile: %s  open res=%d   \r\n",PictureName, udisk_fd);  
		              }  */		              
				 WatchDog_Feed();         
                      // -----    写图片索引 -------
                      Save_MediaIndex(0,PictureName,Camera_Number,0);  					  
               	}  
			
	       //  4.   填写存储图片内容数据  --------------------		
		   DF_WriteFlashDirect(pic_current_page,0,_485_content, PackageLen);// 写一次一个Page 512Bytes
		     if(DispContent==2) 
		   rt_kprintf(" \r\n ---- write  pic_current_page=%d  \r\n",pic_current_page);    
                         //---  read compare 
		    memset(pic_buf,0,600);
		    DF_ReadFlash(pic_current_page,0,pic_buf, PackageLen);
		    for(i=0;i<PackageLen;i++)
		   {		if(pic_buf[i]!=_485_content[i])
		    	       {
					break;		 
		    	        }
		    }	 
				   pic_size+=PackageLen;// 图片大小累加	 				   
				   pic_current_page++; //写一页加一
				  // pic_PageIn_offset+=PackageLen;  
				   DF_delay_ms(80);   
	   //   5.   最后一包 ，即拍照结束
		  if(last_package==1)
		 {
			   //f_close(&FileCameraIn);  //  拍照完成关闭图片文件
			   memset(_485_content,0,sizeof(_485_content)); 
			   _485_content_wr=0;			  

			   //-------------  图片拍照结束 相关处理  ------------------------------------
			   //  1. 写图片索引
			   if(Camera_Number==1)
				   pic_current_page=PicStart_offset; //计算图片起始page 
			   else
			   if(Camera_Number==2)
			         pic_current_page=PicStart_offset2; //计算图片起始page 
			   else
			   if(Camera_Number==3)
			         pic_current_page=PicStart_offset3; //计算图片起始page 
			   else
			   if(Camera_Number==4)
			         pic_current_page=PicStart_offset4; //计算图片起始page        
			   PictureName[18]=Camera_Number;
			   memcpy(PictureName+19,(u8*)&pic_size,4);	 			   
			   DF_WriteFlashDirect(pic_current_page,0,PictureName, 23);  
			   DF_delay_ms(10); 
			   
	               //  5.1   更新图片读写记录
	               pic_write++;
				   if(pic_write>=Max_PicNum)
				   	  pic_write=0;    
	               DF_Write_RecordAdd(pic_write,pic_read,TYPE_PhotoAdd);  
	               //--------------------------------------------------------------------------      
	             if(DispContent==2) 
	               rt_kprintf("\r\n        PicSize: %d Bytes\r\n    Camera  %d   End\r\n",pic_size,Camera_Number); 
			 SingleCamra_TakeResualt_BD=0;	    //  单路摄像头拍照
			 SD_ACKflag.f_BD_CentreTakeAck_0805H=1;  //  发送中心拍照命令应答
		        //----------  Normal process ---------------------
			 End_Camera(); 

				
		        // 5.2   拍照完成后检查有没有多路 拍-----------Multi Take process--------------------
				   if(1==MultiTake.Taking)
				   {
					   switch(Camera_Number)
						 {
							case  1: 
	                                   //-------- old process---------------
	                                   MultiTake.TakeResult[0]=Take_Success;   // 表示第一路摄像头拍照失败
									   //--------- new   process-------------
	                                   MultiTake.Take_retry=0;
	                                   //-------------------------------------------
									   if(CHKendTake_ReadyToSend())// 检查是否是最后的拍照线路
									   	    break;
									   //----------拍照下一路摄像头-----------									   
										   Camera_Number=2;  
									   //-------------------------
									    Start_Camera(Camera_Number);
									 
								    break;
							case  2: 
	                                   //-------- old process---------------
	                                   MultiTake.TakeResult[1]=Take_Success;   // 表示第一路摄像头拍照失败
									   //--------- new   process-------------
	                                   MultiTake.Take_retry=0;
	                                   //-------------------------------------------
									   if(CHKendTake_ReadyToSend())// 检查是否是最后的拍照线路
									   	    break;
									   //----------拍照下一路摄像头-----------									   
										   Camera_Number=3;  
									   //-------------------------
									    Start_Camera(Camera_Number);
									 
								    break;
						   case  3: 
	                                   //-------- old process---------------
	                                   MultiTake.TakeResult[2]=Take_Success;   // 表示第一路摄像头拍照失败
									   //--------- new   process-------------
	                                   MultiTake.Take_retry=0;
	                                   //-------------------------------------------
									   if(CHKendTake_ReadyToSend())// 检查是否是最后的拍照线路
									   	    break;
									   //----------拍照下一路摄像头-----------									   
										   Camera_Number=4;   
									   //-------------------------
									    Start_Camera(Camera_Number);
									 
								    break;				
							case  4: 
	                                   //-------- old process---------------
	                                   MultiTake.TakeResult[3]=Take_Success;   // 表示第一路摄像头拍照失败
									   //--------- new   process-------------
	                                   MultiTake.Take_retry=0;
									   
	                                   //-------------------------------------------
									   if(CHKendTake_ReadyToSend())// 检查是否是最后的拍照线路
									   	    break;
									
								    break;					
						    default:
								    MultiTake_End();
	                                                     break;
								  
					   	 }		

					}		
				   else
				   if((0==MultiTake.Taking)&&(0==MultiTake.Transfering)) 
				    {    
				       //------ 判断车门开关拍照是否上传状态 ------
					    if((OpenDoor_StartTakeFlag==1)&&(Opendoor_transFLAG==0)) 
			                    {
			                         MultiTake_End();   // 车门开关不上传
			                         OpenDoor_StartTakeFlag=0;
							    }     
			                   else
			                   	{
						      if(Camera_Take_not_trans==0)	
						              Photo_send_start(Camera_Number);  //在不是多路拍照的情况下拍完就可以上传了
                                                else
 								Camera_Take_not_trans=0;   					 
 						}
				   	}	
				    //  拍照结束  
	    }
	 else
	  {
	     
		 //------- change state  -------
		 CameraState.status=transfer;
		 CameraState.OperateFlag=0;   // clear 
	  
		  TX_485const_Enable=1;  // 发送485 命令
		   _485_RXstatus._485_receiveflag=IDLE_485;   
                 //rt_kprintf("\r\n  Head info_len : %d\r\n",_485_content_wr); 
		  memset(_485_content,0,sizeof(_485_content));
		  _485_content_wr=0;			  
		  //rt_kprintf("\r\n  One Packet Over!\r\n"); 	 
		 
	  } 			  
}


static rt_err_t   Device_485_init( rt_device_t dev )
{
      GPIO_InitTypeDef  GPIO_InitStructure;
      USART_InitTypeDef USART_InitStructure;    
     NVIC_InitTypeDef NVIC_InitStructure;	  


       //  1 . Clock
 		  
	/* Enable USART2 and GPIOA clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	/* Enable USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

      //   2.  GPIO    
       GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;   
	GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* Connect alternate function */
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); 

     //  3.  Interrupt
        	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
    //   4.  uart  Initial
       USART_InitStructure.USART_BaudRate = 57600;  //485
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; 
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure); 

	/* Enable USART */
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);      

	    /* -------------- 485  操作相关 -----------------	*/
	    /*
			   STM32 Pin	 		   Remark
				 PC4		  		    485_Rx_Tx 控制   0:Rx    1: Tx
				 PD0		  		    485 电源	1: ON  0:OFF
		*/
	  
	   /*  管脚初始化 设置为 推挽输出 */

	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	 // ------------- PD10     --------------------------------
	GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_8;		//--------- 485 外设置的电
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
	
       Power_485CH1_ON;  // 第一路485的电	       上电工作   
	 
 //------------------- PC4------------------------------	
	 GPIO_InitStructure.GPIO_Pin  =GPIO_Pin_4;				 //--------- 485const	收发控制线 
	 GPIO_Init(GPIOC, &GPIO_InitStructure); 
	 RX_485const;  
	 return RT_EOK; 
}

static rt_err_t Device_485_open( rt_device_t dev, rt_uint16_t oflag )  
{
         return RT_EOK;
}
static rt_err_t Device_485_close( rt_device_t dev )
{
        return RT_EOK;
}
static rt_size_t Device_485_read( rt_device_t dev, rt_off_t pos, void* buff, rt_size_t count )
{

        return RT_EOK;
}
static rt_size_t Device_485_write( rt_device_t dev, rt_off_t pos, const void* buff, rt_size_t count )
 {
        unsigned int  Info_len485=0;
	 const char		*p	= (const char*)buff;
	

	Info_len485=(unsigned int)count;
    	/* empty console output */
	TX_485const; 
	_485_delay_ms(1);
		//--------  add by  nathanlnw ---------
  while (Info_len485)
	{
		rt_hw_485_putc (*p++);   
		Info_len485--;
	}
       //--------  add by  nathanlnw  --------	
       _485_delay_ms(3); 
       RX_485const; 	 
        return RT_EOK;
  }
static rt_err_t Device_485_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
       return RT_EOK;
 }

static void timeout_485(void *  parameter)
{     //  100ms  =Dur 
       One_second_Counter_485++;
       if(One_second_Counter_485>10)  
       {
            One_second_Counter_485=0;       
	     Camra_Take_Exception();	 
       }	   
         DwinLCD_Timer();
	  DwinLCD_DispTrigger();	 
}

/* 485 thread */
ALIGN(RT_ALIGN_SIZE)
static char _485_thread_stack[2048]; 
struct rt_thread _485_thread;
struct rt_semaphore _485Rx_sem;

void _485_thread_entry(void* parameter)  
{
       rt_err_t  res=RT_EOK;
	MSG_Q_TYPE  rec_485;
	u8    rec_info[20];
		
         //  1.  Debug out   &  meset  	
	rt_kprintf("\r\n ---> 485 thread start !\r\n"); 

	//Voice_Dev_Init();
	Init_Camera();  


    // 2.  Thread main body below
	while (1)
	{ 
	
      //  Dwin  RxProcess
       DwinLCD_Data_Process();
		
	   //--------------------- 拍照数据处理-----	
	   if(_485_CameraData_Enable)	   
	   {
                  Pic_Data_Process();
                _485_CameraData_Enable=0;
	   }		
	    rt_thread_delay(35); 	  		
	    //-------     485  TX ------------------------
           Send_const485(TX_485const_Enable);   		   

	}
}



/* init 485 */
void _485_startup(void)
{
        rt_err_t result;

     //  Msg Que  Init 
       rt_mq_init( &_485_MsgQue, "mq_485", &_485_MsgQueStack[0], 64, _485_MsgQueStack_SIZE, RT_IPC_FLAG_FIFO );
     //  Create  Timer
     timer_485=rt_timer_create("tim_485",timeout_485,RT_NULL,10,RT_TIMER_FLAG_PERIODIC); 
     if(timer_485!=RT_NULL)
          rt_timer_start(timer_485);      



       // -------  thread-------
	result=rt_thread_init(&_485_thread, 
		"485dev",
		_485_thread_entry, RT_NULL,
		&_485_thread_stack[0], sizeof(_485_thread_stack),  
		Prio_485, 10);  

    if (result == RT_EOK)
    {
       rt_thread_startup(&_485_thread); 
    	}


	Device_485.type	= RT_Device_Class_Char;
	Device_485.init	= Device_485_init;
	Device_485.open	=  Device_485_open;
	Device_485.close	=  Device_485_close;
	Device_485.read	=  Device_485_read;
	Device_485.write	=  Device_485_write;
	Device_485.control =Device_485_control;

	rt_device_register( &Device_485, "485", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
	rt_device_init( &Device_485 );


        //   DwinLCD   Init
        DwinLCD_init(); 
}


