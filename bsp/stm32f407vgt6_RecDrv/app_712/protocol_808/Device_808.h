/*
     Device  808 .h
*/
#ifndef  _DEVICE808
#define   _DEVICE808

#include <rtthread.h> 
#include <rthw.h>
#include "stm32f4xx.h"

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"


#define  PageSIZE          512

//---------ISP    应该放在Dataflash 设置文件中的--------------
#define DF_BL_PageNo		                 10             /*DF_BL_RAM run PageNo:   10  ~ 49  page */
#define DF_APP1_PageNo		                 50             /*DF_APP_flah run PageNo:   50  ~ 903  page*/


//--------  Protocol IO define -------------
#define    BD_IO_Pin6_7_A1C3          //  北斗应用用 PA1    6   灰线 PC3   7  绿线
//----- in pins  -------
#define  ACC_IO_Group          GPIOE               // ACC 管脚设置
#define  ACC_Group_NUM         GPIO_Pin_9

#define  WARN_IO_Group         GPIOE               // 紧急报警 管脚设置
#define  WARN_Group_NUM        GPIO_Pin_8  



//---- 报警状态  ------------------
   /*  
     -------------------------------------------------------------
              F4  行车记录仪 TW705   管脚定义
     -------------------------------------------------------------
     遵循  GB10956 (2012)  Page26  表A.12  规定
    -------------------------------------------------------------
    | Bit  |      Note       |  必备|   MCUpin  |   PCB pin  |   Colour | ADC
    ------------------------------------------------------------
        D7      刹车           *            PE11             9                棕
        D6      左转灯     *             PE10            10               红
        D5      右转灯     *             PC2              8                白
        D4      远光灯     *             PC0              4                黑
        D3      近光灯     *             PC1              5                黄
        D2      雾灯          add          PC3              7                绿      *
        D1      车门          add          PA1              6                灰      *
        D0      预留
   */

#define BREAK_IO_Group                GPIOE                 //  刹车灯
#define BREAK_Group_NUM             GPIO_Pin_11

#define LEFTLIGHT_IO_Group         GPIOE                // 左转灯
#define LEFTLIGHT_Group_NUM       GPIO_Pin_10

#define RIGHTLIGHT_IO_Group       GPIOC               // 右转灯
#define RIGHTLIGHT_Group_NUM      GPIO_Pin_2

#define FARLIGHT_IO_Group         GPIOC              // 远光灯
#define FARLIGHT_Group_NUM        GPIO_Pin_0 

#define NEARLIGHT_IO_Group        GPIOA             // 近光灯
#define NEARLIGHT_Group_NUM       GPIO_Pin_6  

#define FOGLIGHT_IO_Group          GPIOA          //  雾灯
#define FOGLIGHT_Group_NUM         GPIO_Pin_7      

#define DOORLIGHT_IO_Group        GPIOA             // 车门灯   预留
#define DOORLIGHT_Group_NUM       GPIO_Pin_1


//------  out pins ---
#define RELAY_IO_Group           GPIOB               //继电器
#define RELAY_Group_NUM          GPIO_Pin_1
 
#define Buzzer_IO_Group          GPIOB               //蜂鸣器 
#define Buzzer_Group_Num         GPIO_Pin_6  



#define  timer_tim1counter         100  // 14111        // 168*84=1412      -1  =14111


//-----  WachDog related----
extern u8    wdg_reset_flag;    //  Task Idle Hook 相关
extern u16    ADC_ConvertedValue; //电池电压AD数值    
extern  u16   ADC_ConValue[3];   //   3  个通道ID      
extern  u32   TIM1_Timer_Counter; //  测试定时器计数器 
extern 	u32   TIM3_Timer_10ms_counter; // 10ms timer   



/*    
     -----------------------------
    1.    输入管脚状态监测
     ----------------------------- 
*/
extern void  WatchDog_Feed(void);
extern void  WatchDogInit(void); 
extern void  APP_IOpinInit(void) ;
//  INPUT
extern u8    ACC_StatusGet(void);
extern u8    WARN_StatusGet(void);
extern u8    MainPower_cut(void);
extern u8   FarLight_StatusGet(void);
extern u8   WarnLight_StatusGet(void);
extern u8  Speaker_StatusGet(void);
extern u8  LeftLight_StatusGet(void);
extern u8  DoorLight_StatusGet(void);
extern u8  RightLight_StatusGet(void);
extern u8  BreakLight_StatusGet(void);
extern u8 RainBrush_StatusGet(void);

//   OUTPUT
extern void  Enable_Relay(void);
extern void  Disable_Relay(void);
extern void  IO_statusCheck(void); 
extern void  ACC_status_Check(void);


/*    
     -----------------------------
    2.  应用相关
     ----------------------------- 
*/
extern   void TIM2_Configuration(void); 
extern   void Init_ADC(void); 
/*    
     -----------------------------
    3.  RT 驱动相关
     ----------------------------- 
*/


/*
       -----------------------------
       新驱动应用
      -----------------------------
*/
//  1 .  循环存储 
extern   u8       Api_cycle_write(u8 *buffer, u16 len);
extern   u8       Api_cycle_read(u8 *buffer, u16 len); 
extern   u8       Api_cycle_Update(void);
extern   u8       Api_CHK_ReadCycle_status(void);  

 /*
		 ----------------------------
		蜂鸣器驱动相关
		 ----------------------------
 */
 extern  void GPIO_Config_PWM(void); 
 extern  void TIM_Config_PWM(void);


 // 2. Config 
 extern   u8    Api_Config_write(u8 *name,u16 ID,u8* buffer, u16 wr_len);

 //  3.  其他 
 extern   u8   Api_DFdirectory_Create(u8* name, u16 sectorNum);
 extern   void   Api_MediaIndex_Init(void);
 extern   u32  Api_DFdirectory_Query(u8 *name, u8  returnType);
 extern   u8   Api_DFdirectory_Write(u8 *name,u8 *buffer, u16 len); 
 extern   u8    Api_DFdirectory_Read(u8 *name,u8 *buffer, u16 len, u8  style ,u16 numPacket);  // style  1. old-->new   0 : new-->old 
 extern   u8   Api_DFdirectory_Delete(u8* name); 
 extern   u8   API_List_Directories(void );
 extern  void  Api_WriteInit_var_rd_wr(void);    //   写初始化话各类型读写记录地址
 extern  void  Api_Read_var_rd_wr(void);    //   读初始化话各类型读写记录地址 



 extern   u8     Api_Config_Recwrite_Large(u8 *name,u16 ID,u8* buffer, u16 wr_len);
 extern  u8      Api_Config_read(u8 *name,u16 ID,u8* buffer, u16 Rd_len); 
 extern   u8     Api_RecordNum_Write( u8 *name,u8 Rec_Num,u8 *buffer, u16 len);  
 extern    u8    Api_RecordNum_Read( u8 *name,u8 Rec_Num,u8 *buffer, u16 len);  

 
 extern   u8     ISP_Read( u32  Addr, u8*  Instr, u16 len);        
 extern   u8     ISP_Write( u32  Addr, u8*  Instr, u16 len);
 extern   u8     ISP_Format(u16 page_counter,u16 page_offset,u8 *p,u16 length);

 extern u8     TF_Card_Status(void);
 

#endif
