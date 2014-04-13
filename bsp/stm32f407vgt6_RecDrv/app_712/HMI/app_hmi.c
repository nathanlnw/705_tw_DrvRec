/*
     HMI  :  Process  LCD  ��Printer �� KeyChecking
*/

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>
#include "Menu_Include.h"

#define                                      HMI_MsgQueStack_SIZE  512

 struct rt_mailbox mb_hmi;
 char   mb_hmi_pool[128];
 char   mb_str1[]={"******************"};
 char   mb_str2[]={"abcde"};
 char   hmi_counter=1;
 

unsigned char dayin_15MinSpeed[32]={"00:00 000 km/h\r\n"};
unsigned char dayin_data_time[32]={"��ӡʱ��:                    \r\n"};
unsigned char dayin_chepaihaoma[25]={"���ƺ���:00000000\r\n"};
unsigned char dayin_chepaifenlei[25]={"���Ʒ���:000000\r\n"};
unsigned char dayin_cheliangVIN[32]={"����VIN:00000000000000000\r\n"};
//unsigned char dayin_driver_NUM[40]={"��ʻԱ����:000000000000000000000\r\n"};
unsigned char dayin_driver_NUM[40]={"��ʻԱ����:000000\r\n"};
unsigned char dayin_driver_card[40]={"��ʻ֤����:000000000000000000\r\n"};
unsigned char Dayin_TireExpsCard[32]={"1.000000000000000000\r\n"};
unsigned char Dayin_TireExpsStartTime[32]={"S_T:XX/XX/XX XX:XX:XX\r\n"};//45 78 1011 1314 1617 1920
unsigned char Dayin_TireExpsEndTime[32]=  {"E_T:XX/XX/XX XX:XX:XX\r\n"};
unsigned char Dayin_TireExpsMaxSpeed[32]=  {"MaxSpeed:XXX km/h\r\n"};//9 10 11    


//ALIGN(RT_ALIGN_SIZE)
//static uint8_t					HMI_MsgQueStack[HMI_MsgQueStack_SIZE];
static struct rt_messagequeue	HMI_MsgQue; 


HMI_COM    HMI_Comunicate; 



void  HMI_MsgQue_Post(u16  CMD_ID, u16 len, u8 *content)
{
        HMI_Comunicate.Msg_ID=CMD_ID;
        HMI_Comunicate.Info_Len=len;
	 HMI_Comunicate.Content=content;
		
       rt_mq_send( &HMI_MsgQue, (void*)&HMI_Comunicate,sizeof(HMI_COM) );
}

void Dayin_TireExpsFun(unsigned char type,unsigned char n)
{
unsigned char i=0;
if(type==1)
	{
	if(n==1)
		{
		memcpy(Dayin_TireExpsCard+2,PilaoJilu[0].PCard,18);
		printer((const char *)Dayin_TireExpsCard);
		}
	else if(n==2)
		{
		for(i=0;i<6;i++)
			{
			Dayin_TireExpsStartTime[4+i*3]=PilaoJilu[0].StartTime[i]/10+0x30;
			Dayin_TireExpsStartTime[4+i*3+1]=PilaoJilu[0].StartTime[i]%10+0x30;
			}
		printer((const char *)Dayin_TireExpsStartTime);
		}
	else
		{
		for(i=0;i<6;i++)
			{
			Dayin_TireExpsEndTime[4+i*3]=PilaoJilu[0].EndTime[i]/10+0x30;
			Dayin_TireExpsEndTime[4+i*3+1]=PilaoJilu[0].EndTime[i]%10+0x30;
			}
		printer((const char *)Dayin_TireExpsEndTime);
		}
	}
else
	{
	if(n==1)
		{
		memcpy(Dayin_TireExpsCard+2,ChaosuJilu[0].PCard,18);
		printer((const char *)Dayin_TireExpsCard);
		}
	else if(n==2)
		{
		for(i=0;i<6;i++)
			{
			Dayin_TireExpsStartTime[4+i*3]=ChaosuJilu[0].StartTime[i]/10+0x30;
			Dayin_TireExpsStartTime[4+i*3+1]=ChaosuJilu[0].StartTime[i]%10+0x30;
			}
		printer((const char *)Dayin_TireExpsStartTime);
		}
	else if(n==3)
		{
		for(i=0;i<6;i++)
			{
			Dayin_TireExpsEndTime[4+i*3]=ChaosuJilu[0].EndTime[i]/10+0x30;
			Dayin_TireExpsEndTime[4+i*3+1]=ChaosuJilu[0].EndTime[i]%10+0x30;
			}
		printer((const char *)Dayin_TireExpsEndTime);
		}
	else
		{
		Dayin_TireExpsMaxSpeed[9]=ChaosuJilu[0].Speed/100+0x30;
		Dayin_TireExpsMaxSpeed[10]=ChaosuJilu[0].Speed%100/10+0x30;
		Dayin_TireExpsMaxSpeed[11]=ChaosuJilu[0].Speed%10+0x30;
		printer((const char *)Dayin_TireExpsMaxSpeed);
		}
	}
}
	
