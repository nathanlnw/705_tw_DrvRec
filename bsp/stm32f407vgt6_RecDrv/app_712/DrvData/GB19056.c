/*
     GB19056.C
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
#include "App_gsm.h"
#include "GB19056.h"


//    GB  ���ݵ���
#define GB_USB_OUT_idle          0
#define GB_USB_OUT_start         1
#define GB_USB_OUT_running       2
#define GB_USB_OUT_end           3


struct rt_semaphore GB_RX_sem;  //  gb  ����AA 75

rt_device_t   Udisk_device= RT_NULL;

GB_STRKT GB19056;
u8	Demostr[25]={0x55,0x7A,0x01,0x00,0x12,0x00,0x34,0x31,0x30,0x37,0x32,0x37,0x31,0x39,0x38,0x35,0x30,0x33,0x32,0x39,0x34,0x34,0x33,0x36,0x33};
int  usb_fd=0;



void GB_Drv_init(void)
{
  GB19056.workstate=0;
  GB19056.RX_CMD=0;
  GB19056.TX_CMD=0;
  GB19056.RX_FCS=0;
  GB19056.TX_FCS=0;
  GB19056.rx_wr=0;
  GB19056.rx_flag=0;
  GB19056.usb_write_step=GB_USB_OUT_idle; 

}

void  GB19056_Decode(u8 *instr,u8  len)
{
    // AA 75   already   check     AA 75 08 00 01 00 BB
    //  cmd  ID  
    switch(instr[2])
    	{ 
    	    case  0x00:   //�ɼ���¼��ִ�а汾
				
				          break;
			case  0x01:   //  �ɼ���ʻ����Ϣ
				
				          break;
		    case  0x02:   // �ɼ���¼��ʱ��
				
				          break;
			case  0x03:   //  �ɼ��ۼ���ʻ���
				
				          break;
			case  0x04:   // �ɼ���¼������ϵ�� 
				
				          break;
			case  0x05:   // �ɼ�������Ϣ 
				
				          break;
			case  0x06:    // �ɼ���¼���ź�������Ϣ
				
				          break;
			case  0x07:    //  �ɼ���¼��Ψһ�Ա��
				
				          break;
		    case  0x08:     //  �ɼ���¼��ָ������ʻ�ٶȼ�¼
				
				          break;
			case  0x09:    //  �ɼ�ָ����ָ��λ����Ϣ��¼
				
				          break;
			case  0x10:   // �ɼ��¹��ɵ��¼
				
				          break;
			case  0x11:    //  �ɼ�ָ���ĳ�ʱ��ʻ��¼
				
				          break;
			case  0x12:    //  �ɼ�ָ���ļ�ʻ����ݼ�¼ 
				
				          break;
			case  0x13:    // �ɼ�ָ�����ⲿ�����¼
				
				          break;
		    case  0x14:    //  �ɼ�ָ���Ĳ����޸ļ�¼

				          break;
			case  0x15:    // �ɼ�ָ�����ٶ�״̬��־

				          break;
			case  0xFE:    //��ԭ��ʾ״̬
                          GB19056.workstate=0;
	                      rt_kprintf(" GB_Data_disable\r\n");      
						  break;
            default:
	                      break;




    	}



}

void  gb_usb_out(u8 ID)
{  /*
              ������Ϣ��USB device
     */
   if( USB_Disk_RunStatus()==USB_FIND)	
   	{
   	  if(GB19056.usb_exacute_output==2)
      {
         
		 if(GB19056.workstate==0)
			 rt_kprintf("\r\n GB USB ���ݵ����� \r\n");
         return;
   	  }
	  else
	  	{
	  	  
		    switch(ID)
		  	{
		  	   case 0x00:
			   case 0x01:
			   case 0x02:
			   case 0x03:
			   case 0x04:
			   case 0x05:
			   case 0x06:
			   case 0x07:
			   case 0x08:
			   case 0x09:
			   case 0x10:
			   case 0x11:
			   case 0x12:
			   case 0x13:
			   case 0x14:
			   case 0x15:
			   	
			   case 0xFE:  // output all
			   case 0xFB:  // recommond output
			   case 0xFF:  //idle
			              break;
			   default:
                        if(GB19056.workstate==0)
			                rt_kprintf("\r\n ����ID ����!  idle:  FF    0x00~0x15     0xFE :out put all        0xFB : output  recommond info   \r\n");
				        return;


		  	}
             GB19056.usb_write_step=GB_USB_OUT_start;
			 GB19056.usb_exacute_output=1;
			 GB19056.usb_out_selectID=ID;

			 
			 if(GB19056.workstate==0)
			    rt_kprintf("\r\n USB ���ݵ���\r\n");
	  	}
   	}
   else
   	{
   	 if(GB19056.workstate==0)
   	   rt_kprintf("\r\n û�м�⵽ USB device \r\n");
   	}   

}
FINSH_FUNCTION_EXPORT(gb_usb_out, gb_usb_out(ID) idle:  FF    0x00~0x15     0xFE :out put all        0xFB : output  recommond info   );  