void Dayin_15MinSpeedFun(unsigned char n)
{   
    dayin_15MinSpeed[0]=speed_time_rec[n][3]/10+0x30;
	dayin_15MinSpeed[1]=speed_time_rec[n][3]%10+0x30;
	
	dayin_15MinSpeed[3]=speed_time_rec[n][4]/10+0x30;
	dayin_15MinSpeed[4]=speed_time_rec[n][4]%10+0x30;
	
	dayin_15MinSpeed[6]=speed_time_rec[n][5]/100+0x30;
	dayin_15MinSpeed[7]=speed_time_rec[n][5]%100/10+0x30;
	dayin_15MinSpeed[8]=speed_time_rec[n][5]%10+0x30;
	printer((const char *)dayin_15MinSpeed);
}
void Dayin_Fun(void)
{
	memcpy(dayin_chepaihaoma+9,JT808Conf_struct.Vechicle_Info.Vech_Num,8);    //  2
	memcpy(dayin_chepaifenlei+9,JT808Conf_struct.Vechicle_Info.Vech_Type,6);  //  3
	memcpy(dayin_cheliangVIN+8,JT808Conf_struct.Vechicle_Info.Vech_VIN,17);   //  4
	//memcpy(dayin_driver_NUM+11,JT808Conf_struct.Driver_Info.DriveName,21);    //5
	memcpy(dayin_driver_card+11,JT808Conf_struct.Driver_Info.DriverCard_ID,18);//6
	memcpy((char *)dayin_data_time+9,(char *)Dis_date,20);  //7
	switch(DaYin)
	{
	case 1:
		DaYin++;
		break;
	case 2://���ƺ��� 9
		printer((const char *)dayin_chepaihaoma);
		DaYin++;
		break;
	case 3://���Ʒ���
		printer((const char *)dayin_chepaifenlei);
		DaYin++;
		break;
	case 4://����VIN���� 8
		printer((const char *)dayin_cheliangVIN);
		DaYin++;
		break;
	case 5://��ʻԱ���� 11   ��ʻԱ����
		printer((const char *)dayin_driver_NUM);
		DaYin++;
		break;
	case 6://��ʻ֤����
		printer((const char *)dayin_driver_card);
		DaYin++;
		break;
	case 7:
		printer((const char *)dayin_data_time);//00/00/00 00:00:00
		DaYin++;
		break;
	case 8:
		printer("ͣ��ǰ15���ӳ���:\r\n"); 
		DaYin++;
		break;
	case 9:
		Dayin_15MinSpeedFun(0);
		DaYin++;
		break;
	case 10:
		Dayin_15MinSpeedFun(1);
		DaYin++;
		break;
	case 11:
		Dayin_15MinSpeedFun(2);
		DaYin++;
		break;
	case 12:
		Dayin_15MinSpeedFun(3);
		DaYin++;
		break;
	case 13:
		Dayin_15MinSpeedFun(4);
		DaYin++;
		break;
	case 14:
		Dayin_15MinSpeedFun(5);
		DaYin++;
		break;
	case 15:
		Dayin_15MinSpeedFun(6);
		DaYin++;
		break;
	case 16:
		Dayin_15MinSpeedFun(7);
		DaYin++;
		break;
	case 17:
		Dayin_15MinSpeedFun(8);
		DaYin++;
		break;
	case 18:
		Dayin_15MinSpeedFun(9);
		DaYin++;
		break;
	case 19:
	    Dayin_15MinSpeedFun(10);
		DaYin++;
		break;
	case 20:
		Dayin_15MinSpeedFun(11);
		DaYin++;
		break;
	case 21:
		Dayin_15MinSpeedFun(12);
		DaYin++;
		break;
	case 22:
		Dayin_15MinSpeedFun(13);
		DaYin++;
		break;
	case 23:
		Dayin_15MinSpeedFun(14);
		DaYin++;
		break;
	case 24:
		printer("���һ��ƣ�ͼ�ʻ��¼:\r\n");
		DaYin++;
		break;
	case 25:
		if(TiredDrv_write>0)
			{
			Dayin_TireExpsFun(1,1);
			DaYin++;
			}
		else
			{
			printer("��ƣ�ͼ�ʻ��¼\r\n");
			DaYin=28;
			}
		break;
	case 26:
		Dayin_TireExpsFun(1,2);
		DaYin++;
		break;
	case 27:
		Dayin_TireExpsFun(1,3);
		DaYin++;
		break;
	case 28:
		printer("���һ�γ��ټ�ʻ��¼:\r\n");
		DaYin++;
		break;
	case 29:
		if(ExpSpdRec_write>0)
			{
			Dayin_TireExpsFun(2,1);
			DaYin++;
			}
		else
			{
			printer("�޳��ټ�ʻ��¼\r\n");
			DaYin=33;
			}
		break;
	case 30:
		Dayin_TireExpsFun(2,2);
		DaYin++;
		break;
	case 31:
		Dayin_TireExpsFun(2,3);
		DaYin++;
		break;
	case 32:
		Dayin_TireExpsFun(2,4);
		DaYin++;
		break;	
    case 33:
		step(50,1000);
		DaYin++;
		break;
	default:
		DaYin=0;
		print_rec_flag=0;
		GPIO_ResetBits(GPIOB,GPIO_Pin_7);//��ӡ�ص�
		
		break;
	}
	
}


/* HMI  thread */
ALIGN(RT_ALIGN_SIZE) 
char HMI_thread_stack[4096]; 
struct rt_thread HMI_thread;

static void HMI_thread_entry(void* parameter)  
{
    u8 counter_printer=0,can_counter=0;
    rt_size_t  res=RT_ERROR;   
	

     //  finsh_init(&shell->parser);

      //------------ lcd  related --------------
       Init_lcdkey();
       lcd_init();
    if(JT808Conf_struct.password_flag==0)
    	{
    	    JT808Conf_struct.Regsiter_Status=0;   //��Ҫ����ע��
            pMenuItem=&Menu_0_0_password;
	    pMenuItem->show();
    	}
	else
		{
	    pMenuItem=&Menu_1_Idle;
		pMenuItem->show();
		}
	while (1)
	{


	      if(CAN_trans.can1_trans_dur>0) 
              {
                   can_counter++;
		     if(can_counter>=5)
		     	{
                          CAN_trans.can1_enable_get=1;   
                          can_counter=0; 
		     	}		   
              } 
		//----------------------------------		 
		#ifdef BITTER
		Cent_To_Disp();
		#endif
	       KeyCheckFun();
              pMenuItem->timetick( 10 ); 
	 	pMenuItem->keypress( 10 );  
		if(print_rec_flag==1)
			{
			counter_printer++;
			if(counter_printer>=20)//�ӵ��1s��ʼ��ӡ����ӡ�������>300ms
				{
				counter_printer=0;
				if(TiredDrv_write>0)
					{
					WatchDog_Feed();
					ReadPiLao(1);
					Dis_pilao(data_tirexps);
					}
				if(ExpSpdRec_write>0)
					{
					WatchDog_Feed();
					ReadEXspeed(1);
					Dis_chaosu(data_tirexps);
					}
				if(0==Fetch_15minSpeed(15))
					{
					print_rec_flag=2;
					DaYin=1;//��ʼ��ӡ
					}
				else
				      {
				        DaYin=0;
					 print_rec_flag=0;
					}
				WatchDog_Feed();
				}
			}
		else if(print_rec_flag==2)
			{
			counter_printer++;
			if(counter_printer>=7)//��ӡ�������>300ms
				{
				counter_printer=0;
				Dayin_Fun();	
				}
			}
               //--------------------------------------------
               if(ASK_Centre.ASK_disp_Enable==1)
		   {
			ASK_Centre.ASK_disp_Enable=0; 
			pMenuItem=&Menu_3_1_CenterQuesSend;
		       pMenuItem->show();
		 }
		else if(TextInfo.TEXT_SD_FLAG==1)
		{
			TextInfo.TEXT_SD_FLAG=0;
			pMenuItem=&Menu_7_CentreTextDisplay;
		       pMenuItem->show();
		}
	 	//--------------------------------------------	   
              rt_thread_delay(10);     

     }  
}



/* init HMI  */
void HMI_app_init(void)
{
        rt_err_t result;
		

	result=rt_thread_init(&HMI_thread, "HMI", 
		HMI_thread_entry, RT_NULL,
		&HMI_thread_stack[0], sizeof(HMI_thread_stack),    
		Prio_HMI, 10); 

    if (result == RT_EOK)
    {
           rt_thread_startup(&HMI_thread); 
    	}
}