/*��¼�����ݽ���״̬*/
ALIGN(RT_ALIGN_SIZE)

char gbdrv_thread_stack[1024];      
struct rt_thread gbdrv_thread; 

void thread_GBData_mode( void* parameter )
{

/*����һ������ָ�룬�����������	*/
    
	u32   i=0;	    
	rt_err_t res;

//	dev_vuart.flag &= ~RT_DEVICE_FLAG_STREAM;
//	rt_device_control( &dev_vuart, 0x03, &baud );
	GB_Drv_init();


 
	while( 1 )
	{
	  // part1:  serial  RX
		  	if (rt_sem_take(&GB_RX_sem, 20) == RT_EOK)  
			{
			    // 0  debug out
                OutPrint_HEX("��¼����",GB19056.rx_buf,GB19056.rx_wr); 

                // 1  process  rx
				GB19056_Decode(GB19056.rx_buf,GB19056.rx_infoLen+7);
				
			
               GB19056.rx_wr=0;
			   GB19056.rx_flag=0;
			   GB19056.rx_infoLen=0;
			}
	   // part1:  serial TX




       // part2:  USB outputFile 
		        if(GB19056.usb_write_step==GB_USB_OUT_start)	
		  		{
  					 //------ U disk
				   Udisk_dev=rt_device_find("udisk");
				  if (Udisk_dev != RT_NULL)	   
				   {	   rt_kprintf("\r\n  Udiskopen");
								  
						 res=rt_device_open(Udisk_dev, RT_DEVICE_OFLAG_RDWR);  
						if(res==RT_EOK)
						{
						     //  �����ļ�
				              memset(GB19056.Usbfilename,0,sizeof(GB19056.Usbfilename));
							 sprintf((char*)GB19056.Usbfilename,"/udisk/%d%d%d%d.DRV",time_now.day,time_now.hour,time_now.min,time_now.sec);	  
							 //strcpy((char*)GB19056.Usbfilename,"/udisk/Drv.TXT"); 	  
							 usb_fd=open((const char*)GB19056.Usbfilename, (O_CREAT|O_WRONLY|O_TRUNC), 0 );	  // ����U ���ļ�
                              
							  if(GB19056.workstate==0) 
								rt_kprintf(" \r\n udiskfile: %s  create res=%d	 \r\n",GB19056.Usbfilename, usb_fd);  
							 if(usb_fd>=0)
							 {	                                      

								  if(GB19056.workstate==0) 
									rt_kprintf("\r\n			  ����Drv����: %s \r\n ",GB19056.Usbfilename);   
								  WatchDog_Feed();  
								   
								  GB19056.usb_write_step=GB_USB_OUT_running;// goto  next step 										  
							  }
							 else
							 	{                                           
								   if(GB19056.workstate==0) 
									 rt_kprintf(" \r\n udiskfile create Fail   \r\n",GB19056.Usbfilename, usb_fd);  
								   GB19056.usb_write_step=GB_USB_OUT_idle;
							 	} 
					     }
					 }
				}
			if(GB19056.usb_write_step==GB_USB_OUT_running)
			{
			    for(i=0;i<400;i++)
			    	{
							     write(usb_fd, Demostr,25); 							  
							   if(GB19056.workstate==0) 
							   rt_kprintf(" \r\n usboutput   write=%d \r\n",i);  
			    	}
                    GB19056.usb_write_step=GB_USB_OUT_end;  
				}
			if(GB19056.usb_write_step==GB_USB_OUT_end)
			{

                             close(usb_fd);      
                             GB19056.usb_write_step=GB_USB_OUT_idle;
							 
							 if(GB19056.workstate==0) 
							   rt_kprintf(" \r\n usboutput   over! \r\n");  
			}

	   //-------------------------------------------
		rt_thread_delay(10);   // RT_TICK_PER_SECOND / 50 
	}

}

void GB_Drv_app_init(void)
{
        rt_err_t result;

      
	  rt_sem_init(&GB_RX_sem, "gbRxSem", 0, 0);		  
		
      //------------------------------------------------------
	result=rt_thread_init(&gbdrv_thread, 
		"GB_DRV", 
		thread_GBData_mode, RT_NULL,
		&gbdrv_thread_stack[0], sizeof(gbdrv_thread_stack),  
		Prio_GBDRV, 10); 
	   

    if (result == RT_EOK)
    {
           rt_thread_startup(&gbdrv_thread); 
    	}
}






void  gb_work(u8 value)
{
  if(value)
  	{
  	 GB19056.workstate=1;
     rt_kprintf(" GB_Data_enable\r\n");
  	}
  else 
  	{
  	 GB19056.workstate=0;
	 rt_kprintf(" GB_Data_disable\r\n");
  	}

}
FINSH_FUNCTION_EXPORT(gb_work, gb_work(1 :0));


