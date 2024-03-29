/*
     Protocol_808.C
 */

#include <rtthread.h>
#include <rthw.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include "math.h"
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "math.h"
#include "stdarg.h"
#include "string.h"

#define   SYSID 0x037a

#define    ROUTE_DIS_Default 0x3F000000

#define   BLIND_NUM        10100  
#define   MQ_PKNUM 20

//----   多媒体发送状态 -------
_Media_SD_state Photo_sdState;      //  图片发送状态
_Media_SD_state Sound_sdState;      //声音发送
_Media_SD_state Video_sdState;      //视频发送
_Media_SD_state DrvRecoder_sdState; //行车记录仪发送状态
_Media_SD_state BlindZone_sdState;  //盲区发送

//------ Photo -----
u32 PicFileSize = 0;                // 图片文件大小
u8	PictureName[40];

//------  voice -----
u8	local_trig	= 0;                //  本地定时上报
u32 local_timer = 0;                // 本地定时计数器

//------  video  --------
u8 line_warn_enable = 0;            // 使能路线报警


/*
             杂
 */
//------ phone
u8 CallState = CallState_Idle;      // 通话状态

//   ASCII  to   GB    ---- start
//0-9        10
u8 arr_A3B0[20] = { 0xA3, 0xB0, 0xA3, 0xB1, 0xA3, 0xB2, 0xA3, 0xB3, 0xA3, 0xB4, 0xA3, 0xB5, 0xA3, 0xB6, 0xA3, 0xB7, 0xA3, 0xB8, 0xA3, 0xB9 };

//@ A-O      16
u8 arr_A3C0[32] = { 0xA3, 0xC0, 0xA3, 0xC1, 0xA3, 0xC2, 0xA3, 0xC3, 0xA3, 0xC4, 0xA3, 0xC5, 0xA3, 0xC6, 0xA3, 0xC7, 0xA3, 0xC8, 0xA3, 0xC9, 0xA3, 0xCA, 0xA3, 0xCB, 0xA3, 0xCC0, 0xA3, 0xCD, 0xA3, 0xCE, 0xA3, 0xCF };

//P-Z         11个
u8 arr_A3D0[22] = { 0xA3, 0xD0, 0xA3, 0xD1, 0xA3, 0xD2, 0xA3, 0xD3, 0xA3, 0xD4, 0xA3, 0xD5, 0xA3, 0xD6, 0xA3, 0xD7, 0xA3, 0xD8, 0xA3, 0xD9, 0xA3, 0xDA };

//.  a-0       16
u8 arr_A3E0[32] = { 0xA3, 0xE0, 0xA3, 0xE1, 0xA3, 0xE2, 0xA3, 0xE3, 0xA3, 0xE4, 0xA3, 0xE5, 0xA3, 0xE6, 0xA3, 0xE7, 0xA3, 0xE8, 0xA3, 0xE9, 0xA3, 0xEA, 0xA3, 0xEB, 0xA3, 0xEC, 0xA3, 0xED, 0xA3, 0xEE, 0xA3, 0xEF };

//p-z          11
u8 arr_A3F0[22] = { 0xA3, 0xF0, 0xA3, 0xF1, 0xA3, 0xF2, 0xA3, 0xF3, 0xA3, 0xF4, 0xA3, 0xF5, 0xA3, 0xF6, 0xA3, 0xF7, 0xA3, 0xF8, 0xA3, 0xF9, 0xA3, 0xFA };
//-------  ASCII to GB ------

u8	spd_dex[420];
u8	Latiude_hex[420];

u8  _700H_buffer[700]; 


/*
   //------------------ 定位精度process    --------------------------------------------
   u16 spd_dex[420]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,17,26,35,
                  44,53,62,71,80,89,98,107,116,125,134,143,152,161,170,179,188,197,206,
                  215,224,233,242,252,261,270,279,288,297,306,315,324,333,342,351,360,
                  369,378,387,396,405,414,423,432,441,450,459,468,477,486,495,504,513,
                  522,531,540,549,558,567,576,585,594,603,612,621,630,639,648,657,666,
                  675,684,693,702,711,720,729,738,747,756,765,774,783,792,801,810,819,
                  828,837,846,855,864,873,882,891,900,909,918,927,936,945,954,963,972,
                  981,990,999,1008,1017,1026,1035,1044,1053,1062,1071,1080,1080,1080,1080
                  ,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,1080,
                  1080,1080,1080,1080,1080,1071,1062,1053,1044,1035,1026,1017,1008,999,990,
                  981,972,963,954,945,936,927,918,909,900,891,882,873,864,855,846,837,828,819,
                  810,801,792,783,774,765,756,747,738,729,720,711,702,693,684,675,666,657,648,
                  639,630,621,612,603,594,585,576,567,558,549,540,531,522,513,504,495,486,477,
                  468,459,450,441,432,423,414,405,396,387,378,369,360,351,342,333,324,315,306,
                  297,288,279,270,261,252,243,233,224,215,206,197,188,179,170,161,152,143,134,
                  125,116,107,98,89,80,71,62,53,44,35,26,17,8};
   u32 Latiude_hex[420]={0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,
   0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,
   0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,
   0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x2625A00,0x26259FF,0x26259FC,
   0x26259F6,0x26259EE,0x26259E4,0x26259D8,0x26259C9,0x26259B8,0x26259A5,0x2625990,0x2625978,0x262595E,0x2625942,0x2625924,0x2625903,0x26258E0,0x26258BB,0x2625894,0x262586A,0x262583E,
   0x2625810,0x26257E0,0x26257AD,0x2625778,0x2625741,0x2625707,0x26256CC,0x262568E,0x262564E,0x262560B,0x26255C7,0x2625580,0x2625537,0x26254EB,0x262549D,0x262544E,0x26253FB,0x26253A7,
   0x2625350,0x26252F7,0x262529C,0x262523F,0x26251DF,0x262517D,0x2625119,0x26250B3,0x262504A,0x2624FDF,0x2624F72,0x2624F02,0x2624E91,0x2624E1D,0x2624DA6,0x2624D2E,0x2624CB3,0x2624C36,
   0x2624BB7,0x2624B36,0x2624AB2,0x2624A2C,0x26249A4,0x2624919,0x262488D,0x26247FE,0x262476C,0x26246D9,0x2624643,0x26245AB,0x2624511,0x2624474,0x26243D6,0x2624335,0x2624291,0x26241EC,
   0x2624144,0x262409A,0x2623FEE,0x2623F3F,0x2623E8F,0x2623DDC,0x2623D26,0x2623C6F,0x2623BB5,0x2623AF9,0x2623A3B,0x262397A,0x26238B8,0x26237F3,0x262372B,0x2623662,0x2623596,0x26234C8,
   0x26233F8,0x2623325,0x2623250,0x2623179,0x26230A0,0x2622FC5,0x2622EE7,0x2622E07,0x2622D24,0x2622C40,0x2622B59,0x2622A70,0x2622985,0x2622897,0x26227A7,0x26226B5,0x26225C1,0x26224CA,
   0x26223D2,0x26222D7,0x26221D9,0x26220DA,0x2621FD8,0x2621ED4,0x2621DCD,0x2621CC5,0x2621BBA,0x2621AAD,0x262199F,0x2621891,0x2621782,0x2621674,0x2621566,0x2621458,0x262134A,0x262123B,
   0x262112D,0x262101F,0x2620F11,0x2620E03,0x2620CF5,0x2620BE6,0x2620AD8,0x26209CA,0x26208BC,0x26207AE,0x262069F,0x2620591,0x2620483,0x2620375,0x2620267,0x2620159,0x262004A,0x261FF3C,
   0x261FE2E,0x261FD20,0x261FC12,0x261FB03,0x261F9F5,0x261F8E7,0x261F7D9,0x261F6CB,0x261F5BC,0x261F4AE,0x261F3A0,0x261F292,0x261F184,0x261F076,0x261EF67,0x261EE59,0x261ED4B,0x261EC3D,
   0x261EB2F,0x261EA20,0x261E912,0x261E804,0x261E6F6,0x261E5E8,0x261E4DA,0x261E3CB,0x261E2BD,0x261E1AF,0x261E0A1,0x261DF93,0x261DE84,0x261DD76,0x261DC68,0x261DB5A,0x261DA4C,0x261D93D,
   0x261D82F,0x261D721,0x261D613,0x261D505,0x261D3F7,0x261D2E8,0x261D1DA,0x261D0CC,0x261CFBE,0x261CEB0,0x261CDA1,0x261CC93,0x261CB85,0x261CA77,0x261C969,0x261C85B,0x261C74C,0x261C63E,
   0x261C530,0x261C422,0x261C314,0x261C205,0x261C0F7,0x261BFE9,0x261BEDB,0x261BDCD,0x261BCBE,0x261BBB0,0x261BAA2,0x261B994,0x261B886,0x261B778,0x261B669,0x261B55B,0x261B44D,0x261B33F,
   0x261B231,0x261B122,0x261B014,0x261AF06,0x261ADF8,0x261ACEA,0x261ABDB,0x261AACD,0x261A9BF,0x261A8B1,0x261A7A3,0x261A695,0x261A586,0x261A478,0x261A36A,0x261A25C,0x261A14E,0x261A03F,
   0x2619F31,0x2619E23,0x2619D15,0x2619C07,0x2619AFA,0x26199EF,0x26198E6,0x26197E0,0x26196DC,0x26195DA,0x26194DA,0x26193DD,0x26192E2,0x26191E9,0x26190F3,0x2618FFE,0x2618F0C,0x2618E1C,
   0x2618D2F,0x2618C44,0x2618B5B,0x2618A74,0x261898F,0x26188AD,0x26187CD,0x26186EF,0x2618614,0x261853A,0x2618463,0x261838E,0x26182BC,0x26181EC,0x261811E,0x2618052,0x2617F88,0x2617EC1,
   0x2617DFC,0x2617D39,0x2617C79,0x2617BBA,0x2617AFE,0x2617A45,0x261798D,0x26178D8,0x2617825,0x2617774,0x26176C6,0x2617619,0x261756F,0x26174C8,0x2617422,0x261737F,0x26172DE,0x261723F,
   0x26171A3,0x2617108,0x2617070,0x2616FDB,0x2616F47,0x2616EB6,0x2616E27,0x2616D9A,0x2616D10,0x2616C88,0x2616C02,0x2616B7E,0x2616AFC,0x2616A7D,0x2616A00,0x2616986,0x261690D,0x2616897,
   0x2616823,0x26167B1,0x2616742,0x26166D5,0x261666A,0x2616601,0x261659B,0x2616536,0x26164D4,0x2616475,0x2616417,0x26163BC,0x2616363,0x261630D,0x26162B8,0x2616266,0x2616216,0x26161C8,
   0x261617D,0x2616134,0x26160ED,0x26160A8,0x2616066,0x2616026,0x2615FE8,0x2615FAC,0x2615F73,0x2615F3C,0x2615F07,0x2615ED4,0x2615EA4,0x2615E75,0x2615E4A,0x2615E20,0x2615DF9,0x2615DD3,
   0x2615DB1,0x2615D90,0x2615D72,0x2615D55,0x2615D3C,0x2615D24,0x2615D0F,0x2615CFB,0x2615CEB,0x2615CDC,0x2615CD0,0x2615CC5,0x2615CBE,0x2615CB8,0x2615CB5};




 */

//----------- 行车记录仪相关  -----------------
Avrg_MintSpeed	Avrgspd_Mint;
u32				PerMinSpdTotal	= 0;    //记录每分钟速度总数
u8				avgspd_Mint_Wr	= 0;    // 填写每分钟平均速度记录下标
u8				avgspd_Sec_Wr	= 0;    // 填写每秒钟平均速度记录下标
u8				avgWriteOver	= 0;    // 写溢出标志位
u8				AspdCounter		= 0;    // 每分钟速度有效报数计数器
u8				Vehicle_sensor	= 0;    // 车辆传感器状态   0.2s  查询一次


/*
   D7  刹车
   D6  左转灯
   D5  右转灯
   D4  远光灯
   D3  近光灯
   D2  雾灯
   D1  车门
   D0  预留
 */
u8			Vehicle_sensor_BAK = 0;     // 车辆传感器状态	0.2s  查询一次

DOUBT_TYPE	Sensor_buf[100];            // 20s 状态记录
u8			save_sensorCounter	= 0, sensor_writeOverFlag = 0;;
u32			total_plus			= 0;

//   -------  CAN BD new  --------------
CAN_TRAN	CAN_trans;

u8			Camera_Number	= 1;
u8			DispContent		= 1;    // 发送时是否显示数据内容


/*
            1 <->  正常显示
            2 <->  显示发送信息的
            3 <->  显示 任务的运行情况
            0<-> 不显示调试输出，只显示协议数据
 */

u8	TextInforCounter = 0;           //文本信息条数

u8	FCS_GPS_UDP = 0;                //UDP 数据异或和
u8	FCS_RX_UDP	= 0;                // UDP 数据接收校验

u8	Centre_IP_modify	= 0;        //  中修改IP了
u8	IP_change_counter	= 0;        //   中心修改IP 计数器
u8	Down_Elec_Flag		= 0;        //   断油断电使能标志位

//------------ 超速报警---------------------
SPD_EXP		speed_Exd;

GPRMC_PRO	GPRMC_Funs =
{
	Time_pro,
	Status_pro,
	Latitude_pro,
	Lat_NS_pro,
	Longitude_pro,
	Long_WE_pro,
	Speed_pro,
	Direction_pro,
	Date_pro
};

//--------  GPS prototcol----------------------------------------------------------------------------------
static u32	fomer_time_seconds, tmp_time_secnonds, delta_time_seconds;
u8			UDP_dataPacket_flag = 0x03;             /*V	   0X03      ;		   A	  0X02*/
u8			GPS_getfirst		= 0, Shoushi = 0;;  //  首次有经纬度
u8			HDOP_value			= 99;               //  Hdop 数值
u8			Satelite_num		= 0;                // 卫星颗数
u8			CurrentTime[3];
u8			BakTime[3];
u8			Sdgps_Time[3];                          // GPS 发送 时间记录   BCD 方式
u8			gps_log[50];                            //gps log

//static u8      UDP_AsciiTx[1800];
ALIGN( RT_ALIGN_SIZE )
u8 GPRS_info[3000];
u16 GPRS_infoWr_Tx = 0;

ALIGN( RT_ALIGN_SIZE )
u8 UDP_HEX_Rx[1024];                                    // EM310 接收内容hex
u16		UDP_hexRx_len		= 0;                        // hex 内容 长度
u16		UDP_DecodeHex_Len	= 0;                        // UDP接收后808 解码后的数据长度

GPS_RMC GPRMC;                                          // GPMC格式
BD_SEND  BDSD;  //    北斗顺序发送


/*                         pGpsRmc->status,\
   pGpsRmc->latitude_value,\
   pGpsRmc->latitude,\
   pGpsRmc->longtitude_value,\
   pGpsRmc->longtitude,\
   pGpsRmc->speed,\
   pGpsRmc->azimuth_angle);
 */

//----------808 协议 -------------------------------------------------------------------------------------
u16			GPS_Hight		= 0;                        //   808协议-> 高程   m
u16			GPS_speed		= 0;                        //   808协议-> 速度   0.1km/h
u16			GPS_direction	= 0;                        //   808协议-> 方向   度
u16			Centre_FloatID	= 0;                        //  中心消息流水号
u16			Centre_CmdID	= 0;                        //  中心命令ID

u8			Original_info[1024];                        // 没有转义处理前的原始信息
u16			Original_info_Wr = 0;                       // 原始信息写地址
//---------- 用GPS校准特征系数相关 ----------------------------
u8			Speed_area			= 60;                   // 校验K值范围
u16			Speed_gps			= 0;                    // 通过GPS计算出来的速度 0.1km/h
u8			Speed_Rec			= 0;                    // 速度传感器 校验K用的存储器
u16			Speed_cacu			= 0;                    // 通过K值计算出来的速度
u16			Spd_adjust_counter	= 0;                    // 确保匀速状态计数器
u16			Former_DeltaPlus[K_adjust_Duration];        // 前几秒的脉冲数
u8			Former_gpsSpd[K_adjust_Duration];           // 前几秒的速度
u8			DF_K_adjustState = 0;                       // 特征系数自动校准状态说明  1:自动校准过    0:尚未自动校准
//-----  车台注册定时器  ----------
DevRegst	DEV_regist;                                 // 注册
DevLOGIN	DEV_Login;                                  //  鉴权

//------- 文本信息下发 -------
TEXT_INFO	TextInfo;                                   // 文本信息下发
//------- 事件 ----
EVENT		EventObj;                                   // 事件
EVENT		EventObj_8[8];                              // 事件
//-------文本信息-------
MSG_TEXT	TEXT_Obj;
MSG_TEXT	TEXT_Obj_8[8], TEXT_Obj_8bak[8];

//------ 提问  --------
CENTRE_ASK		ASK_Centre;                             // 中心提问
//------  信息点播  ---
MSG_BRODCAST	MSG_BroadCast_Obj;                      // 信息点播
MSG_BRODCAST	MSG_Obj_8[8];                           // 信息点播
//------  电话本  -----
PHONE_BOOK		PhoneBook, Rx_PhoneBOOK;                //  电话本
PHONE_BOOK		PhoneBook_8[20];

//-----  车辆控制 ------
VEHICLE_CONTROL Vech_Control;                           //  车辆控制
//-----  电子围栏  -----
POLYGEN_RAIL	Rail_Polygen;                           // 多边形围栏
RECT_RAIL		Rail_Rectangle;                         // 矩形围栏
RECT_RAIL       Rail_Rectangle_multi[8]; // 矩形围栏
CIRCLE_RAIL     Rail_Cycle_multi[8];     // 圆形围栏

CIRCLE_RAIL		Rail_Cycle;                             // 圆形围栏
//------- 线路设置 -----
POINT			POINT_Obj;                              // 路线的拐点
ROUTE			ROUTE_Obj;                              // 路线相关
//-------    行车记录仪  -----
RECODER			Recode_Obj;                             // 行车记录仪
//-------  拍照  ----
CAMERA			Camera_Obj;                             //  中心拍照相关
//-----   录音  ----
VOICE_RECODE	VoiceRec_Obj;                           //  录音功能
//------ 多媒体  --------
MULTIMEDIA		MediaObj;                               // 多媒体信息
//-------  数据信息透传  -------
DATATRANS		DataTrans;                              // 数据信息透传
//-------  进出围栏状态 --------
INOUT			InOut_Object;                           // 进出围栏状态
//-------- 多媒体检索  ------------
MEDIA_INDEX		MediaIndex;                             // 多媒体信息
//------- 车辆负载状态 ---------------
u8				CarLoadState_Flag = 1;                  //选中车辆状态的标志   1:空车   2:半空   3:重车

//------- 多媒体信息类型---------------
u8	Multimedia_Flag = 1;                                //需要上传的多媒体信息类型   1:视频   2:音频   3:图像
u8	SpxBuf[SpxBuf_Size];
u16 Spx_Wr			= 0, Spx_Rd = 0;
u8	Duomeiti_sdFlag = 0;

//------- 录音开始或者结束---------------
u8 Recor_Flag = 1;                                      //  1:录音开始   2:录音结束

//----------808协议 -------------------------------------------------------------------------------------
u8				SIM_code[6];                            // 要发送的IMSI	号码
u8				IMSI_CODE[15]	= "000000000000000";    //SIM 卡的IMSI 号码
u8				Warn_Status[4]	=
{
	0x00, 0x00, 0x00, 0x00
};                                                      //  报警标志位状态信息
u8				Car_Status[4] =
{
	0x00, 0x00, 0x00, 0x00
};                                                      //  车辆状态信息
T_GPS_Info_GPRS Gps_Gprs, Bak_GPS_gprs;
T_GPS_Info_GPRS Temp_Gps_Gprs;

u8   EverySecond_Time_Get=0;                   // 获取每秒时间标志  0: not get  1: get  day not change  2 : get  day need change
u8   Lati_Get=0;                               // 纬度GGA  处理过
u8   Longi_Get=0;                              //  精度 GGA  接收 过






u8				A_time[6];                              // 定位时刻的时间

u8				ReadPhotoPageTotal	= 0;
u8				SendPHPacketFlag	= 0;                ////收到中心启动接收下一个block时置位

//-------- 紧急报警 --------
u8	warn_flag			= 0;
u8	f_Exigent_warning	= 0;                            //0;     //脚动 紧急报警装置 (INT0 PD0)
u8	Send_warn_times		= 0;                            //   设备向中心上报报警次数，最大3 次
u32 fTimer3s_warncount	= 0;

//------  车门开关拍照 -------
DOORCamera DoorOpen;                                    //  开关车门拍照

//------- 北斗扩展协议  ------------
GNSS_RAW			GNSS_rawdata;                       //  GNSS 详细数据上报
BD_EXTEND			BD_EXT;                             //  北斗扩展协议
DETACH_PKG			Detach_PKG;                         // 分包重传相关
SET_QRY				Setting_Qry;                        //  终端参数查询
PRODUCT_ATTRIBUTE	ProductAttribute;                   // 终端属性
HUMAN_CONFIRM_WARN	HumanConfirmWarn;                   // 人工确认报警

// ---- 拐点 -----
u16 Inflexion_Current		= 0;
u16 Inflexion_Bak			= 0;
u16 Inflexion_chgcnter		= 0;                        //变化计数器
u16 InflexLarge_or_Small	= 0;                        // 判断curent 和 Bak 大小    0 equql  1 large  2 small
u16 InflexDelta_Accumulate	= 0;                        //  差值累计

// ----休眠状态  ------------
u8	SleepState		= 0;                                //   0  不休眠ACC on            1  休眠Acc Off
u8	SleepConfigFlag = 0;                                //  休眠时发送鉴权标志位

//---- 固定文件大小 ---
u32 mp3_fsize		= 5616;
u8	mp3_sendstate	= 0;
u32 wmv_fsize		= 25964;
u8	wmv_sendstate	= 0;

//-------------------   公共 ---------------------------------------
static u8	GPSsaveBuf[40];             // 存储GPS buffer
static u8	ISP_buffer[520];
static u16	GPSsaveBuf_Wr = 0;

POSIT		Posit[60];                  // 每分钟位置信息存储
u8			PosSaveFlag = 0;            // 存储Pos 状态位

NANDSVFlag	NandsaveFlg;
A_AckFlag	Adata_ACKflag;              // 无线GPRS协议 接收相关 RS232 协议返回状态寄存器
TCP_ACKFlag SD_ACKflag;                 // 无线GPRS协议返回状态标志
u32			SubCMD_8103H	= 0;        //  02 H命令 设置记录仪安装参数回复 子命令
u32			SubCMD_FF01H	= 0;        //  FF02 北斗信息扩展
u32			SubCMD_FF03H	= 0;        //  FF03  设置扩展终端参数设置1

u8			SubCMD_10H			= 0;    //  10H   设置记录仪定位告警参数
u8			OutGPS_Flag			= 0;    //  0  默认  1  接外部有源天线
u8			Spd_senor_Null		= 0;    // 手动传感器速度为0
u32			Centre_DoubtRead	= 0;    //  中心读取事故疑点数据的读字段
u32			Centre_DoubtTotal	= 0;    //  中心读取事故疑点的总字段
u8			Vehicle_RunStatus	= 0;    //  bit 0: ACC 开 关             1 开  0关
//  bit 1: 通过速度传感器感知    1 表示行驶  0 表示停止
//  bit 2: 通过gps速度感知       1 表示行驶  0 表示停止
u8	Status_TiredwhRst = 0;              //  当复位时 疲劳驾驶的状态   0 :停车  1:停车但没触发 2:触发了还没结束

u32 SrcFileSize		= 0, DestFilesize = 0, SrcFile_read = 0;
u8	SleepCounter	= 0;

u16 DebugSpd		= 0;                //调试用GPS速度
u8	MMedia2_Flag	= 0;                // 上传固有音频 和实时视频  的标志位    0 传固有 1 传实时

u8	tts_bro_tired_flag = 0;             // 为了认证  ， 只播报一次 疲劳驾驶已经解除

//-----  ISP    远程下载相关 -------
ISP_BD BD_ISP;                          //  BD   升级包

//------ IC 卡模块 ---
IC_MODULE			IC_MOD;             //IC 模块

unsigned short int	FileTCB_CRC16	= 0;
unsigned short int	Last_crc		= 0, crc_fcs = 0;

//---------  中心应答  -----------
u8				Send_Rdy4ok = 0;
unsigned char	Rstart_time = 0;

MQU				MangQU;
MQU             MQ_TrueUse; // 真正的盲区补报模式     

//---------- SMS SD ------------------
SMS_SD SMS_send;

//---------------  速度脉冲相关--------------
u16			Delta_1s_Plus	= 0;
u16			Delta_1s_Plus2	= 0;
u16			Sec_counter		= 0;

LENGTH_BUF	Rx_reg;
LENGTH_BUF	app_rx;
u32			Appringbuf_wr	= 0;            // 循环存储buffer
u32			AppRingbuf_rd	= 0;


//---------74CH595  Q5   control Power----
u8   Print_power_Q5_enable=0;   
u8   Buzzer_on_Q7_enable=0; 

u32     MQsend_counter=0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++行车记录仪+++++
u8 t_hour=0,t_min=0,t_second=0;

void K_AdjustUseGPS( u32 sp, u32 sp_DISP ); // 通过GPS 校准  K 值  (车辆行驶1KM 的脉冲数目)


u16 Protocol_808_Encode( u8 *Dest, u8 *Src, u16 srclen );


void Protocol_808_Decode( void );           // 解析指定buffer :  UDP_HEX_Rx


void Photo_send_end( void );


void Sound_send_end( void );


void Video_send_end( void );
void mq_true_enable(u8 value);



unsigned short int CRC16_file( unsigned short int num );


void Spd_ExpInit( void );


void AvrgSpd_MintProcess( u8 hour, u8 min, u8 sec );


u32 Distance_Point2Line( u32 Cur_Lat, u32 Cur_Longi, u32 P1_Lat, u32 P1_Longi, u32 P2_Lat, u32 P2_Longi );


void RouteRail_Judge( u8* LatiStr, u8* LongiStr );


void app_queenable( u8* instr );
u32 Get_MQ_true_CuurentTotal_packets(void);



//  A.  Total

void  Mangqu_Init( void )
{
	MangQU.PacketNum	= 0;
	MangQU.Sd_timer		= 0;
	MangQU.Sd_flag		= 0;
	//---  5-3
	MangQU.Enable_SD_state = 0;

    //  another  struct  init	
	MQ_TrueUse.PacketNum	= 0;
	MQ_TrueUse.Sd_timer		= 0;
	MQ_TrueUse.Sd_flag		= 0;
	//---  5-3
	MQ_TrueUse.Enable_SD_state = 0; 
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void delay_us( u16 j )
{
	u8 i;
	while( j-- )
	{
		i = 3;
		while( i-- )
		{
			;
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void delay_ms( u16 j )
{
	while( j-- )
	{
		DF_delay_us( 2000 ); // 1000
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void App_que_process( void )
{
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Do_SendGPSReport_GPRS( void )
{
	unsigned short int	crc_file	= 0;
	u16					count		= 0, str_len = 0;
	u8					i			= 0;
	u8					reg_gps[30], reg_wr = 0;
	u8					packet_type = 0;

     //  顺序上报BD 信息
        if(BDSD.Enable_Working==1)
            {
		         if( BDSD.SendFlag==RdCycle_RdytoSD)
		         {
                   Stuff_BDSD_0200H();
                   BDSD.SendFlag=RdCycle_SdOver;
				   return true;
		         }
        	}
	// 1.  GNSS 详细数据上传----------------------------------------------------
	//          真正补报
    if( MQ_TrueUse.Enable_SD_state == 1 )
	{
		if( ( MQ_TrueUse.Sd_flag == 1 ) && ( DEV_Login.Operate_enable == 2 ) )
		{
		                
		               
		                Stuff_MangQu_Packet_Send_0704H_True();  
						MQ_TrueUse.Sd_flag=2;  //2;  
		    return true;            
		}		
	}
	//--------------------------------------------------------------------------------
	if( MangQU.Enable_SD_state == 1 )
	{
		if( ( MangQU.Sd_flag == 1 ) && ( DEV_Login.Operate_enable == 2 ) )
		{
		                Stuff_MangQu_Packet_Send_0704H();  
						//MangQU.Sd_flag=0;
		                MangQU.Sd_flag=2;  //2;  
		              /*  MangQU.PacketNum++;
					    if(MangQU.PacketNum>=(BLIND_NUM/MQ_PKNUM))
									  	    	{
									  	    	   MangQU.PacketNum=0;
												   MangQU.Enable_SD_state=0; 
												   rt_kprintf("\r\nReturn Normal all\r\n");  
									  	    	}
									  	    	*/
				return true;					  	    	
		}		
	}
	if( ( 1 == GNSS_rawdata.WorkEnable ) && ( GNSS_rawdata.save_status ) )
	{
		if( GNSS_rawdata.save_status & ( 1 << GNSS_rawdata.rd_num ) )   // 检查当前rd   数值状态
		{
			str_len = strlen( GNSS_rawdata.Raw[GNSS_rawdata.rd_num] );
			// for(count=0;count<str_len;count++)
			// rt_kprintf("%c",GNSS_rawdata.Raw[GNSS_rawdata.rd_num][count]);
			Stuff_GNSSRawData_0900H( GNSS_rawdata.Raw[GNSS_rawdata.rd_num], str_len );
			//---------------------------
			GNSS_rawdata.rd_num++;
			if( GNSS_rawdata.rd_num > 3 )
			{
				GNSS_rawdata.rd_num = 0;
			}
			GNSS_rawdata.save_status &= ~( 1 << GNSS_rawdata.rd_num ); //clear
		}
		return true;
	}
	//2.  鉴权--------------------------------------------------------------------
	if( DEV_Login.Operate_enable != 2 )
	{
		if( 1 == DEV_Login.Enable_sd )
		{
			Stuff_DevLogin_0102H( ); //  鉴权   ==2 时鉴权完毕
			DEV_Login.Enable_sd = 0;
			//------ 发送鉴权不判断 ------------------
			//DEV_Login.Operate_enable=2;  //  不用判断鉴权了
			return true;
		}
	}
	// 3. 多媒体数据上传
	if( MediaObj.Media_transmittingFlag == 2 )
	{
		if( 1 == MediaObj.SD_Data_Flag )
		{
			Stuff_MultiMedia_Data_0801H( );
			MediaObj.SD_Data_Flag = 0;
			return true;
		}
		return true;                        // 按照808 协议要求 ，传输多媒体过程中不允许发送别的信息包
	}
	//  4.   终端注册
	if( 1 == DEV_regist.Enable_sd )
	{
		Stuff_RegisterPacket_0100H( 0 );    // 注册
		JT808Conf_struct.Msg_Float_ID = 0;
		Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
		DEV_regist.Enable_sd = 0;
		//  JT808Conf_struct.Regsiter_Status=1; //标注注册，但不存储
		return true;
	}
	// 5.  终端注销
	if( 1 == DEV_regist.DeRegst_sd )
	{
		Stuff_DeviceDeregister_0101H( );
		DEV_regist.DeRegst_sd = 0;
		return true;
	}
#if   0
	//  6.  终端心跳包
	if( ( 1 == JT808Conf_struct.DURATION.Heart_SDFlag ) && ( DataLink_Status( ) ) && ( SleepState == 0 ) && ( stopNormal == 0 ) )   //  心跳
	{
		Stuff_DeviceHeartPacket_0002H( );
		JT808Conf_struct.DURATION.Heart_SDFlag	= 0;
		JT808Conf_struct.DURATION.TCP_SD_state	= 1;                                                                                //发送完后置 1
		return true;
	}
	// 7.  休眠时终端心跳包
	if( ( 1 == SleepConfigFlag ) && ( DataLink_Status( ) ) && ( SleepState == 1 ) )                                                 //  休眠时心跳
	{
		Stuff_DevLogin_0102H( );                                                                                                    //  鉴权   ==2 时鉴权完毕
		rt_kprintf( "\r\n	 休眠时用鉴权做心跳包 ! \r\n");
		SleepConfigFlag = 0;
		return true;
	}
#endif
	//  8.   定位数据批量上传
	if( 1 == SD_ACKflag.f_BD_BatchTrans_0704H )                                                                                     //  定位数据批量上传
	{
		Stuff_BatchDataTrans_BD_0704H( );
		SD_ACKflag.f_BD_BatchTrans_0704H = 0;
		return true;
	}
	// 9.  中心拍照命令应答
	if( 1 == SD_ACKflag.f_BD_CentreTakeAck_0805H )                                                                                  // 中心拍照命令应答
	{
		Stuff_CentreTakeACK_BD_0805H( );
		SD_ACKflag.f_BD_CentreTakeAck_0805H = 0;
		return true;
	}
	//   10.  终端属性上传
	if( 1 == SD_ACKflag.f_BD_DeviceAttribute_8107 )                                                                                 //  终端属性上传
	{
		Stuff_DeviceAttribute_BD_0107H( );
		SD_ACKflag.f_BD_DeviceAttribute_8107 = 0;
		return true;
	}
	//   11.  位置信息查询
	if( 1 == SD_ACKflag.f_CurrentPosition_0201H )                                                                                   // 位置信息查询
	{
		Stuff_Current_Data_0201H( );
		SD_ACKflag.f_CurrentPosition_0201H = 0;
		return true;
	}
	//   12.   事件报告
	if( 1 == SD_ACKflag.f_CurrentEventACK_0301H )                                                                                   //  事件报告
	{
		Stuff_EventACK_0301H( );
		SD_ACKflag.f_CurrentEventACK_0301H = 0;
		return true;
	}
	//  13. 提问应答
	if( 2 == ASK_Centre.ASK_SdFlag )                                                                                                //  提问应答
	{
		Stuff_ASKACK_0302H( );
		ASK_Centre.ASK_SdFlag = 0;
		return true;
	}
	// 14. 车辆应答控制
	if( 1 == Vech_Control.ACK_SD_Flag )                                                                                             //  车辆应答控制
	{
		Stuff_ControlACK_0500H( );
		Vech_Control.ACK_SD_Flag = 0;
		return true;
	}
	//  15.  行车记录仪数据上传
	if( ( 1 == Recode_Obj.SD_Data_Flag ) && ( 1 == Recode_Obj.CountStep )&&(0==Recode_Obj.RSD_State) ) 
	{
		//  1. clear   one  packet  flag
		switch( Recode_Obj.CMD ) 
		{
			/*                divide  not  stop
			   case 0x08:
			   case 0x09:
			   case 0x10:
			   case 0x11:
			   case 0x12:
			   case 0x15:
			 */
			case 0x07:
			case 0x13:
			case 0x14:
				Recode_Obj.SD_Data_Flag = 0;
				Recode_Obj.CountStep	= 0;
				break;
		}
		//  2.  stuff   recorder   infomation
		//  judge  packet  type
		if( Recode_Obj.Devide_Flag == 1 )
		{
			packet_type = Packet_Divide;
		} else
		{
			packet_type = Packet_Normal;
		}

        
		rt_kprintf( "\r\n 记录仪 CMD_ID =0x%2X \r\n", Recode_Obj.CMD );
		if( packet_type == Packet_Divide )
		{
		  if(Recode_Obj.RSD_State==0)  // 在非列表重传情况下进行 bak  
		   {
		       Recode_Obj.Bak_current_num=Recode_Obj.Current_pkt_num;
               Recode_Obj.Bak_fcs= Recode_Obj.fcs;   
			   Recode_Obj.Bak_CMD=Recode_Obj.CMD;
		   }
		  
			rt_kprintf( "\r\n              current =%d  Total: %d \r\n", Recode_Obj.Current_pkt_num, Recode_Obj.Total_pkt_num );
		}
		Stuff_RecoderACK_0700H( packet_type ); //   行车记录仪数据上传
		//  3. step  by  step  send   from  00H  ---  07H
		switch( Recode_Obj.CMD )
		{
			case 0x00:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x01;
				break;
			case 0x01:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x02;
				break;
			case 0x02:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x03;
				break;
			case 0x03:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x04;
				break;
			case 0x04:    Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x05;
				break;
			case 0x05:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x06;
				break;
			//-----------------------------------------
			case 0x06:   Recode_Obj.SD_Data_Flag	= 1;
				Recode_Obj.CMD						= 0x07;
				break;
				//-----------------------------------------
		}

		if( Recode_Obj.CountStep == 1 )
		{
			Recode_Obj.CountStep = 2; Recode_Obj.timer = 0;
		}

		return true;
	}

       //15.2     记录仪 列表重传
       if((1 == Recode_Obj.RSD_State)&&(Recode_Obj.SD_Data_Flag==1))  
       	{
            
			if( Recode_Obj.Devide_Flag == 1 )
			{
				packet_type = Packet_Divide;
			} else
			{
				packet_type = Packet_Normal;
			}			
			rt_kprintf( "\r\n 记录仪列表重传 CMD_ID =0x%2X \r\n", Recode_Obj.CMD );
			if( packet_type == Packet_Divide )
			{
				rt_kprintf( "\r\n             current =%d  RsendTotal: %d  CurrentRsd=%d\r\n", Recode_Obj.Current_pkt_num, Recode_Obj.RSD_total,Recode_Obj.RSD_Reader);
			}
			Stuff_RecoderACK_0700H( packet_type ); //	行车记录仪数据上传

            if( Recode_Obj.RSD_Reader == Recode_Obj.RSD_total )
			{
				Recorder_init(0); //  置位等待状态，等待着中心再发重传指令
				rt_kprintf( "\r\n 记录仪列表重传结束!  CMD_ID =0x%2X  RsendTotal:%d CurrentRsd=%d\r\n", Recode_Obj.CMD,Recode_Obj.RSD_total,Recode_Obj.RSD_Reader);
                if(Recode_Obj.Transmit_running==1)
                	{
                	  Rcorder_Recover(); 
                      rt_kprintf( "\r\n 顺序执行尚未完成  current=%d  total=%d\r\n",Recode_Obj.Current_pkt_num,Recode_Obj.Total_pkt_num);
					  return true;
                	}
			}			
			Recode_Obj.SD_Data_Flag=0;// clear   
			return true;

       	}
	
	// 16. 消息点播
	if( SD_ACKflag.f_MsgBroadCast_0303H == 1 ) // 消息点播
	{
		Stuff_MSGACK_0303H( );
		SD_ACKflag.f_MsgBroadCast_0303H = 0;
		return true;
	}
	//17. 多媒体事件信息上传
	if( 1 == MediaObj.SD_media_Flag )
	{
		Stuff_MultiMedia_InfoSD_0800H( ); // 多媒体事件信息上传
		MediaObj.SD_media_Flag = 0;
		return true;
	}
	//  18.数据透传 做远程下载
	if( CAN_trans.canid_0705_sdFlag )
	{
		Stuff_CANDataTrans_BD_0705H( );
		CAN_trans.canid_0705_sdFlag = 0;    // clear
		//   DataTrans_Init();     //clear
		return true;
	}
	// 19. 多媒体索引上报
	if( SD_ACKflag.f_MediaIndexACK_0802H )
	{
		Stuff_MultiMedia_IndexAck_0802H( ); // 多媒体索引上报
		SD_ACKflag.f_MediaIndexACK_0802H = 0;
		return true;
	}
	// 20.驾驶员信息上报
	if( SD_ACKflag.f_DriverInfoSD_0702H )
	{
		Stuff_DriverInfoSD_0702H( );        //  驾驶员信息上报
		SD_ACKflag.f_DriverInfoSD_0702H = 0;
		return true;
	}
	//   21.电子运单
	if( SD_ACKflag.f_Worklist_SD_0701H )
	{
		Stuff_Worklist_0701H( );            //   电子运单
		SD_ACKflag.f_Worklist_SD_0701H = 0;
		return true;
	}
	//    22. 终端通用应答
	if( SD_ACKflag.f_CentreCMDack_0001H )
	{
		Stuff_DevCommmonACK_0001H( );
		if( SD_ACKflag.f_CentreCMDack_0001H == 2 )  //  修改IP设置了需要重拨
		{
			Close_DataLink( );   rt_kprintf( "\r\n Datalink end =>中心修改IP\r\n" );
		}                                           //  AT_END
		else
		if( SD_ACKflag.f_CentreCMDack_0001H == 3 )  //   远程复位
		{
			Systerm_Reset_counter	= Max_SystemCounter;
			ISP_resetFlag			= 2;            //   借助远程下载重启机制复位系统
		}else
		if( SD_ACKflag.f_CentreCMDack_0001H == 5 )  //   关闭数据通信
		{
			Close_DataLink( );
			Stop_Communicate( );
			rt_kprintf( "\r\n Datalink end =>关闭通信n\r\n" );
		}
		SD_ACKflag.f_CentreCMDack_0001H		= 0;
		SD_ACKflag.f_CentreCMDack_resualt	= 0;

		return true;
	}
	//  23.   查询终端参数
	if( SD_ACKflag.f_SettingPram_0104H )
	{
		Stuff_SettingPram_0104H( SD_ACKflag.f_SettingPram_0104H );
		SD_ACKflag.f_SettingPram_0104H = 0;
		return true;
	}
	//  24 .  终端升级结果上报
	if( 1 == SD_ACKflag.f_BD_ISPResualt_0108H )
	{
		Stuff_ISP_Resualt_BD_0108H( );
		SD_ACKflag.f_BD_ISPResualt_0108H = 0;
		return true;
	}
	//  25.  IC 卡透传消息
	if( ( 1 == IC_MOD.Trans_0900Flag ) && ( 1 == TCP2_Connect ) )
	{
		Stuff_DataTrans_0900_BD_ICinfo( );
		IC_MOD.Trans_0900Flag = 0;
		return true;
	}
	//           位置数据上报
	if( MangQU.Enable_SD_state == 1 )
	{
		return true;
	}
	//--------------------------------------------------------------------------------------


	/*    if(app_que_enable==1)
	     {
	             //--------save
	           // if(PositionSD_Status()&&(DEV_Login.Operate_enable==2)&&((enable==BD_EXT.Trans_GNSS_Flag)||(DispContent==6))||(Current_UDP_sd&&PositionSD_Status()&&(DEV_Login.Operate_enable==2))||((DF_LOCK==enable)&&PositionSD_Status()&&(DEV_Login.Operate_enable==2)))	  //首次定位再发
	            if(Current_UDP_sd&&PositionSD_Status())	  //首次定位再发
	   {
	                 //  clear  flag
	                 PositionSD_Disable();
	   Current_UDP_sd=0;

	   // stuff      content
	   memset(app_rx.body,0,sizeof(app_rx.body));
	   app_rx.wr=Reg_save_gps(app_rx.body);
	   //  send queue
	   if((AppQue.write_num*28)>APP_RAWINFO_SIZE )
	   {
	   rt_kprintf("\r\n exceed max appsize !!\r\n ");
	   return true;
	   }
	   memcpy(APP_serialinfo+AppQue.write_num*28,app_rx.body,app_rx.wr);
	                   //  rt_mq_send(&mq_APPs, (void*)&app_rx,  app_rx.wr+2);
	   AppQue.write_num++;
	   rt_kprintf("\r\n msg-save len=%d\r\n ", app_rx.wr);
	           }
	   return true;
	     }
	 */
	if( app_que_enable == 1 )
	{
		return true;
	}
	//-----------------------------------------------------------------------------------
	// if((Current_SD_Duration<=10)||(Current_State==1))   // 调试时30  实际是10 Current_SD_Duration
	// {
	if( stopNormal == 2 )
	{
		return true;
	}
	if( PositionSD_Status( ) && ( DEV_Login.Operate_enable == 2 )&&(VocREC.running==0) && ( ( enable == BD_EXT.Trans_GNSS_Flag ) || ( DispContent == 6 ) ) || ( Current_UDP_sd && PositionSD_Status( ) && ( DEV_Login.Operate_enable == 2 )&&(VocREC.running==0) ) || ( ( DF_LOCK == enable ) && PositionSD_Status( ) && ( DEV_Login.Operate_enable == 2 )&&(VocREC.running==0) ) ) //首次定位再发
	//  if((PositionSD_Status())&&(DataLink_Status())&&(DEV_Login.Operate_enable==2))	                                                                                                                     // DF  锁定发送当前位置信息
	{
		PositionSD_Disable( );
		Current_UDP_sd = 0;
		//1.   时间超前判断
		//if(Time_FastJudge()==false)
		//		return false;
		// 2.
		Stuff_Current_Data_0200H( ); // 上报即时数据
		//----应答次数 ----
		// ACKFromCenterCounter++; // 只关注应答报数，不关心应答时间
		//---------------------------------------------------------------------------------
		if( SleepState == 1 )
		{
		   if(DispContent==2)
			rt_kprintf( "\r\n休眠时发送时间 %d-%d-%d %02d:%02d:%02d\r\n", time_now.year + 2000, time_now.month, time_now.day, \
			            time_now.hour, time_now.min, time_now.sec );
		}
		if(DispContent==2)
		{
			rt_kprintf( "\r\n 发送 GPS -current !\r\n" );
		}

		return true;
	}
	//-------------------------------------------------------------
	return false;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void strtrim( u8 *s, u8 c )
{
	u8	*p1, *p2;
	u16 i, j;

	if( s == 0 )
	{
		return;
	}

	// delete the trailing characters
	if( *s == 0 )
	{
		return;
	}
	j	= strlen( (char const*)s );
	p1	= s + j;
	for( i = 0; i < j; i++ )
	{
		p1--;
		if( *p1 != c )
		{
			break;
		}
	}
	if( i < j )
	{
		p1++;
	}
	*p1 = 0; // null terminate the undesired trailing characters

	// delete the leading characters
	p1 = s;
	if( *p1 == 0 )
	{
		return;
	}
	for( i = 0; *p1++ == c; i++ )
	{
		;
	}
	if( i > 0 )
	{
		p2 = s;
		p1--;
		for(; *p1 != 0; )
		{
			*p2++ = *p1++;
		}
		*p2 = 0;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int str2ip( char *buf, u8 *ip )
{   // convert an ip:port string into a binary values
	int i;
	u16 _ip[4];

	memset( _ip, 0, sizeof( _ip ) );

	strtrim( (u8*)buf, ' ' );

	i = sscanf( buf, "%u.%u.%u.%u", (u32*)&_ip[0], (u32*)&_ip[1], (u32*)&_ip[2], (u32*)&_ip[3] );

	*(u8*)( ip + 0 )	= (u8)_ip[0];
	*(u8*)( ip + 1 )	= (u8)_ip[1];
	*(u8*)( ip + 2 )	= (u8)_ip[2];
	*(u8*)( ip + 3 )	= (u8)_ip[3];

	return i;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int IP_Str( char *buf, u32 IP )
{
	T_IP_Addr ip;

	if( !buf )
	{
		return 0;
	}

	ip.ip32 = IP;

	return sprintf( buf, "%u.%u.%u.%u", ip.ip8[0], ip.ip8[1], ip.ip8[2], ip.ip8[3] );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u16 AsciiToGb( u8 *dec, u8 InstrLen, u8 *scr )
{
	u16 i			= 0, j = 0, m = 0;
	u16 Info_len	= 0;

	for( i = 0, j = 0; i < InstrLen; i++, j++ )
	{
		m = scr[i];
		if( ( m >= 0x30 ) && ( m <= 0x39 ) )
		{
			memcpy( &dec[j], &arr_A3B0[( m - '0' ) * 2], 2 );
			j++;
		}else if( ( m >= 0x41 ) && ( m <= 0x4f ) )
		{
			memcpy( &dec[j], &arr_A3C0[( m - 0x41 + 1 ) * 2], 2 );
			j++;
		}else if( ( m >= 0x50 ) && ( m <= 0x5a ) )
		{
			memcpy( &dec[j], &arr_A3D0[( m - 0x50 ) * 2], 2 );
			j++;
		}else if( ( m >= 0x61 ) && ( m <= 0x6f ) )
		{
			memcpy( &dec[j], &arr_A3E0[( m - 0x61 + 1 ) * 2], 2 );
			j++;
		}else if( ( m >= 0x70 ) && ( m <= 0x7a ) )
		{
			memcpy( &dec[j], &arr_A3F0[( m - 0x70 ) * 2], 2 );
			j++;
		}else
		{
			dec[j] = m;
		}
	}
	Info_len = j;
	return Info_len;
}

// B.   Protocol

//==================================================================================================
// 第一部分 :   以下是GPS 解析转换相关函数
//==================================================================================================

void Time_pro( u8 *tmpinfo, u8 hour, u8 min, u8 sec )
{
	//---- record  to memory
	GPRMC.utc_hour	= hour;
	GPRMC.utc_min	= min;
	GPRMC.utc_sec	= sec;

	CurrentTime[0]	= hour;
	CurrentTime[1]	= min;
	CurrentTime[2]	= sec;

	//-----------  天地通协议 -------------
	Temp_Gps_Gprs.Time[0]	= hour;
	Temp_Gps_Gprs.Time[1]	= ( tmpinfo[2] - 0x30 ) * 10 + tmpinfo[3] - 0x30;
	Temp_Gps_Gprs.Time[2]	= ( tmpinfo[4] - 0x30 ) * 10 + tmpinfo[5] - 0x30;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Status_pro( u8 *tmpinfo )
{
	GPRMC.status = tmpinfo[0];

	//-------------------------天地通协议-----------------------------
	if( tmpinfo[0] == 'V' || tmpinfo[0] == 'v' )
	{
		UDP_dataPacket_flag = 0X03;
		StatusReg_GPS_V( );
	}else if( tmpinfo[0] == 'A' || tmpinfo[0] == 'a' )
	{
		UDP_dataPacket_flag = 0X02;
		StatusReg_GPS_A( );
	}

	//---------------------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Latitude_pro( u8 *tmpinfo )
{
	u32 latitude;
	GPRMC.latitude_value = atof( (char*)tmpinfo );


	/*     Latitude
	       ddmm.mmmm
	 */

	//--------	808 协议 --------------------
	if( UDP_dataPacket_flag == 0X02 ) //精确到百万分之一度
	{
		//------------  dd part   --------
		latitude = ( u32 )( ( tmpinfo[0] - 0x30 ) * 10 + ( u32 )( tmpinfo[1] - 0x30 ) ) * 1000000;
		//------------  mm  part  -----------


		/*    转换成百万分之一度
		      mm.mmmm   *  1000000/60=mm.mmmm*50000/3=mm.mmmm*10000*5/3
		 */
		latitude = latitude + ( u32 )( ( ( tmpinfo[2] - 0x30 ) * 100000 + ( tmpinfo[3] - 0x30 ) * 10000 + ( tmpinfo[5] - 0x30 ) * 1000 + ( tmpinfo[6] - 0x30 ) * 100 + ( tmpinfo[7] - 0x30 ) * 10 + ( tmpinfo[8] - 0x30 ) ) * 5 / 3 );

		if( latitude == 0 )
		{
			GPS_getfirst = 0;
			StatusReg_GPS_V( );
			return;
		}

		Temp_Gps_Gprs.Latitude[0]	= ( u8 )( latitude >> 24 );
		Temp_Gps_Gprs.Latitude[1]	= ( u8 )( latitude >> 16 );
		Temp_Gps_Gprs.Latitude[2]	= ( u8 )( latitude >> 8 );
		Temp_Gps_Gprs.Latitude[3]	= ( u8 )latitude;
	}
	//----------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Lat_NS_pro( u8 *tmpinfo )
{
	GPRMC.latitude = tmpinfo[0];
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Longitude_pro( u8 *tmpinfo )
{
	u32 longtitude;
	GPRMC.longtitude_value = atof( (char*)tmpinfo );


	/*     Latitude
	        dddmm.mmmm
	 */
	//--------  808协议  ---------
	if( UDP_dataPacket_flag == 0X02 ) //精确到百万分之一度
	{
		//------  ddd part -------------------
		longtitude = ( u32 )( ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 ) ) * 1000000;
		//------  mm.mmmm --------------------


		/*    转换成百万分之一度
		   mm.mmmm	 *	1000000/60=mm.mmmm*50000/3=mm.mmmm*10000*5/3
		 */
		longtitude = longtitude + ( u32 )( ( ( tmpinfo[3] - 0x30 ) * 100000 + ( tmpinfo[4] - 0x30 ) * 10000 + ( tmpinfo[6] - 0x30 ) * 1000 + ( tmpinfo[7] - 0x30 ) * 100 + ( tmpinfo[8] - 0x30 ) * 10 + ( tmpinfo[9] - 0x30 ) ) * 5 / 3 );
		if( longtitude == 0 )
		{
			GPS_getfirst = 0; StatusReg_GPS_V( ); return;
		}

		//---jiade --guojingdu yong ---
		if( app_que_enable == 1 )
		{
			longtitude = 116 * 1000000 - ( CSQ_counter % 8 ) + ( TIM1_Timer_Counter % 3 );
		}

		Temp_Gps_Gprs.Longitude[0]	= ( u8 )( longtitude >> 24 );
		Temp_Gps_Gprs.Longitude[1]	= ( u8 )( longtitude >> 16 );
		Temp_Gps_Gprs.Longitude[2]	= ( u8 )( longtitude >> 8 );
		Temp_Gps_Gprs.Longitude[3]	= ( u8 )longtitude;
	}

	//---------------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Long_WE_pro( u8 *tmpinfo )
{
	GPRMC.longtitude = tmpinfo[0];
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Speed_pro( u8 *tmpinfo, u8 Invalue, u8 Point )
{
	u32 sp	= 0, sp_DISP = 0;
	u32 reg = 0;

	//-------------------------------------------------------------------------------------------------------------
	if( Invalue == INIT )
	{
		return;
	}else //---------------------------------------------------------------------------------------------------------
	{
		GPRMC.speed = atof( (char*)tmpinfo );
		//---------------------------------------------------
		if( UDP_dataPacket_flag == 0x02 )
		{
			//-----808 协议 --------------
			//两个字节单位0.1 km/h
			if( Point == 1 )                                                    //0.0-9.9=>
			{
				//++++++++  Nathan Modify on 2008-12-1   ++++++++++
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[2] >= 0x30 ) && ( tmpinfo[2] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 );    //扩大10倍
				}else
				{
					return;
				}
			}else if( Point == 2 )                                              //10.0-99.9
			{
				//++++++++  Nathan Modify on 2008-12-1   ++++++++++
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[1] >= 0x30 ) && ( tmpinfo[1] <= 0x39 ) && ( tmpinfo[3] >= 0x30 ) && ( tmpinfo[3] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + tmpinfo[3] - 0x30;
				}else
				{
					return;
				}
			}else if( Point == 3 ) //100.0-999.9
			{
				//++++++++  Nathan Modify on 2008-12-1	++++++++++
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[1] >= 0x30 ) && ( tmpinfo[1] <= 0x39 ) && ( tmpinfo[2] >= 0x30 ) && ( tmpinfo[2] <= 0x39 ) && ( tmpinfo[4] >= 0x30 ) && ( tmpinfo[4] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 ) * 1000 + ( tmpinfo[1] - 0x30 ) * 100 + ( tmpinfo[2] - 0x30 ) * 10 + tmpinfo[4] - 0x30;
				}else
				{
					return;
				}
			}else
			{
				if( JT808Conf_struct.Speed_GetType == 0 )
				{
					GPS_speed = 0;
				} else
				{
					GPS_speed = Speed_cacu;
				}
			}

			// --------  sp 当前是0.1 knot------------------
			sp = (u32)( sp * 185.6 );                                   //  1 海里=1.856 千米  现在是m/h

			if( sp > 220000 )                                           //时速大于220km/h则剔除
			{
				return;
			}

			sp_DISP = sp / 100;                                         //  sp_Disp 单位是 0.1km/h

			//------------------------------ 通过GPS模块数据获取到的速度 --------------------------------
			Speed_gps = (u16)sp_DISP;
			//---------------------------------------------------------------------------
			if( JT808Conf_struct.Speed_GetType )                        // 通过速度传感器 获取速度
			{
				GPS_speed = Speed_cacu;
#if 0                                                                   //  北斗认证先不用自动校准
				K_AdjustUseGPS( sp, sp_DISP );                          //  调整K值
				if( JT808Conf_struct.DF_K_adjustState == 0 )
				{
					// ---  在未校准前，获得到的速度是通过GPS计算得到的
					GPS_speed = Speed_gps;
					//------- GPS	里程计算--------
					if( sp >= 5000 )                                    //	过滤零点漂移  速度大于
					{
						reg								= sp / 3600;    // 除以3600 是m/s
						JT808Conf_struct.Distance_m_u32 += reg;
						if( JT808Conf_struct.Distance_m_u32 > 0xFFFFFF )
						{
							JT808Conf_struct.Distance_m_u32 = 0;        //里程最长这么多米
						}
						//----- 定距回传处理---
						if( 1 == JT808Conf_struct.SD_MODE.DIST_TOTALMODE )
						{
							DistanceAccumulate += reg;
							if( DistanceAccumulate >= Current_SD_Distance )
							{
								DistanceAccumulate = 0;
								PositionSD_Enable( ); //发送
								Current_UDP_sd = 1;
							}
						}
						//------- 定距处理结束 -----
					}
				}
#endif
			}else
			{                                                       // 从GPS 取速度
				//------- GPS	里程计算--------
				if( sp >= 5000 )                                    //	过滤零点漂移  速度大于
				{
					JT808Conf_struct.Distance_m_u32 += sp / 3600;   // 除以3600 是m/s
					if( JT808Conf_struct.Distance_m_u32 > 0xFFFFFF )
					{
						JT808Conf_struct.Distance_m_u32 = 0;        //里程最长这么多米
					}
					//----- 定距回传处理---
					if( 1 == JT808Conf_struct.SD_MODE.DIST_TOTALMODE )
					{
						DistanceAccumulate += reg;
						if( DistanceAccumulate >= Current_SD_Distance )
						{
							DistanceAccumulate = 0;
							PositionSD_Enable( );   //发送
							Current_UDP_sd = 1;
						}
					}
					//------- 定距处理结束 -----
				}

				GPS_speed = Speed_gps;              // 用GPS数据计算得的速度 单位0.1km/h

				//-----------------------------------------------
			}
			// if(DispContent==2)
			//  rt_kprintf("\r\n				  速度: %d Km/h\r\n",GPS_speed/10);
		}
		else if( UDP_dataPacket_flag == 0x03 )
		{
			if( 0 == JT808Conf_struct.Speed_GetType )
			{
				//----- GPS 临时速度	km/h  ---------
				GPS_speed = 0;
			}
			if( JT808Conf_struct.Speed_GetType ) // 通过速度传感器 获取速度
			{
				//  K_AdjustUseGPS(sp,sp_DISP);  //  调整K值
				GPS_speed = Speed_cacu;
			}
			Speed_gps = 0;
		//	if( DispContent == 2 )
			//{
			//	rt_kprintf( "\r\n 2 GPS没定位\r\n" );
			//}
		}
	}
	//---------------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Direction_pro( u8 *tmpinfo, u8 Invalue, u8 Point )
{
	u32 sp = 0;
	//------------------------------------------------------------------------------------------------
	if( Invalue == INIT )
	{
		return;
	}else //-------------------------------------------------------------------------------------------
	{
		GPRMC.azimuth_angle = atof( (char*)tmpinfo );

		//--------------808 协议  1 度-------------------------
		if( UDP_dataPacket_flag == 0x02 )
		{
			if( Point == 1 ) //5.8
			{
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[2] >= 0x30 ) && ( tmpinfo[2] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 );
				} else
				{
					return;
				}
			}else if( Point == 2 ) // 14.7
			{
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[1] >= 0x30 ) && ( tmpinfo[1] <= 0x39 ) && ( tmpinfo[3] >= 0x30 ) && ( tmpinfo[3] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 ) * 10 + ( tmpinfo[1] - 0x30 );
				} else
				{
					return;
				}
			}else //357.38
			if( Point == 3 )
			{
				if( ( tmpinfo[0] >= 0x30 ) && ( tmpinfo[0] <= 0x39 ) && ( tmpinfo[1] >= 0x30 ) && ( tmpinfo[1] <= 0x39 ) && ( tmpinfo[2] >= 0x30 ) && ( tmpinfo[2] <= 0x39 ) && ( tmpinfo[4] >= 0x30 ) && ( tmpinfo[4] <= 0x39 ) )
				{
					sp = ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 );
				} else
				{
					return;
				}
			}else
			{
				sp = 0;
			}
			GPS_direction = sp; //  单位 1度

			//----------  拐点补传相关   ----------
			// Inflexion_Process();
		}else if( UDP_dataPacket_flag == 0x03 )
		{
			GPS_direction = 0;
		}

		return;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Date_pro( u8 *tmpinfo, u8 fDateModify, u8 hour, u8 min, u8 sec )
{
	uint8_t		year = 0, mon = 0, day = 0;
	TDateTime	now;

	day		= ( ( tmpinfo[0] - 0x30 ) * 10 ) + ( tmpinfo[1] - 0x30 );
	mon		= ( ( tmpinfo[2] - 0x30 ) * 10 ) + ( tmpinfo[3] - 0x30 );
	year	= ( ( tmpinfo[4] - 0x30 ) * 10 ) + ( tmpinfo[5] - 0x30 );

	if( fDateModify )
	{
		//sscanf(tmpinfo,"%2d%2d%2d",&day,&mon,&year);
		day++;
		if( mon == 2 )
		{
			if( ( year % 4 ) == 0 )
			{
				if( day == 30 )
				{
					day = 1; mon++;
				}
			}else if( day == 29 )
			{
				day = 1; mon++;
			}
		}else if( ( mon == 4 ) || ( mon == 6 ) || ( mon == 9 ) || ( mon == 11 ) )
		{
			if( day == 31 )
			{
				mon++; day = 1;
			}
		}else
		{
			if( day == 32 )
			{
				mon++; day = 1;
			}
			if( mon == 13 )
			{
				mon = 1; year++;
			}
		}
	}
	GPRMC.utc_year	= year;
	GPRMC.utc_mon	= mon;
	GPRMC.utc_day	= day;
	if( ( ( sec == 0 ) && ( GPRMC.status == 'A' ) ) || ( Shoushi == 1 ) )
	{
		now.year	= year;
		now.month	= mon;
		now.day		= day;
		now.hour	= hour;
		now.min		= min;
		now.sec		= sec;
		now.week	= 1;
		Device_RTC_set( now );
		Shoushi = 0;
	}
	//------------------------------------------------
	if( GPRMC.status == 'A' )                                               //  记录定位时间
	{
		Time2BCD( A_time );
		//------- Debug 存储 每秒的经纬度  || 实际应该是 存储每分钟的位置  -----
		//  内容持续55秒每秒更新，这寄存器中记录的是在每分钟内最后一包定位的经纬度 ,预留5秒用于存储上一小时的位置
		if( sec < 55 )
		{
			memcpy( Posit[min].latitude_BgEnd, Gps_Gprs.Latitude, 4 );      //北纬
			memcpy( Posit[min].longitude_BgEnd, Gps_Gprs.Longitude, 4 );    //经度
			Posit[min].longitude_BgEnd[0] |= 0x80;                          //  东经
		}
		if( ( min == 59 ) && ( sec == 55 ) )
		{                                                                   // 每个小时的位置信息
			NandsaveFlg.MintPosit_SaveFlag = 1;
		}
	}
	//---- 存储当前的起始里程  跨天时------------
	if( ( hour == 0 ) && ( min == 0 ) && ( sec < 3 ) )                      // 存储3次确保存储成功
	{
		JT808Conf_struct.DayStartDistance_32 = JT808Conf_struct.Distance_m_u32;
		Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
	}

	//-------------------------------------------------
	//---------  天地通协议  -------

	//if(systemTick_TriggerGPS==0)
	{
		Temp_Gps_Gprs.Date[0]	= year;
		Temp_Gps_Gprs.Date[1]	= mon;
		Temp_Gps_Gprs.Date[2]	= day;
	}

	//-------------- 计算算平均速度 ----------------
	AvrgSpd_MintProcess( hour, min, sec );
}

//---------  GGA --------------------------
void HDop_pro( u8 *tmpinfo )
{
	float dop;

	dop			= atof( (char*)tmpinfo );
	HDOP_value	= dop;          //  Hdop 数值
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  GPS_Delta_DurPro( void )  //告GPS 触发上报处理函数
{
	if( line_warn_enable == 1 )
	{
		rt_kprintf( "\r\n  %d-%d-%d %d:%d:%d %s", Temp_Gps_Gprs.Date[0], Temp_Gps_Gprs.Date[1], Temp_Gps_Gprs.Date[2], \
		            Temp_Gps_Gprs.Time[0], Temp_Gps_Gprs.Time[1], Temp_Gps_Gprs.Time[2], gps_log );
	}

	if( 1 == JT808Conf_struct.SD_MODE.DUR_TOTALMODE ) // 定时上报模式
	{
		//----- 上一包数据记录的时间
		fomer_time_seconds = ( u32 )( BakTime[0] * 60 * 60 ) + ( u32 )( BakTime[1] * 60 ) + ( u32 )BakTime[2];

		//-----  当前数据记录的时间
		tmp_time_secnonds = ( u32 )( CurrentTime[0] * 60 * 60 ) + ( u32 )( CurrentTime[1] * 60 ) + ( u32 )CurrentTime[2];

		//一天86400秒

		if( tmp_time_secnonds > fomer_time_seconds )
		{
			delta_time_seconds = tmp_time_secnonds - fomer_time_seconds;
			//systemTickGPS_Clear();
		}else if( tmp_time_secnonds < fomer_time_seconds )
		{
			delta_time_seconds = 86400 - fomer_time_seconds + tmp_time_secnonds;
			//systemTickGPS_Clear();
		}else
		{
			// systemTickGPS_Set();
			UDP_dataPacket_flag = 0X03;
			StatusReg_GPS_V( );
		}

		if( ( SleepState == 1 ) && ( delta_time_seconds == ( Current_SD_Duration - 5 ) ) )  //  --  休眠时 先发鉴权
		{
			SleepConfigFlag = 1;                                                            //发送前5 发送一包鉴权
		}

		if( ( delta_time_seconds >= Current_SD_Duration ) && ( local_trig == 0 ) )          //limitSend_idle
		{
			if( app_que_enable == 0 )                                                       // app 不是能上报
			{
				if( stop_current == 0 )
				{
					                                                  // 每次都是即时上报
	                 if(MQ_TrueUse.Enable_SD_state) 
	                 {
		                  if(MQ_TrueUse.Enable_SD_state==2)  // 存储时候存储。 上报时候不存储
		                  	{      
		                  	      //  upgrade    CurrentTotal  
		                  	       Get_MQ_true_CuurentTotal_packets(); 
								  //---- 盲区写入队列process--------
									MangQU_true_create(Temp_Gps_Gprs);
								    MQsend_counter++;
									//if(GpsStatus.Raw_Output==1) 
									rt_kprintf( "\r\n                 %d-%d-%d %d:%d:%d   sd=%d     Timeenable:%d  lati:%d  longi:%d\r\n",Temp_Gps_Gprs.Date[0], Temp_Gps_Gprs.Date[1], Temp_Gps_Gprs.Date[2], \
												Temp_Gps_Gprs.Time[0], Temp_Gps_Gprs.Time[1], Temp_Gps_Gprs.Time[2],MQsend_counter,EverySecond_Time_Get,Lati_Get,Longi_Get);  
		                  	}
	                 }
					 else
					 {
						 if(BDSD.Enable_Working==1)
						 	{   // 顺序存储判断补报
	                           BD_send_Mque_Tx(Temp_Gps_Gprs);
						 	} 
					    else
					 	 {
	                        PositionSD_Enable( );
						    Current_UDP_sd = 1;   
					 	 }
					}	
				}
			}
			memcpy( BakTime, CurrentTime, 3 );                                              // update

			if( stopNormal == 1 )
			{
				gps_sd_coutner++;
				rt_kprintf( "\r\n %d    %d-%d-%d %d:%d:%d %s", gps_sd_coutner, Temp_Gps_Gprs.Date[0], Temp_Gps_Gprs.Date[1], Temp_Gps_Gprs.Date[2], \
				            Temp_Gps_Gprs.Time[0], Temp_Gps_Gprs.Time[1], Temp_Gps_Gprs.Time[2], gps_log );
			}else
			{
				gps_sd_coutner = 0;
			}
			// rt_kprintf("\r\ntmp_time=%d,fomer_time=%d,delta_time=%d,Current=%d",tmp_time_secnonds,fomer_time_seconds,delta_time_seconds,Current_SD_Duration);
		}
	}

	//------------------------------ do this every  second-----------------------------------------
	memcpy( (char*)&Gps_Gprs, (char*)&Temp_Gps_Gprs, sizeof( Temp_Gps_Gprs ) );

	//------  电子围栏 判断  ----------


	/*  if((Temp_Gps_Gprs.Time[2]%20)==0) //   认证时不检测圆形电子围栏
	   {
	      CycleRail_Judge(Temp_Gps_Gprs.Latitude,Temp_Gps_Gprs.Longitude);
	   //rt_kprintf("\r\n --- 判断圆形电子围栏");
	   }	*/
	// if((Temp_Gps_Gprs.Time[2]==5)||(Temp_Gps_Gprs.Time[2]==25)||(Temp_Gps_Gprs.Time[2]==45)) //

	//  if(Temp_Gps_Gprs.Time[2]%2==0)//    认证时要求2 秒
	// {
	     RectangleRail_Judge( Temp_Gps_Gprs.Latitude, Temp_Gps_Gprs.Longitude );
	//rt_kprintf("\r\n -----判断矩形电子围栏");
	//    }

	if( line_warn_enable == 1 )
	{
		RouteLineWarn_judge( Temp_Gps_Gprs.Latitude, Temp_Gps_Gprs.Longitude );
	}

	/*
	     if((Temp_Gps_Gprs.Time[2]%3)==0) //     路线判断
	   {
	         // printf("\r\n --- 判断圆形电子围栏");
	          RouteRail_Judge(Temp_Gps_Gprs.Latitude,Temp_Gps_Gprs.Longitude);
	   }
	 */
	//rt_kprintf("\r\n Delta_seconds %d \r\n",delta_time_seconds);

	
    EverySecond_Time_Get=0; // clear
    Longi_Get=0;
	Lati_Get=0; 
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void local_time( void )
{
	if( local_trig == 1 )                                                           //  GPS 时间
	{
		local_timer++;

		if( ( SleepState == 1 ) && ( local_timer == ( Current_SD_Duration - 5 ) ) ) //  --  休眠时 先发鉴权
		{
			SleepConfigFlag = 1;                                                    //发送前5 发送一包鉴权
		}

		if( ( local_timer >= Current_SD_Duration ) && ( local_trig == 0 ) )         //limitSend_idle
		{
			PositionSD_Enable( );
			// if(SleepState==1)
			Current_UDP_sd	= 1;
			local_timer		= 0;                                                    // clear

			memcpy( BakTime, CurrentTime, 3 );                                      // update
			rt_kprintf( "\r\n-->local time trig\r\n" );
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  Exact_Test_time( void )
{
	if( local_trig == 2 )                                                                                                                                                                               //  GPS 时间
	{
		local_timer++;
		if( local_timer >= 9 )                                                                                                                                                                          //900ms
		{
			local_timer = 0;
			PositionSD_Enable( );
			// if(SleepState==1)
			Current_UDP_sd	= 1;
			local_timer		= 0;                                                                                                                                                                        // clear
			memcpy( BakTime, CurrentTime, 3 );                                                                                                                                                          // update
			rt_kprintf( "\r\n->DWJD-\r\n" );
		}
	}
}

//----------------------------------------------------------------------------------------------------
void  SpeedSensorProcess( void )                                                                                                                                                                        // 通过汽车的速度传感器获得 速度 并计算里程
{
	u32 Distance_1s_m = 0;                                                                                                                                                                              // 一秒钟运行 多少米
//  u32 sp=0;
	//  1. 用K值计算速度   -----------------------------------------------------------


	/*
	      K 表示 每公里 脉冲数
	      1米/脉冲数 :   K/1000
	      Delta_1s_Plus: 每秒钟采集到的脉冲数
	      每秒行驶米数:  Delta_1s_Plus *1000/K
	      => Delta_1s_Plus *1000/K*3.6*10 单位:0.1  km/h  =>Delta_1s_Plus *36000/K  单位:0.1 km/h
	 */
	Speed_cacu		= ( Delta_1s_Plus * 36000 ) / JT808Conf_struct.Vech_Character_Value;                                                                                                                // 通过计算得到的速度
	GPS_speed		= Speed_cacu;                                                                                                                                                                       //把计算得到的传感器速度给 协议 寄存器
	Distance_1s_m	= ( Delta_1s_Plus * 1000 ) / JT808Conf_struct.Vech_Character_Value;                                                                                                                 // 每秒运行多少米
	// 2. 计算里程相关  -------------------------------------------------------------
	//------------------------------------
	ModuleStatus |= Status_Pcheck;

	//------- GPS  里程计算  --------
	JT808Conf_struct.Distance_m_u32 += Distance_1s_m;                                                                                                                                                   // 除以3600 是m/s
	if( JT808Conf_struct.Distance_m_u32 > 0xFFFFFF )
	{
		JT808Conf_struct.Distance_m_u32 = 0;                                                                                                                                                            //里程最长这么多米
	}
	// ------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------------------------
void K_AdjustUseGPS( u32 sp, u32 sp_DISP )                                                                                                                                                              // 通过GPS 校准  K 值  (车辆行驶1KM 的脉冲数目)
{
	u32 Reg_distance	= 0;
	u32 Reg_plusNum		= 0;
	u16 i				= 0;

	if( JT808Conf_struct.DF_K_adjustState )                                                                                                                                                             // 只有没校准时才有效
	{
		return;
	}

	Speed_Rec = (u8)( sp_DISP / 10 );                                                                                                                                                                   // GPS速度    单位:km/h
	// -------	要求速度在60到65km/h  -------------
	if( ( ( Speed_Rec >= Speed_area ) && ( Speed_Rec <= ( Speed_area + 8 ) ) ) || ( ( Speed_Rec >= 40 ) && ( Speed_Rec <= ( 40 + 8 ) ) ) || ( ( Speed_Rec >= 70 ) && ( Speed_Rec <= ( 70 + 8 ) ) ) )    // Speed_area=60
	// if(Speed_Rec>=Speed_area)
//   if((Speed_Rec>=40)&&(Speed_Rec<=48))   // Speed_area=60
	{
		Spd_adjust_counter++;
		if( Spd_adjust_counter > K_adjust_Duration )                                                                                                                                                    //持续在速度在60~65下认为已经是匀速了
		{
			// 用获取到的匀速GPS速度作为基准，和根据传感器计算出来的速度，做K值得校准
			Reg_distance	= 0;                                                                                                                                                                        // clear
			Reg_plusNum		= 0;                                                                                                                                                                        // clear
			for( i = 0; i < K_adjust_Duration; i++ )
			{
				Reg_distance	+= Former_gpsSpd[i];                                                                                                                                                    // 除以3.6km/h 表示该秒内走了多少米
				Reg_plusNum		+= Former_DeltaPlus[i];
			}


			/*
			     做一个判断  ， 如果速度传感器不管用， 那么返回，
			 */
			if( Reg_plusNum < 20 )
			{
				Spd_adjust_counter = 0;
				rt_kprintf( "\r\n    速度传感器 没有脉冲!\r\n" );
				return;
			}
			//===================================================================
			// 转换成根据GPS速度计算行驶了多少米，(总距离) 先求和在除以3.6 ，为了计算方便 先x10  再除以36
			Reg_distance = (u32)( Reg_distance * 10 / 36 ); // 转换成根据GPS速度计算行驶了多少米，(总距离)
			// (Reg_plusNum/Reg_distance) 表示用脉冲总数除以距离(米)= 每米产生多少个脉冲 ，因为K值是1000米脉冲数，所以应该乘以1000
			JT808Conf_struct.Vech_Character_Value = 1000 * Reg_plusNum / Reg_distance;
			//-------  存储新的特征系数 --------------------------------
			JT808Conf_struct.DF_K_adjustState	= 1;        // clear  Flag
			ModuleStatus						|= Status_Pcheck;
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );

			Spd_adjust_counter = 0;                         // clear  counter
		}else
		{                                                   //-------- 记录规定时间内的脉冲数和GPS速度----------
			Former_gpsSpd[Spd_adjust_counter]		= Speed_Rec;
			Former_DeltaPlus[Spd_adjust_counter]	= Delta_1s_Plus;
		}
	}else
	{
		Spd_adjust_counter = 0;                             // 只要速度超出预设范围 计数器清0
	}
}

//==================================================================================================
// 第二部分 :   以下是外部传感器状态监测
//==================================================================================================


/*
     -----------------------------
     2.1   和协议相关的功能函数
     -----------------------------
 */

void IMSI_Convert_SIMCODE( void )
{
	SIM_code[0] = IMSI_CODE[3] - 0X30;
	SIM_code[0] <<= 4;
	SIM_code[0] |= IMSI_CODE[4] - 0X30;

	SIM_code[1] = IMSI_CODE[5] - 0X30;
	SIM_code[1] <<= 4;
	SIM_code[1] |= IMSI_CODE[6] - 0X30;

	SIM_code[2] = IMSI_CODE[7] - 0X30;
	SIM_code[2] <<= 4;
	SIM_code[2] |= IMSI_CODE[8] - 0X30;

	SIM_code[3] = IMSI_CODE[9] - 0X30;
	SIM_code[3] <<= 4;
	SIM_code[3] |= IMSI_CODE[10] - 0X30;

	SIM_code[4] = IMSI_CODE[11] - 0X30;
	SIM_code[4] <<= 4;
	SIM_code[4] |= IMSI_CODE[12] - 0X30;

	SIM_code[5] = IMSI_CODE[13] - 0X30;
	SIM_code[5] <<= 4;
	SIM_code[5] |= IMSI_CODE[14] - 0X30;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void GNSS_RawDataTrans_Init( u8 mode )  // 北斗添加
{
	if( mode )
	{
		GNSS_rawdata.WorkEnable = 1;    //初始化使能
	}else
	{
		GNSS_rawdata.WorkEnable = 0;    //初始化不使能
	}
	GNSS_rawdata.first_record	= 1;
	GNSS_rawdata.rd_num			= 0;
	GNSS_rawdata.wr_num			= 0;
	GNSS_rawdata.save_status	= 0;
	GNSS_rawdata.Raw_wr			= 0;
}

/*
     -----------------------------
    2.4  不同协议状态寄存器变化
     -----------------------------
 */

void StatusReg_WARN_Enable( void )
{
	//     紧急报警状态下 寄存器的变化
	Warn_Status[3] |= 0x01;     //BIT( 0 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_WARN_Clear( void )
{
	//     清除报警寄存器
	Warn_Status[3] &= ~0x01;    //BIT( 0 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_ACC_ON( void )
{                               //    ACC 开
	Car_Status[3] |= 0x01;      //  Bit(0)     Set  1  表示 ACC开
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_ACC_OFF( void )
{                               //    ACC 关
	Car_Status[3] &= ~0x01;     //  Bit(0)     Set  01  表示 ACC关
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_POWER_CUT( void )
{                               //  主电源断开
	Warn_Status[2]	|= 0x01;    //BIT( 0 );
	ModuleStatus	|= Status_Battery;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_POWER_NORMAL( void )
{                               // 主电源正常
	Warn_Status[2]	&= ~0x01;   //BIT( 0 );
	ModuleStatus	&= ~Status_Battery;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_GPS_A( void )
{                               // GPS 定位
	if( GPS_getfirst == 0 )
	{
#ifdef LCD_5inch
		DwinLCD.Type = LCD_SETTIME;
#endif
		Shoushi = 1;
		if(DispContent==2)
		rt_kprintf( "\r\n firs A  Set RTC\r\n" );
	}
	GPS_getfirst	= 1;
	Car_Status[3]	|= 0x02;    //Bit(1)
	ModuleStatus	|= Status_GPS;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_GPS_V( void )
{                               //  GPS 不定位
	Car_Status[3]	&= ~0x02;   //Bit(1)
	ModuleStatus	&= ~Status_GPS;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_SPD_WARN( void )
{                               //  超速报警
	Warn_Status[3] |= 0x02;     //BIT( 1 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_SPD_NORMAL( void )
{                               //  速度正常
	Warn_Status[3] &= ~0x02;    //BIT( 1 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_Relay_Cut( void )
{                               // 断油断电状态
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_Relay_Normal( void )
{                               //  断油电状态正常
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void StatusReg_Default( void )
{                               //   状态寄存器还原默认设置
	Warn_Status[0]	= 0x00;     //HH
	Warn_Status[1]	= 0x00;     //HL
	Warn_Status[2]	= 0x00;     //LH
	Warn_Status[3]	= 0x00;     //LL
}

//==================================================================================================
// 第三部分 :   以下是GPRS无线传输相关协议
//==================================================================================================
void BD_send_Init(void)
{
  BDSD.Enable_Working=0;
  BDSD.read=0;
  BDSD.SendFlag=0; 
  BDSD.wait__resentTimer=0;
  BDSD.write=0;

}

void BD_send_Mque_Tx(T_GPS_Info_GPRS Gps_Gprs)
{
  
  u16 counter_mainguffer, i;
  u32 Dis_01km=0;
   
  
	  //----------------------- Save GPS --------------------------------------
	  memset( (u8*)BDsd_tx.body, 0, sizeof(BDsd_tx.body) );
	  BDsd_tx.wr= 0;
	  BDsd_tx.wr++;// 长度
	  //------------------------------- Stuff ----------------------------------------
	  counter_mainguffer = BDsd_tx.wr;
	  // 1. 告警状态   4 Bytes
	  memcpy( ( char* ) BDsd_tx.body+ BDsd_tx.wr, ( char* )Warn_Status, 4 );
	  BDsd_tx.wr += 4;
	  // 2. 车辆状态   4 Bytes
	  memcpy( ( char* )BDsd_tx.body + BDsd_tx.wr, ( char* )Car_Status, 4 );
	  BDsd_tx.wr += 4;
	  // 3.   纬度	   4 Bytes
	  memcpy( ( char* )BDsd_tx.body + BDsd_tx.wr, ( char* )Gps_Gprs.Latitude, 4 );   //纬度   modify by nathan
	  BDsd_tx.wr += 4;
	  // 4.   经度	   4 Bytes
	  memcpy( ( char* )BDsd_tx.body + BDsd_tx.wr, ( char* )Gps_Gprs.Longitude, 4 );  //经度	东经  Bit 7->0	  西经 Bit 7 -> 1
	  BDsd_tx.wr += 4;
	  // 5.  高度	2 Bytes    m
	  BDsd_tx.body[BDsd_tx.wr++] = ( GPS_Hight >> 8 ); 							  // High
	  BDsd_tx.body[BDsd_tx.wr++] = (u8)GPS_Hight;									  // Low
	  // 6.  速度	2 Bytes 	0.1Km/h
	  BDsd_tx.body[BDsd_tx.wr++] = ( Speed_gps >> 8 ); 							  // High
	  BDsd_tx.body[BDsd_tx.wr++] = (u8)Speed_gps;									  // Low
	  // 7.  方向	2 Bytes 	  1度
	  BDsd_tx.body[BDsd_tx.wr++] = ( GPS_direction >> 8 ); 						  //High
	  BDsd_tx.body[BDsd_tx.wr++] = GPS_direction;									  // Low
	  // 8.  日期时间	6 Bytes
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	  BDsd_tx.body[BDsd_tx.wr++] = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );





   #if 1
	  //--------------------------------------------------------------	  
	  //----------- 附加信息  ------------
	  //  附加信息 1  -----------------------------
	  //  附加信息 ID
	  BDsd_tx.body[BDsd_tx.wr++] = 0x03; // 行驶记录仪的速度
	  //  附加信息长度
	  BDsd_tx.body[BDsd_tx.wr++] = 2;
	  //  类型
	  BDsd_tx.body[BDsd_tx.wr++]   = (u8)( Speed_cacu >> 8 );
	  BDsd_tx.body[BDsd_tx.wr++]   = (u8)( Speed_cacu );
	  //rt_kprintf("\r\n GPS速度=%d km/h , 传感器速度=%d km/h\r\n",Speed_gps,Speed_cacu);
	  //  附加信息 2  -----------------------------
	  //  附加信息 ID
	  BDsd_tx.body[BDsd_tx.wr++] = 0x01; // 车上的行驶里程
	  //  附加信息长度
	  BDsd_tx.body[BDsd_tx.wr++] = 4;
	  //  类型
	  Dis_01km							  = JT808Conf_struct.Distance_m_u32 / 100;
	  BDsd_tx.body[BDsd_tx.wr++]   = ( Dis_01km >> 24 );
	  BDsd_tx.body[BDsd_tx.wr++]   = ( Dis_01km >> 16 );
	  BDsd_tx.body[BDsd_tx.wr++]   = ( Dis_01km >> 8 );
	  BDsd_tx.body[BDsd_tx.wr++]   = Dis_01km;
	  
	  //  附加信息 3
	  if( Warn_Status[1] & 0x10 )
	  {
		  //  附加信息 ID
		  BDsd_tx.body[BDsd_tx.wr++] = 0x12; //	进出区域/路线报警
		  //  附加信息长度
		  BDsd_tx.body[BDsd_tx.wr++] = 6;
		  //  类型
		  BDsd_tx.body[BDsd_tx.wr++]   = InOut_Object.TYPE;
		  BDsd_tx.body[BDsd_tx.wr++]   = ( InOut_Object.ID >> 24 );
		  BDsd_tx.body[BDsd_tx.wr++]   = ( InOut_Object.ID >> 16 );
		  BDsd_tx.body[BDsd_tx.wr++]   = ( InOut_Object.ID >> 8 );
		  BDsd_tx.body[BDsd_tx.wr++]   = InOut_Object.ID;
		  BDsd_tx.body[BDsd_tx.wr++]   = InOut_Object.InOutState;
		  rt_kprintf( "\r\n ----- 0x0200 current 附加信息 \r\n" );
	  }
	  
	  //  附件信息4
	  if( Warn_Status[3] & 0x02 )
	  {
		  //  附加信息 ID
		  BDsd_tx.body[BDsd_tx.wr++] = 0x11; 				  //  进出区域/路线报警
		  //  附加信息长度
		  BDsd_tx.body[BDsd_tx.wr++] = 1;
		  //  类型
		  BDsd_tx.body[BDsd_tx.wr++] = 0;					  //  无特定位置
	  }
	  // 5. 附加信息
	  BDsd_tx.body[BDsd_tx.wr++]   = 0x25;					  //ID	扩展车辆信号状态位
	  BDsd_tx.body[BDsd_tx.wr++]   = 4;						  //LEN
	  BDsd_tx.body[BDsd_tx.wr++]   = 0x00;
	  BDsd_tx.body[BDsd_tx.wr++]   = 0x00;
	  BDsd_tx.body[BDsd_tx.wr++]   = 0x00;
	  BDsd_tx.body[BDsd_tx.wr++]   = BD_EXT.Extent_IO_status;
	  //6. 附加信息  :
	  if( SleepState == 1 ) 										  //休眠
	  {
		  BDsd_tx.body[BDsd_tx.wr++]   = 0x2A;				  //ID	 IO状态位【1】	 bit0:深度休眠	 bit1:休眠【0】保留
		  BDsd_tx.body[BDsd_tx.wr++]   = 2;					  //LEN
		  BDsd_tx.body[BDsd_tx.wr++]   = 0x02;
		  BDsd_tx.body[BDsd_tx.wr++]   = 0;
	  }
	  //  附加信息 7  -----------------------------
	  //  附加信息 ID
	  BDsd_tx.body[BDsd_tx.wr++] = 0x2B; 					  //模拟量
	  //  附加信息长度
	  BDsd_tx.body[BDsd_tx.wr++] = 4;
	  //  类型
	  BDsd_tx.body[BDsd_tx.wr++]   = ( AD_2through[1] >> 8 );  // AD1
	  BDsd_tx.body[BDsd_tx.wr++]   = AD_2through[1];
	  BDsd_tx.body[BDsd_tx.wr++]   = ( AD_2through[0] >> 8 );  // AD0
	  BDsd_tx.body[BDsd_tx.wr++]   = AD_2through[0];
	  //  附加信息 8  -----------------------------
	  //  附加信息 ID
	  BDsd_tx.body[BDsd_tx.wr++] = 0x30; 					  //无线通信网络信号强度
	  //  附加信息长度
	  BDsd_tx.body[BDsd_tx.wr++] = 1;
	  //  类型
	  BDsd_tx.body[BDsd_tx.wr++] = ModuleSQ;
	  
	  //  附加信号 9  ---------------------------------
	  BDsd_tx.body[BDsd_tx.wr++]   = 0x31;					  // GNSS 定位卫星颗数
	  BDsd_tx.body[BDsd_tx.wr++]   = 1;						  // len
	  BDsd_tx.body[BDsd_tx.wr++]   = Satelite_num;
	#endif  

		BDsd_tx.body[0]=BDsd_tx.wr;  

		//-------------  Caculate  FCS  -----------------------------------
		FCS_GPS_UDP=0;  
		for ( i = 0; i < BDsd_tx.wr; i++ )  
		{
				FCS_GPS_UDP ^= *( BDsd_tx.body + i ); 
		}			   //求上边数据的异或和
		BDsd_tx.body[BDsd_tx.wr++] = FCS_GPS_UDP;  

	  //-------------------------------- stuff msg_queue------------------------------------------
	  rt_mq_send( &mq_BDsd, (void*)&BDsd_tx, BDsd_tx.wr+2); 
  
	  //OutPrint_HEX("BD-send",BDsd_tx.body,BDsd_tx.wr);

}

u8 BD_send_Mque_Rx(void)
{
    rt_err_t	res;

    memset(BDsd_rx.body,0,sizeof(BDsd_rx.body));
   	res = rt_mq_recv( &mq_BDsd, (void*)&BDsd_rx, 128,5 ); //等待100ms,实际上就是变长的延时,最长100ms
	if( res == RT_EOK )                                                     //收到一包数据
	{

               WatchDog_Feed();
       if( BDSD_SaveCycleGPS(BDSD.write,BDsd_rx.body,BDsd_rx.wr))
	     { //---- updata pointer   -------------		
			BDSD.write++;  	
		       if(BDSD.write>=Max_BDSD_Num)
		  	               BDSD.write=0;  
			DF_Write_RecordAdd(BDSD.write,BDSD.write,TYPE_BDsdAdd);   
			DF_delay_ms(20);  
	      //-------------------------------	
	        if(DispContent) 	
					       rt_kprintf("\r\n    BDsq succed\r\n");
            }  
	    else
	    	{
	    	  if(DispContent) 	
					       rt_kprintf("\r\n    BDsq fail\r\n");

	    	}
           //  OutPrint_HEX("BD-RX",BDsd_rx.body,BDsd_rx.wr);  
             return true;
	}

}


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void MangQU_true_create(T_GPS_Info_GPRS Gps_Gprs)
{
	
	u16 counter_mainguffer, i;


		//----------------------- Save GPS --------------------------------------
		memset( GPSsaveBuf, 0, 40 );
		GPSsaveBuf_Wr = 0;
		//------------------------------- Stuff ----------------------------------------
		counter_mainguffer = GPSsaveBuf_Wr;
		// 1. 告警状态   4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Warn_Status, 4 );
		GPSsaveBuf_Wr += 4;
		// 2. 车辆状态   4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Car_Status, 4 );
		GPSsaveBuf_Wr += 4;
		// 3.   纬度     4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Gps_Gprs.Latitude, 4 );   //纬度   modify by nathan
		GPSsaveBuf_Wr += 4;
		// 4.   经度     4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Gps_Gprs.Longitude, 4 );  //经度    东经  Bit 7->0	西经 Bit 7 -> 1
		GPSsaveBuf_Wr += 4;
		// 5.  高度	  2 Bytes    m
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( GPS_Hight >> 8 );                               // High
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)GPS_Hight;                                    // Low
		// 6.  速度	  2 Bytes     0.1Km/h
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( Speed_gps >> 8 );                               // High
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)Speed_gps;                                    // Low
		// 7.  方向	  2 Bytes	    1度
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( GPS_direction >> 8 );                           //High
		GPSsaveBuf[GPSsaveBuf_Wr++] = GPS_direction;                                    // Low
		// 8.  日期时间	  6 Bytes
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );
		//-------------  Caculate  FCS  -----------------------------------
		FCS_GPS_UDP = 0;
		for( i = counter_mainguffer; i < 30; i++ )
		{
			FCS_GPS_UDP ^= *( GPSsaveBuf + i );
		}                                                           //求上边数据的异或和
		GPSsaveBuf[30] = FCS_GPS_UDP;

		
		//-------------------------------- stuff msg_queue------------------------------------------
		rt_mq_send( &mq_MQBuBao, (void*)&GPSsaveBuf, 32); 

		//OutPrint_HEX("MQ-Genernate",GPSsaveBuf,32);
		
}

void MangQU_true_Save(void)
{
    rt_err_t	res;
	u8  Rx_MQ[40];

    memset(Rx_MQ,0,40);
   	res = rt_mq_recv( &mq_MQBuBao, (void*)&Rx_MQ, 32,5 ); //等待100ms,实际上就是变长的延时,最长100ms
	if( res == RT_EOK )                                                     //收到一包数据
	{
       	if( Api_cycle_write( GPSsaveBuf, 31 ) ) 
		{
			if( DispContent )
			{
				rt_kprintf( "\r\n    MQ Save succed wr=%d\r\n",cycle_write ); 
			}
		}else
		{
			WatchDog_Feed( );
			if( DispContent )
			{
				rt_kprintf( "\r\n MQ save fail\r\n" );
			}

			if( Api_cycle_write( GPSsaveBuf, 31 ) )
			{
				rt_kprintf( "\r\n MQ save retry ok\r\n" ); 
			}
		}
      // OutPrint_HEX("MQ-save",Rx_MQ,32);  
	}

}


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void MangQU_read( u32 packetnum, u8 *str, u32 length )
{
    u32  i=0,Size_pkt=0, pagein_offset=0;
    u8  latiBuf[8*MQ_PKNUM];  // 16*8
    u8   regstr[4];	
	u8   reg8str[8];
	u8	lati[4], longi[4];
	u32 lati_stuff	= 0, longi_stuff = 0;
	u16 spd_stuff	= 0;
	u8	time_stuff[3];
	u32 timervalue = 0;

	   Size_pkt=8*MQ_PKNUM;	
	memset( latiBuf, 0, sizeof( latiBuf ) );

	for( i = 0; i < Size_pkt; i++ )
	{
		latiBuf[i] = SST25V_ByteRead( ( (u32)( DF_MQ_Page ) * 512 ) + (u32)( packetnum * Size_pkt + i ) ); //512bytes 一个单位
	}
		//OutPrint_HEX("lati",latiBuf,175);
        // value start      18:13:14   18*3600+13*60+14=65594       
		timervalue=65594+packetnum*MQ_PKNUM; 
		for(i=0;i<MQ_PKNUM;i++)
		{
		     //--------- convert latitude ---------------------
			 memcpy(reg8str,latiBuf+8*i,8);
		//OutPrint_HEX("ascii",reg8str,8);
		//------------------------------------
		regstr[0]	= ( HexValue( reg8str[0] ) << 4 ) + HexValue( reg8str[1] );
		regstr[1]	= ( HexValue( reg8str[2] ) << 4 ) + HexValue( reg8str[3] );
		regstr[2]	= ( HexValue( reg8str[4] ) << 4 ) + HexValue( reg8str[5] );
		regstr[3]	= ( HexValue( reg8str[6] ) << 4 ) + HexValue( reg8str[7] );
		//------------------------------------
		// OutPrint_HEX("hex",regstr,4);

		//----------  信息长度 --------------------------
		Original_info[Original_info_Wr++]	= 0;
		Original_info[Original_info_Wr++]	= 32;
		//----------------0200 -----------------------
		// 1. 告警标志  4
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
		Original_info_Wr += 4;
		// 2. 状态  4
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
		Original_info_Wr += 4;
		// 3.  纬度
		lati_stuff	= ( regstr[0] << 24 ) + ( regstr[1] << 16 ) + ( regstr[2] << 8 ) + ( regstr[3] ) - ( CSQ_counter % 25 ) + ( TIM1_Timer_Counter % 17 );  // need change	a  little
		lati[0]		= ( u8 )( lati_stuff >> 24 );
		lati[1]		= ( u8 )( lati_stuff >> 16 );
		lati[2]		= ( u8 )( lati_stuff >> 8 );
		lati[3]		= ( u8 )lati_stuff;
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )lati, 4 );                                                                              //纬度	 modify by nathan
		Original_info_Wr += 4;
		// 4.  经度

		longi_stuff = 116 * 1000000 - ( CSQ_counter % 12 ) + ( TIM1_Timer_Counter % 3 );
		longi[0]	= ( u8 )( longi_stuff >> 24 );
		longi[1]	= ( u8 )( longi_stuff >> 16 );
		longi[2]	= ( u8 )( longi_stuff >> 8 );
		longi[3]	= ( u8 )longi_stuff;
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )longi, 4 );                             //经度	  东经	Bit 7->0   西经 Bit 7 -> 1
		Original_info_Wr += 4;
		// 5.  高程
		GPS_Hight							= 50 + ( CSQ_counter % 8 ) - ( TIM1_Timer_Counter % 6 );
		Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
		Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
		// 6.  速度	 0.1 Km/h
		spd_stuff							= 1080 - ( CSQ_counter % 10 ) + ( TIM1_Timer_Counter % 12 );    // need change	a  little  76
		Original_info[Original_info_Wr++]	= (u8)( spd_stuff >> 8 );                                       //(GPS_speed>>8);
		Original_info[Original_info_Wr++]	= (u8)( spd_stuff );                                            //GPS_speed;
		// 7. 方向   单位 1度
		Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );                                       //High
		Original_info[Original_info_Wr++]	= GPS_direction;                                                // Low
		// 8.  日期时间
		Original_info[Original_info_Wr++]	= 0x13;                                                         //(((Gps_Gprs.Date[0])/10)<<4)+((Gps_Gprs.Date[0])%10);
		Original_info[Original_info_Wr++]	= 0x03;                                                         //((Gps_Gprs.Date[1]/10)<<4)+(Gps_Gprs.Date[1]%10);
		Original_info[Original_info_Wr++]	= 0x05;                                                         // ((Gps_Gprs.Date[2]/10)<<4)+(Gps_Gprs.Date[2]%10);

			  timervalue=65594+packetnum*MQ_PKNUM+i;
		time_stuff[0]	= timervalue / 3600;
		time_stuff[1]	= ( timervalue % 3600 ) / 60;
		time_stuff[2]	= ( timervalue % 60 );
		// rt_kprintf("\r\n %d stufftime %d:%d:%d ",i,time_stuff[0],time_stuff[1],time_stuff[2]);

		Original_info[Original_info_Wr++]	= ( ( time_stuff[0] / 10 ) << 4 ) + ( time_stuff[0] % 10 ); //((Gps_Gprs.Time[0]/10)<<4)+(Gps_Gprs.Time[0]%10);
		Original_info[Original_info_Wr++]	= ( ( time_stuff[1] / 10 ) << 4 ) + ( time_stuff[1] % 10 );
		Original_info[Original_info_Wr++]	= ( ( time_stuff[2] / 10 ) << 4 ) + ( time_stuff[2] % 10 );

		//----------- 附加信息  ------------
		//  附加信息 1  -----------------------------
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x03;                                                       // 行驶记录仪的速度
		//  附加信息长度
		Original_info[Original_info_Wr++] = 2;
		//  类型
		Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
		Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void latrd( u8* instr )
{
	u32 i = 0, Size_pkt = 0, pagein_offset = 0;
	u8	latiBuf[7 * MQ_PKNUM]; // 15*7
	u8	regstr[4];
	u8	reg8str[8];
	u16 packetnum = 0;

	Size_pkt = 7 * MQ_PKNUM;

	sscanf( instr, "%d", (u32*)&packetnum );
	rt_kprintf( "\r\n inputpacket %d \r\n", packetnum );

	memset( latiBuf, 0, sizeof( latiBuf ) );

	for( i = 0; i < Size_pkt; i++ )
	{
		latiBuf[i] = SST25V_ByteRead( ( (u32)( DF_Record_Page + 1 ) * 512 ) + (u32)( packetnum * Size_pkt + i ) ); //512bytes 一个单位
	}
	OutPrint_HEX( "lati", latiBuf, Size_pkt );

	for( i = 0; i < MQ_PKNUM; i++ )
	{
		reg8str[0] = '0';
		memcpy( reg8str + 1, latiBuf + 7 * i, 7 );
		OutPrint_HEX( "ascii", reg8str, 8 );
		//------------------------------------
		regstr[0]	= ( HexValue( reg8str[0] ) << 4 ) + HexValue( reg8str[1] );
		regstr[1]	= ( HexValue( reg8str[2] ) << 4 ) + HexValue( reg8str[3] );
		regstr[2]	= ( HexValue( reg8str[4] ) << 4 ) + HexValue( reg8str[5] );
		regstr[3]	= ( HexValue( reg8str[6] ) << 4 ) + HexValue( reg8str[7] );
		//------------------------------------
		OutPrint_HEX( "hex", regstr, 4 );
	}
}

FINSH_FUNCTION_EXPORT( latrd, latrd );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  MangquSave_GPS( void )
{
	u16 counter_mainguffer, i;
	u8	readbuf[MANGQsize]; //,regstatus;

	if( Current_UDP_sd && PositionSD_Status( ) )
	{
		PositionSD_Disable( );
		Current_UDP_sd = 0;
		//-------------------------------------------------------
		//1.   时间超前判断
		//  if(Time_FastJudge()==false)
		//		return ;
		//----------------------- Save GPS --------------------------------------
		memset( GPSsaveBuf, 0, 40 );
		GPSsaveBuf_Wr = 0;
		//------------------------------- Stuff ----------------------------------------
		counter_mainguffer = GPSsaveBuf_Wr;
		// 1. 告警状态   4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Warn_Status, 4 );
		GPSsaveBuf_Wr += 4;
		// 2. 车辆状态   4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Car_Status, 4 );
		GPSsaveBuf_Wr += 4;
		// 3.   纬度     4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Gps_Gprs.Latitude, 4 );   //纬度   modify by nathan
		GPSsaveBuf_Wr += 4;
		// 4.   经度     4 Bytes
		memcpy( ( char* )GPSsaveBuf + GPSsaveBuf_Wr, ( char* )Gps_Gprs.Longitude, 4 );  //经度    东经  Bit 7->0	西经 Bit 7 -> 1
		GPSsaveBuf_Wr += 4;
		// 5.  高度	  2 Bytes    m
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( GPS_Hight >> 8 );                               // High
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)GPS_Hight;                                    // Low
		// 6.  速度	  2 Bytes     0.1Km/h
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( Speed_gps >> 8 );                               // High
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)Speed_gps;                                    // Low
		// 7.  方向	  2 Bytes	    1度
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( GPS_direction >> 8 );                           //High
		GPSsaveBuf[GPSsaveBuf_Wr++] = GPS_direction;                                    // Low
		// 8.  日期时间	  6 Bytes
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );
		//----------- 附加信息  ------------
		//  附加信息 1  -----------------------------
		//	附加信息 ID
		GPSsaveBuf[GPSsaveBuf_Wr++] = 0x03; // 行驶记录仪的速度
		//	附加信息长度
		GPSsaveBuf[GPSsaveBuf_Wr++] = 2;
		//	类型
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)( Speed_cacu >> 8 );
		GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)( Speed_cacu );

		//	 memcpy(MangQU.Buffer+MangQU.buf_wr,GPSsaveBuf,GPSsaveBuf_Wr);
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Reg_save_gps( u8 *instr )
{
	u8 wr = 0;
	// 1. 告警状态   4 Bytes
	memcpy( ( char* )instr + wr, ( char* )Warn_Status, 4 );
	wr += 4;
	// 2. 车辆状态   4 Bytes
	memcpy( ( char* )instr + wr, ( char* )Car_Status, 4 );
	wr += 4;
	// 3.   纬度     4 Bytes
	memcpy( ( char* )instr + wr, ( char* )Gps_Gprs.Latitude, 4 );   //纬度   modify by nathan
	wr += 4;
	// 4.   经度     4 Bytes
	memcpy( ( char* )instr + wr, ( char* )Gps_Gprs.Longitude, 4 );  //经度    东经  Bit 7->0	西经 Bit 7 -> 1
	wr += 4;
	// 5.  高度	  2 Bytes    m
	instr[wr++] = ( GPS_Hight >> 8 );                               // High
	instr[wr++] = (u8)GPS_Hight;                                    // Low
	// 6.  速度	  2 Bytes     0.1Km/h
	instr[wr++] = ( Speed_gps >> 8 );                               // High
	instr[wr++] = (u8)Speed_gps;                                    // Low
	// 7.  方向	  2 Bytes	    1度
	instr[wr++] = ( GPS_direction >> 8 );                           //High
	instr[wr++] = GPS_direction;                                    // Low
	// 8.  日期时间	  6 Bytes
	instr[wr++] = ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	instr[wr++] = ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	instr[wr++] = ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	instr[wr++] = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	instr[wr++] = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	instr[wr++] = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

	return wr;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  App_mq_Read_Process( void )
{
	char		ch;
//-----------------------------------------------------
	rt_err_t	res;

	if( AppQue.sd_enable_flag == 1 )
	{
		Stuff_Normal_Data_0200H( AppQue.read_num );
		AppQue.sd_enable_flag = 2;
		rt_kprintf( "\r\n DWJD     %d  send\r\n", AppQue.read_num );
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void App_mq_NoAck_counter( void )           // Que  方式下 没有新的应答超时判断
{
	if( ( AppQue.sd_enable_flag == 2 ) && ( app_que_enable == 1 ) )
	{
		AppQue.abnormal_counter++;
		if( AppQue.abnormal_counter >= 4 )  // 2s
		{
			AppQue.abnormal_counter = 0;
			AppQue.sd_enable_flag	= 1;
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void App_mq_SendTimer( void )
{
	if( ( app_que_enable == 1 ) && ( AppQue.sd_enable_flag == 0 ) )
	{
		AppQue.send_timer++;
		if( AppQue.send_timer >= 10 ) // 100ms
		{
			AppQue.send_timer		= 0;
			AppQue.abnormal_counter = 0;
			AppQue.sd_enable_flag	= 1;
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  App_mq_Resend( void )           // 启动Queue 重发
{
	if( AppQue.re_send_flag == 1 )
	{
		rt_kprintf( "\r\n + msg_resend ->app msg  wr=%d  rd=%d \r\n", AppQue.write_num, AppQue.read_num );
		Stuff_AppMq_Data_0200H( Rx_reg.body );
		AppQue.re_send_flag = 2;    // enable   wait for  Ack
		return true;
	}else
	{
		return false;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void MangQu_Timer( void )
{
	//------------------------------------------------------------
	if( ( JT808Conf_struct.Close_CommunicateFlag == 0 ))
	{
		//   存储方式盲区
		if( MangQU.Enable_SD_state == 1 )
		{
		               // ----- normal -----
						if( MangQU.Sd_flag == 0 )
						{
							MangQU.Sd_timer++;
								  if(MangQU.Sd_timer>=2) 
							{
								MangQU.Sd_timer = 0;
										 MangQU.NoAck_timer=0;
								MangQU.Sd_flag	= 1;
							}
						}
		              //------- no  ack  process -----
		              if(MangQU.Sd_flag==2)
		              	{
		              	  MangQU.NoAck_timer++;
						  if( MangQU.NoAck_timer>=2)  
						  {
						      MangQU.NoAck_timer=0;
							  MangQU.Sd_timer=0;
		                      MangQU.Sd_flag=1;		
							  rt_kprintf("\r\n MQ Noack rensend! \r\n");
						  }

		              	}
		 } 
	  // 真正盲区补报从发定时器
	  	if( MQ_TrueUse.Enable_SD_state == 1 )
		{
		               // ----- normal -----
						if( MQ_TrueUse.Sd_flag == 0 )
						{
							MQ_TrueUse.Sd_timer++;
								  if(MQ_TrueUse.Sd_timer>=2) 
							{
								MQ_TrueUse.Sd_timer = 0;
										 MQ_TrueUse.NoAck_timer=0;
								MQ_TrueUse.Sd_flag	= 1;
							}
						}
		              //------- no  ack  process -----
		              if(MQ_TrueUse.Sd_flag==2)
		              	{
		              	  MQ_TrueUse.NoAck_timer++;
						  if( MQ_TrueUse.NoAck_timer>=2)  
						  {
						      MQ_TrueUse.NoAck_timer=0;
							  MQ_TrueUse.Sd_timer=0;
							  cycle_read=mangQu_read_reg;   //   还原read重新发送
				              MQ_TrueUse.Sd_flag=1; //enable发送
							  rt_kprintf("\r\n MQ_true Noack rensend! \r\n");
						  }

		              	}
		 } 
				
	}
}

//----------------------------------------------------------------------
u8  Protocol_Head( u16 MSG_ID, u8 Packet_Type )
{
    u16  CurrentID=0; 
	//----  clear --------------
	Original_info_Wr = 0;
	//	1. Head

	//  original info
	Original_info[Original_info_Wr++]	= ( MSG_ID >> 8 );                                              // 消息ID
	Original_info[Original_info_Wr++]	= (u8)MSG_ID;

	Original_info[Original_info_Wr++]	= 0x00;                                                         // 分包、加密方式、状态位
	Original_info[Original_info_Wr++]	= 28;                                                           // 消息体长度   位置信息长度为28个字节

	memcpy( Original_info + Original_info_Wr, SIM_code, 6 );                                            // 终端手机号 ，设备标识ID	BCD
	Original_info_Wr += 6;


    switch(Recode_Obj.CMD)
    {
        case 0x08:CurrentID=0x8000+Recode_Obj.Current_pkt_num;
			      Original_info[Original_info_Wr++]   = ( CurrentID>> 8 ); 					  //消息流水号
				  Original_info[Original_info_Wr++]   =CurrentID;
			       break;
		case 0x09:CurrentID=0x9000+Recode_Obj.Current_pkt_num;
					Original_info[Original_info_Wr++]	= ( CurrentID>> 8 );					//消息流水号
					Original_info[Original_info_Wr++]	=CurrentID;

                   break;
		case 0x10:
			       CurrentID=0xA000+Recode_Obj.Current_pkt_num;
				   Original_info[Original_info_Wr++]   = ( CurrentID>> 8 ); 				   //消息流水号
				   Original_info[Original_info_Wr++]   =CurrentID;

			       break;
		case 0x11:
			       CurrentID=0xB000+Recode_Obj.Current_pkt_num;
				   Original_info[Original_info_Wr++]   = ( CurrentID>> 8 ); 				   //消息流水号
				   Original_info[Original_info_Wr++]   =CurrentID;
			       break;
		case 0x12:
			       CurrentID=0xC000+Recode_Obj.Current_pkt_num;
				   Original_info[Original_info_Wr++]   = ( CurrentID>> 8 ); 				   //消息流水号
				   Original_info[Original_info_Wr++]   =CurrentID;
			       break;
		case  0x15:
			      CurrentID=0xD000+Recode_Obj.Current_pkt_num;
			      Original_info[Original_info_Wr++]   = ( CurrentID>> 8 ); 					  //消息流水号
				  Original_info[Original_info_Wr++]   =CurrentID;
 
				   break;  
		default:	
			     if(MediaObj.Media_Type<3) // 不是  图片和音频
			     {
				    Original_info[Original_info_Wr++]   = ( JT808Conf_struct.Msg_Float_ID >> 8 ); 					  //消息流水号
				    Original_info[Original_info_Wr++]   = JT808Conf_struct.Msg_Float_ID;
			     }
				  break;

    } 


	if( Packet_Type == Packet_Divide )
	{
		switch( MediaObj.Media_Type )
		{
			case 0:                                                                                     // 图像
				MediaObj.Media_totalPacketNum	= Photo_sdState.Total_packetNum;                        // 图片总包数
				MediaObj.Media_currentPacketNum = Photo_sdState.SD_packetNum;                           // 图片当前报数
				MediaObj.Media_ID				= 1;                                                    //  多媒体ID
				MediaObj.Media_Channel			= Camera_Number;                                        // 图片摄像头通道号
                
				
			    CurrentID=0xF000+Camera_Number*0x0100+Photo_sdState.SD_packetNum;
				Original_info[Original_info_Wr++]=( CurrentID>>8); //消息流水号
				Original_info[Original_info_Wr++]=	CurrentID; 			   

				break;
			case 1:                                                                                     // 音频
				MediaObj.Media_totalPacketNum	= Sound_sdState.Total_packetNum;                        // 音频总包数
				MediaObj.Media_currentPacketNum = Sound_sdState.SD_packetNum;                           // 音频当前报数
				MediaObj.Media_ID				= 1;                                                    //  多媒体ID
				MediaObj.Media_Channel			= 1;                                                    // 音频通道号
                CurrentID=0xE000+Sound_sdState.SD_packetNum;
			    Original_info[Original_info_Wr++]=( CurrentID>>8); //消息流水号
	            Original_info[Original_info_Wr++]=  CurrentID; 
				break;
			case 2:                                                                                     // 视频
				MediaObj.Media_totalPacketNum	= Video_sdState.Total_packetNum;                        // 视频总包数
				MediaObj.Media_currentPacketNum = Video_sdState.SD_packetNum;                           // 视频当前报数
				MediaObj.Media_ID				= 1;                                                    //  多媒体ID
				MediaObj.Media_Channel			= 1;                                                    // 视频通道号
				break;
			case  3:                                                                                    //行车记录仪
				MediaObj.Media_totalPacketNum	= Recode_Obj.Total_pkt_num;                             // 记录仪总包数
				MediaObj.Media_currentPacketNum = Recode_Obj.Current_pkt_num;                           // 记录仪当前报数

				break;
			case  4:                                                                                    // 盲区补报
				MediaObj.Media_totalPacketNum	= BlindZone_sdState.Total_packetNum;                    // 盲区总包数
				MediaObj.Media_currentPacketNum = BlindZone_sdState.SD_packetNum;                       // 盲区当前报数

				break;
			default:
				return false;
		}

		Original_info[Original_info_Wr++]	= ( MediaObj.Media_totalPacketNum & 0xff00 ) >> 8;          //总block
		Original_info[Original_info_Wr++]	= (u8)MediaObj.Media_totalPacketNum;                        //总block

		Original_info[Original_info_Wr++]	= ( ( MediaObj.Media_currentPacketNum ) & 0xff00 ) >> 8;    //当前block
		Original_info[Original_info_Wr++]	= (u8)( ( MediaObj.Media_currentPacketNum ) & 0x00ff );     //当前block
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Protocol_End( u8 Packet_Type, u8 LinkNum )
{
	u16 packet_len	= 0;
	u16 i			= 0;    //要发送的UDP 数据内容的长度
	u8	Gfcs		= 0;
	u16 Msg_bodyLen = 0;    //  协议里的消息只表示消息体     不包含消息头 消息头默认长度是12 , 分包消息头长度 20

	Gfcs = 0;               //  计算从消息头开始到校验前数据的异或和  808协议校验  1Byte
	//---  填写信息长度 ---
	if( Packet_Normal == Packet_Type )
	{
		Msg_bodyLen			= Original_info_Wr - 12;
		Original_info[2]	= ( Msg_bodyLen >> 8 );
		Original_info[3]	= Msg_bodyLen;
	}else
	if( Packet_Divide == Packet_Type )
	{
		Msg_bodyLen = Original_info_Wr - 16;
		// rt_kprintf("\r\n Divide Infolen=%d  \r\n",Msg_bodyLen);
		Original_info[2]	= ( Msg_bodyLen >> 8 ) | 0x20; // Bit 13  0x20 就是Bit 13
		Original_info[3]	= Msg_bodyLen;
	}
	//---- 计算校验  -----
/*行车记录仪多包传输时，只在最后才加校验*/

	for( i = 0; i < Original_info_Wr; i++ )
	{
		Gfcs ^= Original_info[i];
	}
	Original_info[Original_info_Wr++] = Gfcs;   // 填写G校验位

	// 1.stuff start
	GPRS_infoWr_Tx				= 0;
	GPRS_info[GPRS_infoWr_Tx++] = 0x7e;         // Start 标识位
	if( Packet_Divide == Packet_Type )
	{
		//rt_kprintf("\r\n Tx=%d  Divide Infolen=%d  \r\n",GPRS_infoWr_Tx,Original_info_Wr);


		/* rt_kprintf("\r\n PacketContent: ");
		    for(i=0;i<Original_info_Wr;i++)
		                    rt_kprintf(" %X",Original_info[i]);
		    rt_kprintf("\r\n");
		 */
	}
	// 2.  convert
	packet_len		= Protocol_808_Encode( GPRS_info + GPRS_infoWr_Tx, Original_info, Original_info_Wr );
	GPRS_infoWr_Tx	+= packet_len;
	if( Packet_Divide == Packet_Type )
	{
		//rt_kprintf("\r\n Divide  Send Infolen=%d  \r\n",packet_len);


		/* rt_kprintf("\r\n EncodeContent: ");
		   for(i=0;i<packet_len;i++)
		                      rt_kprintf(" %X",GPRS_info[i+1]);
		   rt_kprintf("\r\n");
		 */
		//rt_kprintf("\r\n GPRStx  Send Infolen=%d  \r\n",GPRS_infoWr_Tx+1);
	}
	GPRS_info[GPRS_infoWr_Tx++] = 0x7e; //  End  标识
	//  4. Send

	// 4.1 发送信息内容1
	// 4.2   MsgQueue
	WatchDog_Feed( );

#ifdef BITTER
	rt_kprintf( "\r\n App to GSM info:" );
	for( i = 0; i < GPRS_infoWr_Tx; i++ )
	{
		rt_kprintf( " %02X", GPRS_info[i] );
	}
	rt_kprintf( "\r\n" );
#endif

	Gsm_rxAppData_SemRelease( GPRS_info, GPRS_infoWr_Tx, LinkNum );
	//--------消息序号 递增 --------
	JT808Conf_struct.Msg_Float_ID++;
	//------------------------------
}

u8  Stuff_BDSD_0200H(void)  
{
  u8 spd_sensorReg[2];
  u8  rd_infolen=0;
 //  1. Head  
   if(!Protocol_Head(MSG_0x0200,Packet_Normal)) 
 	  return false;  
 // 2. content 
 WatchDog_Feed();
 if( BDSD_ReadCycleGPS( BDSD.read,Original_info+Original_info_Wr, 128)==false)              
  {
     rt_kprintf("\r\n  读取 false\r\n "); 
    return false;
  }
 // 获取信息长度
 rd_infolen=Original_info[Original_info_Wr];
 //OutPrint_HEX("read -1",Original_info+Original_info_Wr,rd_infolen+1);
 memcpy(Original_info+Original_info_Wr,Original_info+Original_info_Wr+1,rd_infolen);
 //OutPrint_HEX("read -2",Original_info+Original_info_Wr,rd_infolen+1); 
 Original_info_Wr+=rd_infolen-1;   // 内容长度 剔除第一个长度字节 

 
 //  3. Send 
 Protocol_End(Packet_Normal ,0);
   	
 return true; 

}


//------------------------------------------------------------------------------------
u8  Stuff_MangQu_Packet_Send_0704H( void )
{
	u16 Qsize = 0;

	// 1. Head
	if( !Protocol_Head( MSG_0x0704, Packet_Normal ) )
	{
		return false;                           //定位数据批量上传
	}
	//  2.  Content
	//-----  判断填充-------
	//  2.1    数据项个数
	Original_info[Original_info_Wr++]	= 0x00; //  10000=0x2710
	Original_info[Original_info_Wr++]	= MQ_PKNUM;

	//  2.2    数据类型     1  盲区补报    0:   正常位置批量汇报
	Original_info[Original_info_Wr++] = 1;
	//  3.   数据包
	//  3.1   数据汇报体长度+ 内容

	// 3.2   数据内容

	MangQU_read( MangQU.PacketNum, Original_info + Original_info_Wr, Qsize );
	//  Original_info_Wr += Qsize;

	//   4 .  end
	Protocol_End( Packet_Normal, 0 );

	rt_kprintf( "\r\n	0704H  current=%d  total=%d  \r\n", MangQU.PacketNum, BLIND_NUM / MQ_PKNUM );
}


u8  Stuff_MangQu_Packet_Send_0704H_True( void )
{
    /*  Note:
                      读取存储时有可能存在读取校验不正确的可能，
                      这种情况下rd_error 要记录错误次数
                      同时填充的记录数也要递减
      */
    u8   Rd_error_Counter=0;  // 读取错误计数器
    u8   i=0;
	u16  len_wr_reg=0;//   长度单位下标 记录 
	u8   rd_infolen=0;
	u8	 reg_128[128];  // 0704 寄存器
   
	u16 Qsize = 0;

       //0 .  congifrm   batch  num
        if(cycle_read<cycle_write)
        {
            delta_0704_rd=cycle_write-cycle_read;
			// 判断偏差记录条数是否大于最大记录数
            if(delta_0704_rd>=MQ_PKNUM)
				 delta_0704_rd=MQ_PKNUM;   

        }
		else   // write 小于 read
		{	    
			delta_0704_rd=Max_CycleNum-cycle_read; 
		}

		 rt_kprintf("\r\n	 delat_0704=%d\r\n",delta_0704_rd); 

	// 1. Head
	 if(!Protocol_Head(MSG_0x0704,Packet_Normal)) 
		  return false; 
	  // 2. content   
	  //  2.1	 数据项个数
	  Original_info[Original_info_Wr++]   = 0x00; //  10000=0x2710
	  len_wr_reg=Original_info_Wr; //记录长度下标
	  Original_info[Original_info_Wr++]   = delta_0704_rd;
	  
	  //  2.2	 数据类型	  1  盲区补报	 0:   正常位置批量汇报
	  Original_info[Original_info_Wr++] = 1;

	  //	2.3  数据项目
		mangQu_read_reg=cycle_read;   //   存储当前的记录
		 for(i=0;i<delta_0704_rd;i++)
		 {
			//	 读取信息
		   memset(reg_128,0,sizeof(reg_128)); 
		   if( ReadCycleGPS(cycle_read,reg_128, 32)==false)	 // 实际内容只有28个字节
		   {  
			  Rd_error_Counter++;
			  continue; 
		   } 
		   cycle_read++; 
			//----------  子项信息长度 --------------------------		   
			rd_infolen=28;
			Original_info[Original_info_Wr++]	= 0;
			Original_info[Original_info_Wr++]	= 32; // 28+ 附件信息长度
	
			memcpy(Original_info+Original_info_Wr,reg_128,28);
			Original_info_Wr+=28; 	// 内容长度 剔除第一个长度字节	

	        
			//----------- 附加信息	------------
			//	附加信息 1	-----------------------------
			//	附加信息 ID
			Original_info[Original_info_Wr++] = 0x03;														// 行驶记录仪的速度
			//	附加信息长度
			Original_info[Original_info_Wr++] = 2;
			//	类型
			Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
			Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
			
			//OutPrint_HEX("read -1"reg_128,rd_infolen+1);
			
			//OutPrint_HEX("read -2",reg_128+1,rd_infolen); 
	//==================================================	
		}

	//   3 .  end
	Protocol_End( Packet_Normal, 0 );

    
	if(DispContent)
			rt_kprintf("\r\n  定位数据批量上传	delta=%d   read=%d  write=%d current=%d  ConstTotal=%d   CurrentTotal=%d\r\n",delta_0704_rd,cycle_read,cycle_write,MQ_TrueUse.PacketNum, Mq_total_pkg,CurrentTotal);  
	if(MQ_TrueUse.PacketNum==Mq_total_pkg)
		    rt_kprintf("\r\n  盲区True存储部分已经完成\r\n");
	if(cycle_read==cycle_write) 
		{ 
           MQ_TrueUse.PacketNum=0;
		   MQ_TrueUse.Enable_SD_state=0; 
		   // 每次应答存储相关记录数目
		   DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd);  
		   rt_kprintf("\r\nTrue Return Normal_place2\r\n");
		}

}

//--------------------------------------------------------------------------------------
u8  Stuff_DevCommmonACK_0001H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0001, Packet_Normal ) )
	{
		return false; //终端通用应答
	}
	// 2. content  is null
	//   float ID
	Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;
	//  cmd  ID
	Original_info[Original_info_Wr++]	= (u8)( Centre_CmdID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Centre_CmdID;
	//   resualt
	Original_info[Original_info_Wr++] = SD_ACKflag.f_CentreCMDack_resualt;
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Common CMD ACK! \r\n");
	}
	return true;
}

//-------------------------------------------------------------------------
u8  Stuff_RegisterPacket_0100H( u8 LinkNum )
{
	u8 i = 0;
// 1. Head
	if( !Protocol_Head( MSG_0x0100, Packet_Normal ) )
	{
		return false;
	}

	// 2. content
	//  province ID
	//  Original_info[Original_info_Wr++]=(u8)(JT808Conf_struct.Vechicle_Info.Dev_ProvinceID>>8);
	// Original_info[Original_info_Wr++]=(u8)JT808Conf_struct.Vechicle_Info.Dev_ProvinceID;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 12;               // 天津市
	//  county  ID
	// Original_info[Original_info_Wr++]=(u8)(JT808Conf_struct.Vechicle_Info.Dev_CityID>>8);
	//Original_info[Original_info_Wr++]=(u8)JT808Conf_struct.Vechicle_Info.Dev_CityID;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 107;              // 塘沽区
	//  厂商ID
	memcpy( Original_info + Original_info_Wr, "70420", 5 ); //TW705
	Original_info_Wr += 5;
	//  终端型号 20 Bytes      -- 补充协议里做更改
	//memcpy(Original_info+Original_info_Wr,"Tianjin TCB TW701-BD",20);
	// Original_info_Wr+=20;
	memcpy( Original_info + Original_info_Wr, "TW705", 5 ); //  正式  ----
	// memcpy(Original_info+Original_info_Wr,"SW703",5);      // 调试
	Original_info_Wr += 5;
	for( i = 0; i < 15; i++ )
	{
		Original_info[Original_info_Wr++] = 0x00;
	}

	//  终端ID   7 Bytes    ,
	memcpy( Original_info + Original_info_Wr, IMSI_CODE + 8, 7 );                   //000013601300001
	Original_info_Wr += 7;
	//   Original_info_Wr+=3;
	// Original_info[Original_info_Wr++]='0';
	// memcpy(Original_info+Original_info_Wr,IMSI_CODE+12,3);
	// Original_info_Wr+=3;
	//  车牌颜色
	Original_info[Original_info_Wr++] = JT808Conf_struct.Vechicle_Info.Dev_Color;   // 车辆颜色
	//  车牌
	/*
	memcpy( Original_info + Original_info_Wr, "TST000", 6 ); 
	Original_info_Wr					+= 6;
	Original_info[Original_info_Wr++]	= IMSI_CODE[14];
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	*/
	memcpy(Original_info+Original_info_Wr,JT808Conf_struct.Vechicle_Info.Vech_Num,8);
	Original_info_Wr+=8;

	//  3. Send
	Protocol_End( Packet_Normal, LinkNum );
	if( DispContent )
	{
		rt_kprintf( "\r\n	SEND Reigster Packet! \r\n");
	}
	return true;
}

//--------------------------------------------------------------------------------------
u8  Stuff_DeviceHeartPacket_0002H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0002, Packet_Normal ) )
	{
		return false;
	}
	// 2. content  is null

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Send Dev Heart! \r\n");
	}
	return true;
}

//--------------------------------------------------------------------------------------
u8  Stuff_DeviceDeregister_0101H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0003, Packet_Normal ) )
	{
		return false; //终端注销
	}
	// 2. content  is null
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Deregister  注销! \r\n");
	}
	return true;
}

//------------------------------------------------------------------------------------
u8  Stuff_DevLogin_0102H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0102, Packet_Normal ) )
	{
		return false;                                                                                                               //终端鉴权
	}
	// 2. content

	memcpy( Original_info + Original_info_Wr, JT808Conf_struct.ConfirmCode, strlen( (const char*)JT808Conf_struct.ConfirmCode ) );  // 鉴权码  string Type
	Original_info_Wr += strlen( (const char*)JT808Conf_struct.ConfirmCode );
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 发送鉴权! \r\n");
	}
	return true;
}

//--------------------------------------------------------------------------------------
u8  Stuff_Normal_Data_0200H( u16 InNUM )
{
	u8	status = 0, U8_STUFF = 0;
	u8	spd_sensorReg[2];
	u32 Dis_01km = 0;
	// ---- add  new  --------
	u8	lati[4], longi[4];
	u32 lati_stuff	= 0, longi_stuff = 0;
	u16 spd_stuff	= 0;
	u8	time_stuff[2];

	//  1. Head
	if( !Protocol_Head( MSG_0x0200, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//------------------------------- Stuff ----------------------------------------
	// 1. 告警标志  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
	Original_info_Wr += 4;
	// 2. 状态  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
	Original_info_Wr += 4;
	// 3.	纬度
	lati_stuff	= Latiude_hex[InNUM] - ( CSQ_counter % 7 ) + ( TIM1_Timer_Counter % 2 );    // need change  a  little
	lati[0]		= ( u8 )( lati_stuff >> 24 );
	lati[1]		= ( u8 )( lati_stuff >> 16 );
	lati[2]		= ( u8 )( lati_stuff >> 8 );
	lati[3]		= ( u8 )lati_stuff;
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )lati, 4 );                  //纬度	modify by nathan
	Original_info_Wr += 4;
	// 4.	经度

	longi_stuff = 116 * 1000000 - ( CSQ_counter % 8 ) + ( TIM1_Timer_Counter % 3 );
	longi[0]	= ( u8 )( longi_stuff >> 24 );
	longi[1]	= ( u8 )( longi_stuff >> 16 );
	longi[2]	= ( u8 )( longi_stuff >> 8 );
	longi[3]	= ( u8 )longi_stuff;
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )longi, 4 ); //经度	 东经  Bit 7->0   西经 Bit 7 -> 1
	Original_info_Wr += 4;
	// 5.	高程
	GPS_Hight							= 50 + ( CSQ_counter % 5 ) - ( Satelite_num % 4 );
	Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
	Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
	// 6.	速度	0.1 Km/h
	if( spd_dex[InNUM] > 11 )
	{
		spd_stuff = spd_dex[InNUM] - ( CSQ_counter % 10 ) + ( TIM1_Timer_Counter % 3 ); // need change  a  little  76
	}else
	{
		spd_stuff = spd_dex[InNUM];
	}
	Original_info[Original_info_Wr++]	= (u8)( spd_stuff >> 8 );                       //(GPS_speed>>8);
	Original_info[Original_info_Wr++]	= (u8)( spd_stuff );                            //GPS_speed;
	// 7. 方向   单位 1度
	Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );                       //High
	Original_info[Original_info_Wr++]	= GPS_direction;                                // Low
	// 8.	日期时间
	Original_info[Original_info_Wr++]	= 0x13;                                         //(((Gps_Gprs.Date[0])/10)<<4)+((Gps_Gprs.Date[0])%10);
	Original_info[Original_info_Wr++]	= 0x03;                                         //((Gps_Gprs.Date[1]/10)<<4)+(Gps_Gprs.Date[1]%10);
	Original_info[Original_info_Wr++]	= 0x05;                                         // ((Gps_Gprs.Date[2]/10)<<4)+(Gps_Gprs.Date[2]%10);
	Original_info[Original_info_Wr++]	= 0x18;                                         //((Gps_Gprs.Time[0]/10)<<4)+(Gps_Gprs.Time[0]%10);
	time_stuff[0]						= 12 + ( 30 + InNUM ) / 60;
	Original_info[Original_info_Wr++]	= ( ( time_stuff[0] / 10 ) << 4 ) + ( time_stuff[0] % 10 );
	time_stuff[1]						= ( 30 + InNUM ) % 60;
	Original_info[Original_info_Wr++]	= ( ( time_stuff[1] / 10 ) << 4 ) + ( time_stuff[1] % 10 );

	//----------- 附加信息  ------------
	//  附加信息 1  -----------------------------
	//	附加信息 ID
	Original_info[Original_info_Wr++] = 0x03; // 行驶记录仪的速度
	//	附加信息长度
	Original_info[Original_info_Wr++] = 2;
	//	类型
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
	//rt_kprintf("\r\n GPS速度=%d km/h , 传感器速度=%d km/h\r\n",Speed_gps,Speed_cacu);
	//  附加信息 2  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x01; // 车上的行驶里程
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Dis_01km							= JT808Conf_struct.Distance_m_u32 / 100;
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 24 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 16 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 8 );
	Original_info[Original_info_Wr++]	= Dis_01km;

	//  附加信息 3
	if( Warn_Status[1] & 0x10 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x12; //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 6;
		//  类型
		Original_info[Original_info_Wr++]	= InOut_Object.TYPE;
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 24 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 16 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 8 );
		Original_info[Original_info_Wr++]	= InOut_Object.ID;
		Original_info[Original_info_Wr++]	= InOut_Object.InOutState;
		rt_kprintf( "\r\n ----- 0x0200 current 附加信息 \r\n" );
	}

	//  附件信息4
	if( Warn_Status[3] & 0x02 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x11;                   //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 1;
		//  类型
		Original_info[Original_info_Wr++] = 0;                      //  无特定位置
	}
	// 5. 附加信息
	Original_info[Original_info_Wr++]	= 0x25;                     //ID  扩展车辆信号状态位
	Original_info[Original_info_Wr++]	= 4;                        //LEN
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= BD_EXT.Extent_IO_status;
	//6. 附加信息  :
	if( SleepState == 1 )                                           //休眠
	{
		Original_info[Original_info_Wr++]	= 0x2A;                 //ID   IO状态位【1】   bit0:深度休眠   bit1:休眠【0】保留
		Original_info[Original_info_Wr++]	= 2;                    //LEN
		Original_info[Original_info_Wr++]	= 0x02;
		Original_info[Original_info_Wr++]	= 0;
	}
	//  附加信息 7  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x2B;                       //模拟量
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Original_info[Original_info_Wr++]	= ( AD_2through[1] >> 8 );  // AD1
	Original_info[Original_info_Wr++]	= AD_2through[1];
	Original_info[Original_info_Wr++]	= ( AD_2through[0] >> 8 );  // AD0
	Original_info[Original_info_Wr++]	= AD_2through[0];
	//  附加信息 8  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x30;                       //无线通信网络信号强度
	//  附加信息长度
	Original_info[Original_info_Wr++] = 1;
	//  类型
	Original_info[Original_info_Wr++] = ModuleSQ;

	//  附加信号 9  ---------------------------------
	Original_info[Original_info_Wr++]	= 0x31;                     // GNSS 定位卫星颗数
	Original_info[Original_info_Wr++]	= 1;                        // len
	Original_info[Original_info_Wr++]	= Satelite_num;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );

	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_AppMq_Data_0200H( u8 *instr )
{
	u8	status		= 0, U8_STUFF = 0;
	u32 Dis_01km	= 0;
	//  1. Head
	if( !Protocol_Head( MSG_0x0200, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	memcpy( Original_info + Original_info_Wr, instr, 28 );
	Original_info_Wr += 28;                     // 内容只有28

	//----------- 附加信息  ------------
	//  附加信息 1  -----------------------------
	//	附加信息 ID
	Original_info[Original_info_Wr++] = 0x03;   // 行驶记录仪的速度
	//	附加信息长度
	Original_info[Original_info_Wr++] = 2;
	//	类型
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
	//rt_kprintf("\r\n GPS速度=%d km/h , 传感器速度=%d km/h\r\n",Speed_gps,Speed_cacu);
	//  附加信息 2  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x01; // 车上的行驶里程
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Dis_01km							= JT808Conf_struct.Distance_m_u32 / 100;
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 24 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 16 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 8 );
	Original_info[Original_info_Wr++]	= Dis_01km;

	//  附加信息 3
	if( Warn_Status[1] & 0x10 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x12; //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 6;
		//  类型
		Original_info[Original_info_Wr++]	= InOut_Object.TYPE;
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 24 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 16 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 8 );
		Original_info[Original_info_Wr++]	= InOut_Object.ID;
		Original_info[Original_info_Wr++]	= InOut_Object.InOutState;
		rt_kprintf( "\r\n ----- 0x0200 current 附加信息 \r\n" );
	}

	//  附件信息4
	if( Warn_Status[3] & 0x02 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x11;                   //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 1;
		//  类型
		Original_info[Original_info_Wr++] = 0;                      //  无特定位置
	}
	// 5. 附加信息
	Original_info[Original_info_Wr++]	= 0x25;                     //ID  扩展车辆信号状态位
	Original_info[Original_info_Wr++]	= 4;                        //LEN
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= BD_EXT.Extent_IO_status;
	//6. 附加信息  :
	if( SleepState == 1 )                                           //休眠
	{
		Original_info[Original_info_Wr++]	= 0x2A;                 //ID   IO状态位【1】   bit0:深度休眠   bit1:休眠【0】保留
		Original_info[Original_info_Wr++]	= 2;                    //LEN
		Original_info[Original_info_Wr++]	= 0x02;
		Original_info[Original_info_Wr++]	= 0;
	}
	//  附加信息 7  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x2B;                       //模拟量
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Original_info[Original_info_Wr++]	= ( AD_2through[1] >> 8 );  // AD1
	Original_info[Original_info_Wr++]	= AD_2through[1];
	Original_info[Original_info_Wr++]	= ( AD_2through[0] >> 8 );  // AD0
	Original_info[Original_info_Wr++]	= AD_2through[0];
	//  附加信息 8  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x30;                       //无线通信网络信号强度
	//  附加信息长度
	Original_info[Original_info_Wr++] = 1;
	//  类型
	Original_info[Original_info_Wr++] = ModuleSQ;

	//  附加信号 9  ---------------------------------
	Original_info[Original_info_Wr++]	= 0x31;                     // GNSS 定位卫星颗数
	Original_info[Original_info_Wr++]	= 1;                        // len
	Original_info[Original_info_Wr++]	= Satelite_num;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );

	return true;
}

//------------------------------------------------------------------------------------
u8  Stuff_Current_Data_0200H( void )   //  发送即时数据不存储到存储器中
{
	u8	status		= 0, U8_STUFF = 0;
	u32 Dis_01km	= 0;

	if( GPS_speed <= ( JT808Conf_struct.Speed_warn_MAX * 10 ) )
	{
		StatusReg_SPD_NORMAL( );
	}

	//  1. Head
	if( !Protocol_Head( MSG_0x0200, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//------------------------------- Stuff ----------------------------------------
	// 1. 告警标志  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
	Original_info_Wr += 4;
	// 2. 状态  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
	Original_info_Wr += 4;
	// 3.  纬度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Latitude, 4 );     //纬度   modify by nathan
	Original_info_Wr += 4;
	// 4.  经度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Longitude, 4 );    //经度    东经  Bit 7->0   西经 Bit 7 -> 1
	Original_info_Wr += 4;
	// 5.  高程
	Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
	Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
	// 6.  速度    0.1 Km/h
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps >> 8 );                           //(GPS_speed>>8);
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps );                                //GPS_speed;
	// 7. 方向   单位 1度
	Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );                           //High
	Original_info[Original_info_Wr++]	= GPS_direction;                                    // Low
	// 8.  日期时间
	Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

	//----------- 附加信息  ------------
	//  附加信息 1  -----------------------------
	//	附加信息 ID
	Original_info[Original_info_Wr++] = 0x03; // 行驶记录仪的速度
	//	附加信息长度
	Original_info[Original_info_Wr++] = 2;
	//	类型
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
	//rt_kprintf("\r\n GPS速度=%d km/h , 传感器速度=%d km/h\r\n",Speed_gps,Speed_cacu);
	//  附加信息 2  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x01; // 车上的行驶里程
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Dis_01km							= JT808Conf_struct.Distance_m_u32 / 100;
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 24 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 16 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 8 );
	Original_info[Original_info_Wr++]	= Dis_01km;

	//  附加信息 3
	if( Warn_Status[1] & 0x10 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x12; //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 6;
		//  类型
		Original_info[Original_info_Wr++]	= InOut_Object.TYPE;
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 24 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 16 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 8 );
		Original_info[Original_info_Wr++]	= InOut_Object.ID;
		Original_info[Original_info_Wr++]	= InOut_Object.InOutState;
		rt_kprintf( "\r\n ----- 0x0200 current 附加信息 \r\n" );
	}

	if( Warn_Status[1] & 0x80 )                     // 出 路线报警
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x12;   //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 6;
		//  类型
		InOut_Object.ID						= 100;  // 下的是100
		Original_info[Original_info_Wr++]	= 4;
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 24 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 16 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 8 );
		Original_info[Original_info_Wr++]	= InOut_Object.ID;
		Original_info[Original_info_Wr++]	= 1;
		rt_kprintf( "\r\n ----- 0x0200 线路报警 附加信息 \r\n" );
	}

	//  附件信息4
	if( Warn_Status[3] & 0x02 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x11;                   //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 1;
		//  类型
		Original_info[Original_info_Wr++] = 0;                      //  无特定位置
	}
	// 5. 附加信息
	Original_info[Original_info_Wr++]	= 0x25;                     //ID  扩展车辆信号状态位
	Original_info[Original_info_Wr++]	= 4;                        //LEN
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= BD_EXT.Extent_IO_status;
	//6. 附加信息  :
	if( SleepState == 1 )                                           //休眠
	{
		Original_info[Original_info_Wr++]	= 0x2A;                 //ID   IO状态位【1】   bit0:深度休眠   bit1:休眠【0】保留
		Original_info[Original_info_Wr++]	= 2;                    //LEN
		Original_info[Original_info_Wr++]	= 0x02;
		Original_info[Original_info_Wr++]	= 0;
	}
	//  附加信息 7  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x2B;                       //模拟量
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Original_info[Original_info_Wr++]	= ( AD_2through[1] >> 8 );  // AD1
	Original_info[Original_info_Wr++]	= AD_2through[1];
	Original_info[Original_info_Wr++]	= ( AD_2through[0] >> 8 );  // AD0
	Original_info[Original_info_Wr++]	= AD_2through[0];
	//  附加信息 8  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x30;                       //无线通信网络信号强度
	//  附加信息长度
	Original_info[Original_info_Wr++] = 1;
	//  类型
	Original_info[Original_info_Wr++] = ModuleSQ;

	//  附加信号 9  ---------------------------------
	Original_info[Original_info_Wr++]	= 0x31;                     // GNSS 定位卫星颗数
	Original_info[Original_info_Wr++]	= 1;                        // len
	Original_info[Original_info_Wr++]	= Satelite_num;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	return true;
}

//-----------------------------------------------------------------------
u8  Stuff_Current_Data_0201H( void )   //   位置信息查询回应
{
	u8	status		= 0, U8_STUFF = 0;
	u32 Dis_01km	= 0;
	//  1. Head
	if( !Protocol_Head( MSG_0x0201, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//------------------------------- Stuff ----------------------------------------
	//   float ID                                                // 对应中心应答消息的流水号
	Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;

	// 1. 告警标志  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
	Original_info_Wr += 4;
	// 2. 状态  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
	Original_info_Wr += 4;
	// 3.  纬度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Latitude, 4 );     //纬度   modify by nathan
	Original_info_Wr += 4;
	// 4.  经度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Longitude, 4 );    //经度    东经  Bit 7->0   西经 Bit 7 -> 1
	Original_info_Wr += 4;
	// 5.  高程
	Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
	Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
	// 6.  速度    0.1 Km/h
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Speed_gps;
	// 7. 方向   单位 1度
	Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );   //High
	Original_info[Original_info_Wr++]	= GPS_direction;            // Low
	// 8.  日期时间
	Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

	//----------- 附加信息  ------------
	//  附加信息 1  -----------------------------
	//	附加信息 ID
	Original_info[Original_info_Wr++] = 0x03; // 行驶记录仪的速度
	//	附加信息长度
	Original_info[Original_info_Wr++] = 2;
	//	类型
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu >> 8 );
	Original_info[Original_info_Wr++]	= (u8)( Speed_cacu );
	//rt_kprintf("\r\n GPS速度=%d km/h , 传感器速度=%d km/h\r\n",Speed_gps,Speed_cacu);
	//  附加信息 2  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x01; // 车上的行驶里程
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Dis_01km							= JT808Conf_struct.Distance_m_u32 / 100;
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 24 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 16 );
	Original_info[Original_info_Wr++]	= ( Dis_01km >> 8 );
	Original_info[Original_info_Wr++]	= Dis_01km;

	//  附加信息 3
	if( Warn_Status[1] & 0x10 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x12; //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 6;
		//  类型
		Original_info[Original_info_Wr++]	= InOut_Object.TYPE;
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 24 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 16 );
		Original_info[Original_info_Wr++]	= ( InOut_Object.ID >> 8 );
		Original_info[Original_info_Wr++]	= InOut_Object.ID;
		Original_info[Original_info_Wr++]	= InOut_Object.InOutState;
		rt_kprintf( "\r\n ----- 0x0200 current 附加信息 \r\n" );
	}

	//  附件信息4
	if( Warn_Status[3] & 0x02 )
	{
		//  附加信息 ID
		Original_info[Original_info_Wr++] = 0x11;                   //  进出区域/路线报警
		//  附加信息长度
		Original_info[Original_info_Wr++] = 1;
		//  类型
		Original_info[Original_info_Wr++] = 0;                      //  无特定位置
	}
	// 5. 附加信息       扩展车辆信号线
	Original_info[Original_info_Wr++]	= 0x25;                     //ID  扩展车辆信号状态位
	Original_info[Original_info_Wr++]	= 4;                        //LEN
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= 0x00;
	Original_info[Original_info_Wr++]	= BD_EXT.Extent_IO_status;
	//6. 附加信息  :
	if( SleepState == 1 )                                           //休眠
	{
		Original_info[Original_info_Wr++]	= 0x2A;                 //ID   IO状态位【1】   bit0:深度休眠   bit1:休眠【0】保留
		Original_info[Original_info_Wr++]	= 2;                    //LEN
		Original_info[Original_info_Wr++]	= 0x02;
		Original_info[Original_info_Wr++]	= 0;
	}
	//  附加信息 7  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x2B;                       //模拟量
	//  附加信息长度
	Original_info[Original_info_Wr++] = 4;
	//  类型
	Original_info[Original_info_Wr++]	= ( AD_2through[1] >> 8 );  // AD1
	Original_info[Original_info_Wr++]	= AD_2through[1];
	Original_info[Original_info_Wr++]	= ( AD_2through[0] >> 8 );  // AD0
	Original_info[Original_info_Wr++]	= AD_2through[0];
	//  附加信息 8  -----------------------------
	//  附加信息 ID
	Original_info[Original_info_Wr++] = 0x30;                       //无线通信网络信号强度
	//  附加信息长度
	Original_info[Original_info_Wr++] = 1;
	//  类型
	Original_info[Original_info_Wr++] = ModuleSQ;

	//  附加信号 9  ---------------------------------
	Original_info[Original_info_Wr++]	= 0x31;                     // GNSS 定位卫星颗数
	Original_info[Original_info_Wr++]	= 1;                        // len
	Original_info[Original_info_Wr++]	= Satelite_num;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	SEND GPS CMD=81H ! \r\n");
	}

	return true;
}

//-----------------------------------------------------------------------
u8  Sub_stuff_AppointedPram_0104( void )
{
	//   参数个数
	Original_info[Original_info_Wr++] = Setting_Qry.Num_pram;
	//   参数列表
}

//-----------------------------------------------------------------------
u8  Stuff_SettingPram_0104H( u8 in )
{
	u8 reg_str[30];

	//  1. Head
	if( !Protocol_Head( MSG_0x0104, Packet_Normal ) )
	{
		return false; // 终端参数上传
	}
	//  2. content
	//   float ID
	Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;
//if(in==1)
	{
		//   参数个数
		Original_info[Original_info_Wr++] = 4;
		//   参数列表

		//   2.1   车牌号


		/* Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x83;
		   Original_info[Original_info_Wr++]=strlen((const char*)JT808Conf_struct.Vechicle_Info.Vech_Num); // 参数长度
		   memcpy( ( char * ) Original_info+ Original_info_Wr, ( char * )JT808Conf_struct.Vechicle_Info.Vech_Num,strlen((const char*)JT808Conf_struct.Vechicle_Info.Vech_Num) ); // 参数值
		   Original_info_Wr+=strlen((const char*)JT808Conf_struct.Vechicle_Info.Vech_Num);
		 */
		//   2.2  主服务器IP
		Original_info[Original_info_Wr++]	= 0x00; // 参数ID 4Bytes
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x13;
		// 参数长度
		memset( reg_str, 0, sizeof( reg_str ) );
		IP_Str( (char*)reg_str, *( u32* )RemoteIP_aux );
		Original_info[Original_info_Wr++] = strlen( (const char*)reg_str );
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )reg_str, strlen( (const char*)reg_str ) );          // 参数值
		Original_info_Wr += strlen( (const char*)reg_str );

		//   2.3   主服务TCP端口
		Original_info[Original_info_Wr++]	= 0x00;                                                                     // 参数ID 4Bytes
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x18;
		Original_info[Original_info_Wr++]	= 4;                                                                        // 参数长度
		Original_info[Original_info_Wr++]	= 0x00;                                                                     // 参数值
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= ( RemotePort_main >> 8 );
		Original_info[Original_info_Wr++]	= RemotePort_main;

		//   2.4  APN 字符串
		Original_info[Original_info_Wr++]	= 0x00;                                                                     // 参数ID 4Bytes
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x10;
		Original_info[Original_info_Wr++]	= strlen( (const char*)APN_String );                                        // 参数长度
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )APN_String, strlen( (const char*)APN_String ) );    // 参数值
		Original_info_Wr += strlen( (const char*)APN_String );

		//  2.5   备用IP
		Original_info[Original_info_Wr++]	= 0x00;                                                                     // 参数ID 4Bytes
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x17;
		// 参数长度
		memset( reg_str, 0, sizeof( reg_str ) );
		IP_Str( (char*)reg_str, *( u32* )RemoteIP_aux );
		Original_info[Original_info_Wr++] = strlen( (const char*)reg_str );
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )reg_str, strlen( (const char*)reg_str ) ); // 参数值
		Original_info_Wr += strlen( (const char*)reg_str );


		/*
		   //   2.4   缺省时间上报间隔
		   Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x29;
		   Original_info[Original_info_Wr++]=4  ; // 参数长度
		   Original_info[Original_info_Wr++]=(JT808Conf_struct.DURATION.Default_Dur>>24);   // 参数值
		   Original_info[Original_info_Wr++]=(JT808Conf_struct.DURATION.Default_Dur>>16);
		   Original_info[Original_info_Wr++]=(JT808Conf_struct.DURATION.Default_Dur>>8);
		   Original_info[Original_info_Wr++]=(JT808Conf_struct.DURATION.Default_Dur);

		   //   2.5   中心监控号码
		   Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x40;
		   Original_info[Original_info_Wr++]=strlen((const char*)JT808Conf_struct.LISTEN_Num); // 参数长度
		   memcpy( ( char * ) Original_info+ Original_info_Wr, ( char * )JT808Conf_struct.LISTEN_Num,strlen((const char*)JT808Conf_struct.LISTEN_Num)); // 参数值
		   Original_info_Wr+=strlen((const char*)JT808Conf_struct.LISTEN_Num);
		   //   2.6   最大速度门限
		   Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x55;
		   Original_info[Original_info_Wr++]=4  ; // 参数长度
		   Original_info[Original_info_Wr++]=( JT808Conf_struct.Speed_warn_MAX>>24);   // 参数值
		   Original_info[Original_info_Wr++]=( JT808Conf_struct.Speed_warn_MAX>>16);
		   Original_info[Original_info_Wr++]=( JT808Conf_struct.Speed_warn_MAX>>8);
		   Original_info[Original_info_Wr++]=( JT808Conf_struct.Speed_warn_MAX);
		   //   2.7   连续驾驶门限
		   Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x57;
		   Original_info[Original_info_Wr++]=4  ; // 参数长度
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_DrvKeepingSec>>24);   // 参数值
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_DrvKeepingSec>>16);
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_DrvKeepingSec>>8);
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_DrvKeepingSec);
		   //   2.8   最小休息时间
		   Original_info[Original_info_Wr++]=0x00;   // 参数ID 4Bytes
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x00;
		   Original_info[Original_info_Wr++]=0x59;
		   Original_info[Original_info_Wr++]=4  ; // 参数长度
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_MinSleepSec>>24);	 // 参数值
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_MinSleepSec>>16);
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_MinSleepSec>>8);
		   Original_info[Original_info_Wr++]=(TiredConf_struct.TiredDoor.Door_MinSleepSec);
		 */
	}


/*else
   if(in==2)
   {
   Sub_stuff_AppointedPram_0104();
   }*/
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	发送参数查询信息! \r\n");
	}

	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_DeviceAttribute_BD_0107H( void )
{
	u16 infoLen = 0;
	// 1. Head
	if( !Protocol_Head( MSG_0x0107, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	Original_info[Original_info_Wr++] = ProductAttribute._1_DevType;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._2_ProducterID, 5 );
	Original_info_Wr += 5;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._3_Dev_TYPENUM, 20 );
	Original_info_Wr += 20;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._4_Dev_ID, 7 );
	Original_info_Wr += 7;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._5_Sim_ICCID, 10 );
	Original_info_Wr += 10;

	Original_info[Original_info_Wr++] = ProductAttribute._6_HardwareVer_Len;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._7_HardwareVer, ProductAttribute._6_HardwareVer_Len );
	Original_info_Wr += ProductAttribute._6_HardwareVer_Len;

	Original_info[Original_info_Wr++] = ProductAttribute._8_SoftwareVer_len;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._9_SoftwareVer, ProductAttribute._8_SoftwareVer_len );
	Original_info_Wr += ProductAttribute._8_SoftwareVer_len;

	Original_info[Original_info_Wr++] = ProductAttribute._10_FirmWareVer_len;
	memcpy( ( char* )Original_info + Original_info_Wr, (u8*)ProductAttribute._11_FirmWare, ProductAttribute._10_FirmWareVer_len );
	Original_info_Wr += ProductAttribute._10_FirmWareVer_len;

	Original_info[Original_info_Wr++]	= ProductAttribute._12_GNSSAttribute;
	Original_info[Original_info_Wr++]	= ProductAttribute._13_ComModuleAttribute;


	/*
	   infoLen=sizeof(ProductAttribute);
	   memcpy( ( char * ) Original_info+ Original_info_Wr,(u8*)&ProductAttribute,infoLen);
	   Original_info_Wr+=infoLen;
	 */

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	发送终端属性 \r\n");
	}

	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_ISP_Resualt_BD_0108H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0108, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	BD_ISP.ISP_running = 0;                                     // clear
	//----------------------------------------------------
	Original_info[Original_info_Wr++]	= BD_ISP.Update_Type;   // 升级类型
	Original_info[Original_info_Wr++]	= 0;                    //BD_ISP.Update_Type;  // 升级结果

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	远程升级结果上报 \r\n");
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_BatchDataTrans_BD_0704H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0704, Packet_Divide ) )
	{
		return false;
	}
	// 2. content
	// 2.1   数据项个数
	Original_info[Original_info_Wr++] = 1;
	// 2.2   位置数据类型
	Original_info[Original_info_Wr++] = 0; //  0: 正常上报  1: 盲区补报
	// 2.3   位置汇报数据项
	// 1. 告警标志  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
	Original_info_Wr += 4;
	// 2. 状态  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
	Original_info_Wr += 4;
	// 3.  纬度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Latitude, 4 );     //纬度   modify by nathan
	Original_info_Wr += 4;
	// 4.  经度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Longitude, 4 );    //经度    东经  Bit 7->0   西经 Bit 7 -> 1
	Original_info_Wr += 4;
	// 5.  高程
	Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
	Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
	// 6.  速度    0.1 Km/h
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps >> 8 );                           //(GPS_speed>>8);
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps );                                //GPS_speed;
	// 7. 方向   单位 1度
	Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );                           //High
	Original_info[Original_info_Wr++]	= GPS_direction;                                    // Low
	// 8.  日期时间
	Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	定位数据批量上传\r\n");
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_CANDataTrans_BD_0705H( void )
{
	u16 DataNum = 0, i = 0;
	u32 read_rd = 0;
	// 1. Head
	if( !Protocol_Head( MSG_0x0705, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	// 数据项个数
	DataNum								= ( CAN_trans.canid_1_SdWr >> 3 ); // 除以8
	Original_info[Original_info_Wr++]	= ( DataNum >> 8 );
	Original_info[Original_info_Wr++]	= DataNum;
	Can_sdnum							+= DataNum;
	//接收时间
	Original_info[Original_info_Wr++]	= ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
	Original_info[Original_info_Wr++]	= ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
	Original_info[Original_info_Wr++]	= ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );
	Original_info[Original_info_Wr++]	= ( Can_same % 10 ); //0x00;  // ms 毫秒
	Original_info[Original_info_Wr++]	= 0x00;
	//  CAN 总线数据项
	read_rd = 0;
	for( i = 0; i < DataNum; i++ )
	{
		/*
		   Original_info[Original_info_Wr++]= (CAN_trans.canid_1_Filter_ID>>24)|0x40;// 返回透传数据的类型
		   Original_info[Original_info_Wr++]=(CAN_trans.canid_1_Filter_ID>>16);
		   Original_info[Original_info_Wr++]=(CAN_trans.canid_1_Filter_ID>>8);
		   Original_info[Original_info_Wr++]=CAN_trans.canid_1_Filter_ID;
		 */

		Original_info[Original_info_Wr++]	= ( CAN_trans.canid_1_ID_SdBUF[i] >> 24 ) | 0x40; // 返回透传数据的类型
		Original_info[Original_info_Wr++]	= ( CAN_trans.canid_1_ID_SdBUF[i] >> 16 );
		Original_info[Original_info_Wr++]	= ( CAN_trans.canid_1_ID_SdBUF[i] >> 8 );
		Original_info[Original_info_Wr++]	= CAN_trans.canid_1_ID_SdBUF[i];

		//--------------------------------------------------------------------------
		memcpy( Original_info + Original_info_Wr, CAN_trans.canid_1_Sdbuf + read_rd, 8 );
		Original_info_Wr	+= 8;
		read_rd				+= 8;
	}
	CAN_trans.canid_1_SdWr = 0;
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	CAN 总线数据上传 数据项=%d   Rxnum=%d  Sd_num=%d  Can_loudiao=%d  Can_notsame=%d Same=%d\r\n", DataNum, Can_RXnum, Can_sdnum, Can_loudiao, Can_same, Can_notsame );
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_CentreTakeACK_BD_0805H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0805, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//   float ID
	Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );      //  应答流水号
	Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;
	Original_info[Original_info_Wr++]	= (u8)SingleCamra_TakeResualt_BD;   // 中心应答结果
	Original_info[Original_info_Wr++]	= 0x00;                             //  成功拍照多媒体个数    1
	Original_info[Original_info_Wr++]	= 1;
	Original_info[Original_info_Wr++]	= 0;                                // 多媒体ID 列表
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 0;
	Original_info[Original_info_Wr++]	= 1;
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	摄像头立即拍照应答 \r\n");
	}
}

//--------------------------------------------------------------------------
u8  Stuff_EventACK_0301H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0301, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	Original_info[Original_info_Wr++] = EventObj.Event_ID; // 返回事件ID

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 事件结果返回  \r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_ASKACK_0302H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0302, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//  应答流水号
	Original_info[Original_info_Wr++]	= ( ASK_Centre.ASK_floatID >> 8 ); // 返回事件ID
	Original_info[Original_info_Wr++]	= ASK_Centre.ASK_floatID;
	Original_info[Original_info_Wr++]	= ASK_Centre.ASK_answerID;
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	发送提问选择结果 \r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_MSGACK_0303H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0303, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//  应答流水号
	Original_info[Original_info_Wr++]	= MSG_BroadCast_Obj.INFO_TYPE;
	Original_info[Original_info_Wr++]	= MSG_BroadCast_Obj.INFO_PlyCancel; //  0  取消  1 点播
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	点播取消回复  \r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Stuff_ControlACK_0500H( void )   //   车辆控制应答
{
	//  1. Head
	if( !Protocol_Head( MSG_0x0500, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//------------------------------- Stuff ----------------------------------------
	//   float ID                                                // 对应中心应答消息的流水号
	Original_info[Original_info_Wr++]	= (u8)( Vech_Control.CMD_FloatID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Vech_Control.CMD_FloatID;

	// 1. 告警标志  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
	Original_info_Wr += 4;
	// 2. 状态  4
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
	Original_info_Wr += 4;
	// 3.  纬度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Latitude, 4 );     //纬度   modify by nathan
	Original_info_Wr += 4;
	// 4.  经度
	memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Longitude, 4 );    //经度    东经  Bit 7->0   西经 Bit 7 -> 1
	Original_info_Wr += 4;
	// 5.  高程
	Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
	Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
	// 6.  速度    0.1 Km/h
	Original_info[Original_info_Wr++]	= (u8)( Speed_gps >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Speed_gps;
	// 7. 方向   单位 1度
	Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );   //High
	Original_info[Original_info_Wr++]	= GPS_direction;            // Low
	// 8.  日期时间
	Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	SEND  Vech  Control  ! \r\n");
	}

	return true;
}

//  行车记录仪 :采集指定的位置信息记录
u8  REC_09H_Stuff( void )
{
	u8 i = 0;
#if 0
	// 1.  最近1 小时的的位置信息记录
	// 1.1   开始时间
	Time2BCD( Original_info + Original_info_Wr ); // 记录仪实时时间
	Original_info_Wr += 6;
	for( i = 0; i < 60; i++ )
	{
		//  1.2  开始后第一分钟的位置
		//  longitude    3905.292651,N,11733.124913,E        long: 043438B1  lat: 0165DCFE
		Original_info[Original_info_Wr++]	= 0x04;
		Original_info[Original_info_Wr++]	= 0x34;
		Original_info[Original_info_Wr++]	= 0x38;
		Original_info[Original_info_Wr++]	= 0xB1;
		// lat
		Original_info[Original_info_Wr++]	= 0x01;
		Original_info[Original_info_Wr++]	= 0x65;
		Original_info[Original_info_Wr++]	= 0xDC;
		Original_info[Original_info_Wr++]	= 0xFE;
		// high
		Original_info[Original_info_Wr++]	= 0x00;
		Original_info[Original_info_Wr++]	= 0x65;
		// spd
		Original_info[Original_info_Wr++] = 0x00;
	}
#endif
}



/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:  wwww
***********************************************************/
u8  Stuff_RecoderACK_0700H( u8 PaketType )  //   行车记录仪数据上传
{
	u16 SregLen				= 0, Swr = 0;   //,Gwr=0; // S:serial  G: GPRS
	u16 Reg_len_position	= 0;
	u8	Sfcs				= 0;
	u16 i					= 0;
	u32 regdis				= 0, reg2 = 0;
	u8	Reg[70];
	u8	QueryRecNum = 0;                    // 查询记录数目

	//  1. Head
	if( !Protocol_Head( MSG_0x0700, PaketType ) )
	{
		return false;
	}

	switch( Recode_Obj.CMD )
	{
		case   0x00:                                                //  执行标准版本年号
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;   // 命令字
			Swr									= Original_info_Wr;

			Original_info[Original_info_Wr++]	= 0x55;             // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;
			Original_info[Original_info_Wr++]	= 0x00;             //命令字
			SregLen								= 0x02;             // 信息长度
			Original_info[Original_info_Wr++]	= 0x00;             // Hi
			Original_info[Original_info_Wr++]	= 2;                // Lo

			Original_info[Original_info_Wr++]	= 0x00;             // 保留字
			Original_info[Original_info_Wr++]	= 0x12;             //  12  年标准 
			Original_info[Original_info_Wr++]	= 0x00;

			break;
		//---------------- 上传数类型  -------------------------------------
		case 0x01:
			//  01  当前驾驶员代码及对应的机动车驾驶证号
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;               // 命令字
			Swr									= Original_info_Wr;

			Original_info[Original_info_Wr++]	= 0x55;                         // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;
			Original_info[Original_info_Wr++]	= 0x01;                         //命令字

			SregLen								= 0x00;                         // 信息长度
			Original_info[Original_info_Wr++]	= 0x00;                         // Hi
			Original_info[Original_info_Wr++]	= 18;                           // Lo

			Original_info[Original_info_Wr++] = 0x00;                           // 保留字

			memcpy( Original_info + Original_info_Wr, "410727198503294436", 18 );  //信息内容
			Original_info_Wr					+= 18;
			/*
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			*/
			break;
		case 0x02:                                                              //  02  采集记录仪的实时时钟
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;               // 命令字
			Swr									= Original_info_Wr;

			Original_info[Original_info_Wr++]	= 0x55;                         // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;
			Original_info[Original_info_Wr++]	= 0x02;                         //命令字

			SregLen								= 0x00;                         // 信息长度
			Original_info[Original_info_Wr++]	= 0x00;                         // Hi
			Original_info[Original_info_Wr++]	= 6;                            // Lo

			Original_info[Original_info_Wr++] = 0x00;                           // 保留字
			Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );
			//+++++++++++++++++++++++++++++++++++++++++++++shi jian ++++++++++++++++++++++
			t_hour = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
			t_min = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
			t_second = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );
			/*
			Original_info[Original_info_Wr++] = 0x14;
			Original_info[Original_info_Wr++] = 0x03;
			Original_info[Original_info_Wr++] = 0x20;
			Original_info[Original_info_Wr++] =	0x08;
			Original_info[Original_info_Wr++] = 0x17;
			Original_info[Original_info_Wr++] = 0x30;
			*/
			/*
			Time2BCD( Original_info + Original_info_Wr );                       //记录仪时间
			Original_info_Wr += 6;
			*/
			break;
		case 0x03:                                                              // 03 采集360h内里程
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;               // 命令字
			Swr									= Original_info_Wr;

			Original_info[Original_info_Wr++]	= 0x55;                         // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;
			Original_info[Original_info_Wr++]	= 0x03;                         //命令字

			SregLen								= 0x00;                         // 信息长度
			Original_info[Original_info_Wr++]	= 0x00;                         // Hi
			Original_info[Original_info_Wr++]	= 20;                           // Lo

			Original_info[Original_info_Wr++] = 0x00;                           // 保留字
			//	信息内容
			/*
			Time2BCD( Original_info + Original_info_Wr );                       // 记录仪实时时间
			Original_info_Wr					+= 6;
			*/
			Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
			Original_info[Original_info_Wr++]	= t_hour;
			Original_info[Original_info_Wr++]	= t_min;
			Original_info[Original_info_Wr++]	= t_second;
			/*
			Original_info[Original_info_Wr++] = 0x14;
			Original_info[Original_info_Wr++] = 0x03;
			Original_info[Original_info_Wr++] = 0x20;
			Original_info[Original_info_Wr++] =	0x08;
			Original_info[Original_info_Wr++] = 0x17;
			Original_info[Original_info_Wr++] = 0x30;
			*/
			
			Original_info[Original_info_Wr++]	= 0x13;                         // 初次安装时间
			Original_info[Original_info_Wr++]	= 0x04;
			Original_info[Original_info_Wr++]	= 0x01;
			Original_info[Original_info_Wr++]	= 0x08;
			Original_info[Original_info_Wr++]	= 0x30;
			Original_info[Original_info_Wr++]	= 0x26;
			//--- 初始里程
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			// -- 累积里程 3个字节 单位0.1km    6位
			regdis								= JT808Conf_struct.Distance_m_u32 / 100; //单位0.1km
			reg2								= regdis / 100000;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= ( reg2 << 4 ) + ( regdis % 100000 / 10000 );
			Original_info[Original_info_Wr++]	= ( ( regdis % 10000 / 1000 ) << 4 ) + ( regdis % 1000 / 100 );
			Original_info[Original_info_Wr++]	= ( ( regdis % 100 / 10 ) << 4 ) + ( regdis % 10 );
			break;

		case 0x04:                                                                  // 04  采集记录仪脉冲系数
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;                   // 命令字
			Swr									= Original_info_Wr;
			Original_info[Original_info_Wr++]	= 0x55;                             // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x04;                               //命令字

			SregLen								= 0x00;                             // 信息长度
			Original_info[Original_info_Wr++]	= 0x00;                             // Hi
			Original_info[Original_info_Wr++]	= 8;                                // Lo

			Original_info[Original_info_Wr++] = 0x00;                               // 保留字

			//  信息内容
			/*
			Time2BCD( Original_info + Original_info_Wr );                           // 记录仪实时时间
			Original_info_Wr					+= 6;
			*/
			Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
			Original_info[Original_info_Wr++]	= t_hour;
			Original_info[Original_info_Wr++]	= t_min;
			Original_info[Original_info_Wr++]	= t_second;
			/*
			Original_info[Original_info_Wr++] = 0x14;
			Original_info[Original_info_Wr++] = 0x03;
			Original_info[Original_info_Wr++] = 0x20;
			Original_info[Original_info_Wr++] =	0x08;
			Original_info[Original_info_Wr++] = 0x17;
			Original_info[Original_info_Wr++] = 0x30;
			*/
			
			Original_info[Original_info_Wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value >> 8 );
			Original_info[Original_info_Wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value );
			break;
		case 0x05:                                                                  //05      车辆信息采集
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;                   // 命令字
			Swr									= Original_info_Wr;
			Original_info[Original_info_Wr++]	= 0x55;                             // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x05;                               //命令字

			SregLen								= 41;                               // 信息长度
			Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );             // Hi
			Original_info[Original_info_Wr++]	= (u8)SregLen;                      // Lo	  65x7

			Original_info[Original_info_Wr++] = 0x00;                               // 保留字
			//-----------	信息内容  --------------
			memcpy( Original_info + Original_info_Wr, "LGBK22E70AY100102", 17 );    //信息内容
			Original_info_Wr += 17;
			memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Vechicle_Info.Vech_Num, 8 );               // 车牌号
			Original_info_Wr					+= 7;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			//车辆类型
			/*Original_info[Original_info_Wr++]	= 0x03; //小型车
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;*/
			memcpy( Original_info + Original_info_Wr, "小型车", 6 );   
			Original_info_Wr+=6;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;  
			
			break;
		case   0x06:                                                    // 06-09
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // 命令字
			Swr									= Original_info_Wr;
			//  06 信号配置信息
			Original_info[Original_info_Wr++]	= 0x55;                 // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x06;                   //命令字

			SregLen								= 87;                   // 信息长度
			Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
			Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo

			Original_info[Original_info_Wr++] = 0x00;                   // 保留字

			Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
			Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
			Original_info[Original_info_Wr++]	= t_hour;
			Original_info[Original_info_Wr++]	= t_min;
			Original_info[Original_info_Wr++]	= t_second;
			/*
			Time2BCD( Original_info + Original_info_Wr );               //记录仪实时时间
			Original_info_Wr += 6;
			*/
			//-------  状态字个数----------------------
			Original_info[Original_info_Wr++] = 0;// 8//修改为0
			//---------- 状态字内容-------------------


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
			memcpy( Original_info + Original_info_Wr, "预留      ", 10 );       // D0
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "预留      ", 10 );       // D1
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "预留      ", 10 );       // D2
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "近光灯    ", 10 );       // D3
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "远光灯    ", 10 );       // D4
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "右转灯    ", 10 );       // D5
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "左转灯    ", 10 );       // D6
			Original_info_Wr += 10;
			memcpy( Original_info + Original_info_Wr, "刹车      ", 10 );       // D7
			Original_info_Wr += 10;
			break;

		case 0x07:                                                              //07  记录仪唯一编号
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;               // 命令字
			Swr									= Original_info_Wr;

			Original_info[Original_info_Wr++]	= 0x55;                         // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x07;                           //命令字

			SregLen								= 30;                           //206;		 // 信息长度
			Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );         // Hi
			Original_info[Original_info_Wr++]	= (u8)SregLen;                  // Lo

			Original_info[Original_info_Wr++] = 0x00;                           // 保留字
			//------- 信息内容 ------
			memcpy( Original_info + Original_info_Wr, "7654321", 7 );           // 3C 认证代码
			Original_info_Wr += 7;
			memcpy( Original_info + Original_info_Wr, "TW705   TW705   ", 16 ); // 产品型号
			Original_info_Wr					+= 16;
			Original_info[Original_info_Wr++]	= 0x13;                         // 生产日期
			Original_info[Original_info_Wr++]	= 0x03;
			Original_info[Original_info_Wr++]	= 0x01;
			Original_info[Original_info_Wr++]	= 0x00;                         // 生产流水号
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= 0x00;
			Original_info[Original_info_Wr++]	= IMSI_CODE[14] - 0x30;
			break;

		case 0x08:                                                              //  08   采集指定的行驶速度记录
			if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // 命令字
				Swr									= Original_info_Wr;

				Original_info[Original_info_Wr++]	= 0x55;                     // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;
				Original_info[Original_info_Wr++]	= 0x08;                     //命令字

				SregLen								= 504;//630;                      // 信息长度       630
				Original_info[Original_info_Wr++]	= SregLen >> 8;             // Hi
				Original_info[Original_info_Wr++]	= SregLen;                  // Lo

				Original_info[Original_info_Wr++] = 0x00;                       // 保留字
			}
			//	信息内容
			//WatchDog_Feed( );
			get_08h( Original_info + Original_info_Wr,Recode_Obj.Current_pkt_num);                        //126*5=630        num=576  packet

			Original_info_Wr += 504; //630;
			//  后续需要分包处理  -----nate
			break;
		case   0x09:                                                            // 09   指定的位置信息记录
			if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // 命令字
				Swr									= Original_info_Wr;
				Original_info[Original_info_Wr++]	= 0x55;                     // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;

				Original_info[Original_info_Wr++] = 0x09;                       //命令字

				SregLen								= 666 * 2;                  // 信息长度
				Original_info[Original_info_Wr++]	= SregLen >> 8;             // Hi      666=0x29A
				Original_info[Original_info_Wr++]	= SregLen;                  // Lo

				//Original_info[Original_info_Wr++]=0;    // Hi      666=0x29A
				//Original_info[Original_info_Wr++]=0;	   // Lo

				Original_info[Original_info_Wr++] = 0x00;                       // 保留字
			}
			//	信息内容
			//WatchDog_Feed( );
			if(Recode_Obj.Current_pkt_num%2) 
			{    
			  get_09h(_700H_buffer,Recode_Obj.Current_pkt_num);
			  memcpy(Original_info + Original_info_Wr,_700H_buffer,333);
			}
			else
               memcpy(Original_info + Original_info_Wr,_700H_buffer+333,333); 
			
			Original_info_Wr += 333;
			break;
		case   0x10:                                                            // 10-13     10   事故疑点采集记录
			//事故疑点数据
			if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // 命令字
				Swr									= Original_info_Wr;

				Original_info[Original_info_Wr++]	= 0x55;                     // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;

				Original_info[Original_info_Wr++] = 0x10;                       //命令字

				SregLen								= 234 * 100;                //0		 // 信息长度
				Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
				Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo

				Original_info[Original_info_Wr++] = 0x00;                       // 保留字
			}
			//------- 信息内容 ------
			//WatchDog_Feed( );
			delay_ms( 3 );
			get_10h( Original_info + Original_info_Wr );                        //234  packetsize      num=100
			Original_info_Wr += 234;

			break;

		case  0x11:                                                             // 11 采集指定的的超时驾驶记录
			if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // 命令字
				Swr									= Original_info_Wr;

				Original_info[Original_info_Wr++]	= 0x55;                     // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;

				Original_info[Original_info_Wr++] = 0x11;                       //命令字

				SregLen								= 500 * 10;                 // 信息长度
				Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
				Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo    65x7

				Original_info[Original_info_Wr++] = 0x00;                       // 保留字
			}


			/*
			       每条 50 bytes  ，100 条    获取的是每10 条打一包  500 packet    Totalnum=10
			 */
			 WatchDog_Feed( );
			get_11h( Original_info + Original_info_Wr );                        //50  packetsize      num=100
			Original_info_Wr += 50;
			break;
		case  0x12:                                                             // 12 采集指定驾驶人身份记录  ---Devide
			if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // 命令字
				Swr									= Original_info_Wr;

				Original_info[Original_info_Wr++]	= 0x55;                     // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;

				Original_info[Original_info_Wr++] = 0x12;                       //命令字

				SregLen								= 50 * 100;                 // 信息长度
				Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
				Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo    65x7

				Original_info[Original_info_Wr++] = 0x00;                       // 保留字
			}
			//------- 信息内容 ------


			/*
			        驾驶员身份登录记录  每条25 bytes      200条      20条一包 500 packetsize  totalnum=10
			 */
			 WatchDog_Feed( );
			get_12h( Original_info + Original_info_Wr );
			Original_info_Wr += 500;
			break;
		case  0x13:                                                     // 13 采集记录仪外部供电记录
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // 命令字
			Swr									= Original_info_Wr;
			Original_info[Original_info_Wr++]	= 0x55;                 // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x13;                   //命令字

			SregLen								= 700;                  // 信息长度
			Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
			Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

			Original_info[Original_info_Wr++] = 0x00;                   // 保留字
			//------- 信息内容 ------


			/*
			   外部供电记录   7 个字节  100 条       一个完整包
			 */
			 WatchDog_Feed( );
			get_13h( Original_info + Original_info_Wr );
			Original_info_Wr += 700;

			break;
		case   0x14:
			Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
			Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // 命令字
			Swr									= Original_info_Wr;
			// 14 记录仪参数修改记录
			Original_info[Original_info_Wr++]	= 0x55;                 // 起始头
			Original_info[Original_info_Wr++]	= 0x7A;

			Original_info[Original_info_Wr++] = 0x14;                   //命令字

			SregLen								= 700;                  // 信息长度
			Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
			Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

			Original_info[Original_info_Wr++] = 0x00;                   // 保留字
			//------- 信息内容 ------


			/*
			       每条 7 个字节   100 条    1个完整包
			 */
			 WatchDog_Feed( );
			get_14h( Original_info + Original_info_Wr );
			Original_info_Wr += 700;
			break;

		case     0x15:                                                      // 15 采集指定的速度状态日志     --------Divde
			//if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
			if( ( Recode_Obj.Current_pkt_num == 1 ) )
			{
				Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
				Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
				Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // 命令字
				Swr									= Original_info_Wr;
				Original_info[Original_info_Wr++]	= 0x55;                 // 起始头
				Original_info[Original_info_Wr++]	= 0x7A;

				Original_info[Original_info_Wr++] = 0x15;                   //命令字

				SregLen								= 133 * 10;             // 信息长度
				Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
				Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

				Original_info[Original_info_Wr++] = 0x00;                   // 保留字
			}


			/*
			       每条 133 个字节   10 条    1个完整包     5*133=665        totalnum=2
			 */
			WatchDog_Feed( );
			get_15h( Original_info + Original_info_Wr );
			Original_info_Wr += 133;

			break;
	}
	//---------------  填写计算 A 协议	Serial Data   校验位  -------------------------------------

	Sfcs = 0;                            //  计算S校验 从Ox55 开始
	for( i = Swr; i < Original_info_Wr; i++ )
	{
		Sfcs ^= Original_info[i];
	}
	//Original_info[Original_info_Wr++] = Sfcs;               // 填写FCS

/*bitter:最后一包发送fcs*/
#if 1
	if( PaketType == Packet_Divide )
	{
		Recode_Obj.fcs ^= Sfcs;
		if( Recode_Obj.Current_pkt_num == Recode_Obj.Total_pkt_num )
		{
			Original_info[Original_info_Wr++] = Recode_Obj.fcs; // 填写FCS
		}
	}else
	{
		Original_info[Original_info_Wr++] = Sfcs;               // 填写FCS
	}
#endif

	//  3. Send
	Protocol_End( PaketType, 0 );

	//  4.     如果是分包 判断结束
	if(( Recode_Obj.Devide_Flag == 1 )&&(Recode_Obj.RSD_State==0))   // 不给应答 ,非列表重传状态下进行
	{
		if( Recode_Obj.Current_pkt_num >= Recode_Obj.Total_pkt_num )
		{
			Recorder_init(1);           //  clear
		}else
		{
			Recode_Obj.Current_pkt_num++;
		}
	}

	if( DispContent )
	{
		rt_kprintf( "\r\n	SEND Recorder Data ! \r\n");
	}

	return true;
}

//------------------------------------------------------------------

u8  Stuff_GNSSRawData_0900H( u8*Instr, u16 len )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0900, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	Original_info[Original_info_Wr++] = 0x00; // 返回透传数据的类型
	memcpy( Original_info + Original_info_Wr, Instr, len );
	Original_info_Wr += len;
	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	发送透传  \r\n");
	}
	return true;
}

//----------------------------------------------------------------------
u8  Stuff_MultiMedia_InfoSD_0800H( void )
{
	// 1. Head
	if( !Protocol_Head( MSG_0x0800, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	switch( MediaObj.Media_Type )
	{
		case 0:                                                                 // 图像
			MediaObj.Media_totalPacketNum	= Photo_sdState.Total_packetNum;    // 图片总包数
			MediaObj.Media_currentPacketNum = Photo_sdState.SD_packetNum;       // 图片当前报数
			MediaObj.Media_ID				= 1;                                //  多媒体ID
			MediaObj.Media_Channel			= Camera_Number;                    // 图片摄像头通道号
			break;
		case 1:                                                                 // 音频

			MediaObj.Media_totalPacketNum	= Sound_sdState.Total_packetNum;    // 图片总包数
			MediaObj.Media_currentPacketNum = Sound_sdState.SD_packetNum;       // 图片当前报数
			MediaObj.Media_ID				= 1;                                //  多媒体ID
			MediaObj.Media_Channel			= 1;                                // 图片摄像头通道号
			// rt_kprintf(" \r\n 申请上传音频信息 \r\n");
			break;
		case 2:                                                                 // 视频
			MediaObj.Media_totalPacketNum	= Video_sdState.Total_packetNum;    // 图片总包数
			MediaObj.Media_currentPacketNum = Video_sdState.SD_packetNum;       // 图片当前报数
			MediaObj.Media_ID				= 1;                                //  多媒体ID
			MediaObj.Media_Channel			= 1;                                // 图片摄像头通道号
			// rt_kprintf(" \r\n 申请上传视频信息 \r\n");

			break;
		default:
			return false;
	}

	//  MediaID
	Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 24 ); // 返回事件ID
	Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 16 );
	Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 8 );
	Original_info[Original_info_Wr++]	= MediaObj.Media_ID;
	//  Type
	Original_info[Original_info_Wr++] = MediaObj.Media_Type;
	//  MediaCode Type
	Original_info[Original_info_Wr++]	= MediaObj.Media_CodeType;
	Original_info[Original_info_Wr++]	= MediaObj.Event_Code;
	Original_info[Original_info_Wr++]	= MediaObj.Media_Channel;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	发送多媒体事件信息上传  \r\n");
	}
	return true;
}

//--------------------------------------------------------------------------
u8  Stuff_MultiMedia_Data_0801H( void )
{
	u16 inadd = 0, readsize = 0; //,soundpage=0,sounddelta=0;
//	u8  instr[SpxGet_Size];

	//  rt_kprintf("\r\n  1--- pic_total_num:  %d	current_num:  %d  MediaObj.Media_Type: %d \r\n ",MediaObj.Media_totalPacketNum,MediaObj.Media_currentPacketNum,MediaObj.Media_Type);
	// 1. Head
	if( !Protocol_Head( MSG_0x0801, Packet_Divide ) )
	{
		return false;
	}
	// 2. content1  ==>  MediaHead
	if( MediaObj.Media_currentPacketNum == 1 )
	{
		//  MediaID
		Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 24 );  //  多媒体ID
		Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 16 );
		Original_info[Original_info_Wr++]	= ( MediaObj.Media_ID >> 8 );
		Original_info[Original_info_Wr++]	= MediaObj.Media_ID;
		//  Type
		Original_info[Original_info_Wr++] = MediaObj.Media_Type;            // 多媒体类型
		//  MediaCode Type
		Original_info[Original_info_Wr++]	= MediaObj.Media_CodeType;      // 多媒体编码格式
		Original_info[Original_info_Wr++]	= MediaObj.Event_Code;          // 多媒体事件编码
		Original_info[Original_info_Wr++]	= MediaObj.Media_Channel;       // 通道ID

		//  Position Inifo
		//  告警标志  4
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Warn_Status, 4 );
		Original_info_Wr += 4;
		// . 状态  4
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Car_Status, 4 );
		Original_info_Wr += 4;
		//   纬度
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Latitude, 4 );     //纬度   modify by nathan
		Original_info_Wr += 4;
		//   经度
		memcpy( ( char* )Original_info + Original_info_Wr, ( char* )Gps_Gprs.Longitude, 4 );    //经度    东经  Bit 7->0   西经 Bit 7 -> 1
		Original_info_Wr += 4;
		//   高程
		Original_info[Original_info_Wr++]	= (u8)( GPS_Hight << 8 );
		Original_info[Original_info_Wr++]	= (u8)GPS_Hight;
		//   速度    0.1 Km/h
		Original_info[Original_info_Wr++]	= (u8)( Speed_gps >> 8 );                           //(GPS_speed>>8);
		Original_info[Original_info_Wr++]	= (u8)( Speed_gps );                                //GPS_speed;
		//   方向   单位 1度
		Original_info[Original_info_Wr++]	= ( GPS_direction >> 8 );                           //High
		Original_info[Original_info_Wr++]	= GPS_direction;                                    // Low
		//   日期时间
		Original_info[Original_info_Wr++]	= ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
		Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
		Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
		Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
		Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
		Original_info[Original_info_Wr++]	= ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );

		if( 1 == MediaObj.Media_Type ) //  音频头
		{
			//  AMR 文件头     23 21 41 4D 52 0A
			Original_info[Original_info_Wr++]	= 0x23;
			Original_info[Original_info_Wr++]	= 0x21;
			Original_info[Original_info_Wr++]	= 0x41;
			Original_info[Original_info_Wr++]	= 0x4D;
			Original_info[Original_info_Wr++]	= 0x52;
			Original_info[Original_info_Wr++]	= 0x0A;
		}
	}
	// 4. content3  ==> Media Info
	switch( MediaObj.Media_Type )
	{
		case 0:                                     // 图像

			if( ( ( Photo_sdState.photo_sending ) == enable ) && ( ( Photo_sdState.SD_flag ) == enable ) )
			{
				Photo_sdState.SD_flag = disable;    // clear
			}else
			{
				return false;
			}
			//  ---------------  填写内容  ---------------
			//			read		Photo_sdState.SD_packetNum从1开始计数
			//			content_startoffset     picpage_offset				 contentpage_offset
			if( TF_Card_Status( ) == 0 )
			{
				if( Camera_Number == 1 )
				{
					Api_DFdirectory_Read( camera_1, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum );
				} else
				if( Camera_Number == 2 )
				{
					Api_DFdirectory_Read( camera_2, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum );
				} else
				if( Camera_Number == 3 )
				{
					Api_DFdirectory_Read( camera_3, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum );
				} else
				if( Camera_Number == 4 )
				{
					Api_DFdirectory_Read( camera_4, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum );
				}

				inadd = ( Photo_sdState.SD_packetNum - 1 ) << 9; //乘以512
				if( PicFileSize > inadd )
				{
					if( ( PicFileSize - inadd ) > 512 )
					{
						readsize = 512;
					} else
					{
						readsize = PicFileSize - inadd; // 最后一包
						rt_kprintf( "\r\n   最后一包 readsize =%d \r\n", readsize );
					}
				}else
				{
					return false;
				}
			}else
			if( TF_Card_Status( ) == 1 )
			{
				;


				/* inadd=(Photo_sdState.SD_packetNum-1)<<9; //乘以512
				   if(PicFileSize>inadd)
				   {
				                   if((PicFileSize-inadd)>512)
				                        readsize=512;
				   else
				   {
				   readsize=PicFileSize-inadd; // 最后一包
				   rt_kprintf("\r\n   最后一包 readsize =%d \r\n",readsize);
				   }
				   }
				   else
				   return false;
				             i=read_file(PictureName,inadd,readsize,Original_info + Original_info_Wr);
				   if(i==false)
				   {
				                 rt_kprintf("\r\n 图片文件: %s   读取失败\r\n",PictureName);
				                 return false;
				   } */
			}
			Original_info_Wr += readsize;           //
			break;
		case 1:                                     // 音频
			if( ( ( Sound_sdState.photo_sending ) == enable ) && ( ( Sound_sdState.SD_flag ) == enable ) )
			{
				Sound_sdState.SD_flag = disable;    // clear
			}else
			{
				return false;
			}
			//------------------------------------------------------------------------
			//  ---------------  填写内容  ---------------
			//			read		Photo_sdState.SD_packetNum从1开始计数
			//			content_startoffset     picpage_offset				 contentpage_offset
			if( TF_Card_Status( ) == 0 )
			{
				Api_DFdirectory_Read( voice, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum );
				inadd = ( Sound_sdState.SD_packetNum - 1 ) << 9; //乘以512
				if( SrcFileSize > inadd )
				{
					if( ( SrcFileSize - inadd ) > 512 )
					{
						readsize = 512;
					} else
					{
						readsize = SrcFileSize - inadd; // 最后一包
						rt_kprintf( "\r\n   最后一包 readsize =%d \r\n", readsize );
					}
				}else
				{
					return false;
				}
			}
			rt_kprintf( "\r\n Sound_sdState.SD_packetNum= %d   filesize=%d  readsize=%d  \r\n", Sound_sdState.SD_packetNum, SrcFileSize, SrcFileSize - inadd );
			Original_info_Wr += readsize;

			//-------------------------------------------------------------------------


			/*
			   //  ---------------	填写内容  ---------------
			   if(TF_Card_Status()==1)
			   {
			   if(mp3_sendstate==0)
			   {
			   if(Sound_sdState.SD_packetNum==1)
			   {  // wav tou

			   inadd=WaveFile_EncodeHeader(SrcFileSize ,Original_info + Original_info_Wr);
			   Original_info_Wr+=inadd;
			   rt_kprintf("\r\n 写入文件头大小为 wav fileheadersize=%d  \r\n",inadd);

			   }
			   //---------------------------------------------------------
			   soundpage=(Sound_sdState.SD_packetNum-1)/5;// 得到page
			   sounddelta=((Sound_sdState.SD_packetNum-1)%5)*SpxGet_Size; // 得到页内偏移
			              rt_kprintf("\r\n inadd=%d  soundpage =%d  inpageoffset=%d \r\n",inadd,soundpage,sounddelta);
			   //  i=read_file(SpxSrcName,(soundpage<<9),512,SpxBuf);
			   //  if(i==false)
			   // {
			   // rt_kprintf("\r\n spx文件: %s   读取失败--2\r\n",SpxSrcName);
			   //return false;
			   //}
			   Api_Config_read(voice, Sound_sdState.SD_packetNum, SpxBuf,500);
			   memcpy(instr,SpxBuf+sounddelta,SpxGet_Size);
			            //---------  spx Decode  5  包 ---------
			   speachDecode(instr, Original_info + Original_info_Wr);
			   Original_info_Wr+=160;
			   speachDecode(instr+20, Original_info + Original_info_Wr);
			   Original_info_Wr+=160;
			   speachDecode(instr+40, Original_info + Original_info_Wr);
			   Original_info_Wr+=160;
			   speachDecode(instr+60, Original_info + Original_info_Wr);
			   Original_info_Wr+=160;
			   speachDecode(instr+80, Original_info + Original_info_Wr);
			   Original_info_Wr+=160;
			   }
			   else
			   if(mp3_sendstate==1)
			   {
			   inadd=(Sound_sdState.SD_packetNum-1)<<9; //乘以512
			   if(mp3_fsize>inadd)
			   {
			   if((mp3_fsize-inadd)>512)
			    readsize=512;
			   else
			   {
			   readsize=mp3_fsize-inadd; // 最后一包
			   rt_kprintf("\r\n	 最后一包 mp3size =%d \r\n",readsize);
			   }
			   }
			   else
			   return false;
			   //rt_kprintf("\r\n 读取文件\r\n");
			   i=read_file(SpxSrcName,inadd,readsize,Original_info + Original_info_Wr);
			   //rt_kprintf("\r\n 读取文件完毕\r\n");
			   if(i==false)
			   {
			   rt_kprintf("\r\n mp3文件: %s	读取失败\r\n",SpxSrcName);
			   return false;
			   }
			   Original_info_Wr+=readsize;//


			   }
			   }
			       else
			   return false;
			 */
			break;
		case 2: // 视频
			if( TF_Card_Status( ) == 1 )
			{
				;


				/*	inadd=(Video_sdState.SD_packetNum-1)<<9; //乘以512
				   if(wmv_fsize>inadd)
				   {
				   if((wmv_fsize-inadd)>512)
				     readsize=512;
				   else
				    {
				    readsize=wmv_fsize-inadd; // 最后一包
				    rt_kprintf("\r\n	 最后一包 wmvsize =%d \r\n",readsize);
				    }
				   }
				   else
				   return false;
				   i=read_file(SpxSrcName,inadd,readsize,Original_info + Original_info_Wr);
				   if(i==false)
				   {
				   rt_kprintf("\r\n mp3文件: %s	读取失败\r\n",SpxSrcName);
				   return false;
				   }
				   Original_info_Wr+=readsize;
				 */
			}else
			{
				return false;
			}

			break;
		default:
			return false;
	}

	if( MediaObj.Media_currentPacketNum > MediaObj.Media_totalPacketNum )
	{
		return false;
	}
	//  5. Send
	Protocol_End( Packet_Divide, 0 );
	WatchDog_Feed( );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Send Media Data \r\n");
	}
	//  else
	{
		rt_kprintf( "\r\n pic_total_num:  %d   current_num:  %d   \r\n ", MediaObj.Media_totalPacketNum, MediaObj.Media_currentPacketNum );
		if( MediaObj.Media_currentPacketNum >= MediaObj.Media_totalPacketNum )
		{
			rt_kprintf( "\r\n Media 最后一个block\r\n" );

			if( 0 == MediaObj.RSD_State ) // 如果在顺序传输模式下，则改为停止状态,等待中心下重传
			{
				MediaObj.RSD_State	= 2;
				MediaObj.RSD_Timer	= 0;
			}
		}
	}
	//----------  累加发送报数 --------------------
	if( 0 == MediaObj.RSD_State )
	{
		if( MediaObj.Media_currentPacketNum < MediaObj.Media_totalPacketNum )
		{
			//  图片
			if( Photo_sdState.photo_sending == enable )
			{
				Photo_sdState.SD_packetNum++;
			}
			//  音频
			if( Sound_sdState.photo_sending == enable )
			{
				Sound_sdState.SD_packetNum++;
			}
			//视频
			if( Video_sdState.photo_sending == enable )
			{
				Video_sdState.SD_packetNum++;
			}
		}
	}


/*	 else
   if(1==MediaObj.RSD_State)
   {
    MediaObj.RSD_Reader++;
     rt_kprintf("\r\n	  MediaObj.RSD_Reader++  =%d\r\n",MediaObj.RSD_Reader);
    if(MediaObj.RSD_Reader==MediaObj.RSD_total)
         MediaObj.RSD_State=2; //  置位等待状态，等待着中心再发重传指令
   }
 */
	//----------  返回  -------------------
	return true;
}

//----------------------------------------------------------------------
u8  Stuff_MultiMedia_IndexAck_0802H( void )
{
	u16 totalNum	= 0, lenregwr = 0;
	u16 i			= 0;

	// 1. Head
	if( !Protocol_Head( MSG_0x0802, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//   float ID  应答流水号
	Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );
	Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;

	//------- 多媒体总项数 ----
	lenregwr							= Original_info_Wr;
	Original_info[Original_info_Wr++]	= (u8)( totalNum >> 8 );                    // 临时占上位置
	Original_info[Original_info_Wr++]	= (u8)totalNum;

	//----- 查找有效效位置 ----
	totalNum = 0;
	for( i = 0; i < 8; i++ )
	{
		if( SD_ACKflag.f_MediaIndexACK_0802H == 1 )                                 // 图像
		{
			Api_RecordNum_Read( pic_index, i, (u8*)&MediaIndex, sizeof( MediaIndex ) );
		}else
		if( SD_ACKflag.f_MediaIndexACK_0802H == 2 )                                 // 音频
		{
			Api_RecordNum_Read( voice_index, i, (u8*)&MediaIndex, sizeof( MediaIndex ) );
		}
		// rt_kprintf("\r\n Effective_Flag %d  f_QueryEventCode %d  EventCode %d  \r\n",MediaIndex.Effective_Flag,SD_ACKflag.f_QueryEventCode,MediaIndex.EventCode);
		if( ( MediaIndex.Effective_Flag == 1 ) && ( SD_ACKflag.f_QueryEventCode == MediaIndex.EventCode ) )
		{                                                                           //  查找有效的索引和相对应类型的索引
			Original_info[Original_info_Wr++]	= (u8)( MediaIndex.MediaID >> 24 ); //  多媒体ID dworrd
			Original_info[Original_info_Wr++]	= (u8)( MediaIndex.MediaID >> 16 );
			Original_info[Original_info_Wr++]	= (u8)( MediaIndex.MediaID >> 8 );
			Original_info[Original_info_Wr++]	= (u8)( MediaIndex.MediaID );
			Original_info[Original_info_Wr++]	= MediaIndex.Type;                  //  多媒体类型
			Original_info[Original_info_Wr++]	= MediaIndex.ID;                    //  通道
			Original_info[Original_info_Wr++]	= MediaIndex.EventCode;
			memcpy( Original_info + Original_info_Wr, MediaIndex.PosInfo, 28 );
			Original_info_Wr += 28;
			totalNum++;
		}
	}

	//---------   补上总项数  --------
	Original_info[lenregwr]		= (u8)( totalNum >> 8 );
	Original_info[lenregwr + 1] = totalNum;

	//  3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Send Media Index \r\n");
	}
	return true;
}

//--------------------------------------------------------------------------------------
u8  Stuff_DriverInfoSD_0702H( void )
{
	u8 i = 0;
	// 1. Head
	if( !Protocol_Head( MSG_0x0702, Packet_Normal ) )
	{
		return false;
	}

	// 2. content

#if  0                                                                                              // old  not   right


	/*Original_info[Original_info_Wr++]=JT808Conf_struct.Driver_Info.BD_IC_status;
	   memcpy(Original_info+Original_info_Wr,JT808Conf_struct.Driver_Info.BD_IC_inoutTime,6);
	   Original_info_Wr+=6;
	   Original_info[Original_info_Wr++]=JT808Conf_struct.Driver_Info.BD_IC_rd_res;   //卡片被拔出
	 */Original_info[Original_info_Wr++] = JT808Conf_struct.Driver_Info.BD_DriveName_Len; //  姓名长度
	memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_DriveName, JT808Conf_struct.Driver_Info.BD_DriveName_Len );
	Original_info_Wr += JT808Conf_struct.Driver_Info.BD_DriveName_Len;
	memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_Drv_CareerID, 20 );   //  从业资格证编码
	Original_info_Wr					+= 20;
	Original_info[Original_info_Wr++]	= JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len;
	memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_Confirm_agentID, JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len );
	Original_info_Wr += JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len;


	/* memcpy(Original_info+Original_info_Wr,JT808Conf_struct.Driver_Info.BD_ExpireDate,4);  //YYYYMMDD		Original_info_Wr+=i;
	   Original_info_Wr+=4;	*/
#endif

#if   1            //  new  BD format
	//  状态
	if( IC_MOD.IC_Status == 1 )
	{
		JT808Conf_struct.Driver_Info.BD_IC_status	= 0x01;
		Original_info[Original_info_Wr++]			= JT808Conf_struct.Driver_Info.BD_IC_status;
		memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_IC_inoutTime, 6 );
		Original_info_Wr					+= 6;
		Original_info[Original_info_Wr++]	= JT808Conf_struct.Driver_Info.BD_IC_rd_res;                    //卡片被拔出
		if( JT808Conf_struct.Driver_Info.BD_IC_rd_res == 0 )
		{
			Original_info[Original_info_Wr++] = JT808Conf_struct.Driver_Info.BD_DriveName_Len;              //	姓名长度
			memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_DriveName, JT808Conf_struct.Driver_Info.BD_DriveName_Len );
			Original_info_Wr += JT808Conf_struct.Driver_Info.BD_DriveName_Len;
			memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_Drv_CareerID, 20 );   //	从业资格证编码
			Original_info_Wr					+= 20;
			Original_info[Original_info_Wr++]	= JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len;
			memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_Confirm_agentID, JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len );
			Original_info_Wr += JT808Conf_struct.Driver_Info.BD_Confirm_agentID_Len;
			memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_ExpireDate, 4 );      //YYYYMMDD		Original_info_Wr+=i;
			Original_info_Wr += 4;
		}
	} else
	{
		JT808Conf_struct.Driver_Info.BD_IC_status	= 0x02;
		Original_info[Original_info_Wr++]			= JT808Conf_struct.Driver_Info.BD_IC_status;
		memcpy( Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.BD_IC_inoutTime, 6 );
		Original_info_Wr += 6;
	}
#endif

	// 3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Send Driver Info \r\n");
	}
	return true;
}

//---------------------------------------------------------------------------------
u8  Stuff_Worklist_0701H( void )
{
	u32 listlen = 215;
	// 1. Head
	if( !Protocol_Head( MSG_0x0701, Packet_Normal ) )
	{
		return false;
	}

	// 2. content
	//   信息长度
	listlen								= 207;
	Original_info[Original_info_Wr++]	= ( listlen >> 24 ); // 返回事件ID
	Original_info[Original_info_Wr++]	= ( listlen >> 16 );
	Original_info[Original_info_Wr++]	= ( listlen >> 8 );
	Original_info[Original_info_Wr++]	= listlen;

	memcpy( Original_info + Original_info_Wr, "托运单位:天津七一二通信广播有限公司 电话:022-26237216  ", 55 );
	Original_info_Wr += 55;
	memcpy( Original_info + Original_info_Wr, "承运单位:天津物流运输公司 电话:022-86692666  ", 45 );
	Original_info_Wr += 45;
	memcpy( Original_info + Original_info_Wr, "物品名称:GPS车载终端  包装方式:  箱式   每箱数量: 20   总量: 30箱  ", 67 );
	Original_info_Wr += 67;
	memcpy( Original_info + Original_info_Wr, "车型:箱式小货车 运达日期 :  2012-1-11   ", 40 );
	Original_info_Wr += 40;

	// 3. Send
	Protocol_End( Packet_Normal, 0 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	Send Worklist  \r\n");
	}
	return true;
}

//------------------------------------------------------------------------------------
u8 Stuff_DataTrans_0900_BD_GNSSData( void )
{
	u16 TX_NUM	= 0, Rec_Num = 0, i = 0;
	u8	Gfcs	= 0;
	//---------------------------------------------
	// 1. Head
	if( !Protocol_Head( MSG_0x0900, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//	应答流水号
	Original_info[Original_info_Wr++] = 0x00; // 返回透传数据的类型 1	表示远程下载
	// 3. 内容
	// GNSS 信息
	//	4. Send
	Protocol_End( Packet_Normal, 1 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 发送透传 GNSS Data\r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8 Stuff_DataTrans_0900_BD_ICinfo( void )
{
	u16 TX_NUM	= 0, Rec_Num = 0, i = 0;
	u8	Gfcs	= 0;
	//---------------------------------------------
	// 1. Head
	if( !Protocol_Head( MSG_0x0900, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//	应答流水号
	Original_info[Original_info_Wr++] = 0x0B;                           //IC 卡信息
	// 3. 内容
	// IC 信息
	memcpy( Original_info + Original_info_Wr, IC_MOD.IC_Tx40H, 64 );    //IC 64 字节信息
	Original_info_Wr += 64;
	//	4. Send
	Protocol_End( Packet_Normal, 1 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 发送透传IC 卡信息\r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8 Stuff_DataTrans_0900_BD_Serial1( void )
{
	u16 TX_NUM	= 0, Rec_Num = 0, i = 0;
	u8	Gfcs	= 0;
	//---------------------------------------------
	// 1. Head
	if( !Protocol_Head( MSG_0x0900, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//	应答流水号
	Original_info[Original_info_Wr++] = 0x41; // 串口1 透传消息
	// 3. 内容
	// 串口1 信息
	//	4. Send
	Protocol_End( Packet_Normal, 1 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 发送透传串口1信息\r\n");
	}
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8 Stuff_DataTrans_0900_BD_Serial2( void )
{
	u16 TX_NUM	= 0, Rec_Num = 0, i = 0;
	u8	Gfcs	= 0;
	//---------------------------------------------
	// 1. Head
	if( !Protocol_Head( MSG_0x0900, Packet_Normal ) )
	{
		return false;
	}
	// 2. content
	//	应答流水号
	Original_info[Original_info_Wr++] = 0x42; // 串口2 透传消息
	// 3. 内容
	// 串口1 信息
	//	4. Send
	Protocol_End( Packet_Normal, 1 );
	if( DispContent )
	{
		rt_kprintf( "\r\n	 发送透传串口2 信息\r\n");
	}
	return true;
}

//----------------------------------------------
u8  Update_HardSoft_Version_Judge( u8 * instr )
{
	//   读取第50 页信息
	Device_type		= ( (u32)instr[1] << 24 ) + ( (u32)instr[2] << 16 ) + ( (u32)instr[3] << 8 ) + (u32)instr[4];
	Firmware_ver	= ( (u32)instr[5] << 24 ) + ( (u32)instr[6] << 16 ) + ( (u32)instr[7] << 8 ) + (u32)instr[8];
	rt_kprintf( "	\r\n 设备类型: %x  软件版本:%x \r\n", Device_type, Firmware_ver );

	if( Device_type != STM32F407_Recoder_32MbitDF )
	{
		rt_kprintf( "\r\n 设备类型不匹配不予更新" );
		return false;
	}    else
	{
		return true;
	}
}

//-------------------- ISP Check  ---------------------------------------------
void  ISP_file_Check( void )
{
	memset( ISP_buffer, 0, sizeof( ISP_buffer ) );
	ISP_Read( ISP_APP_Addr, ISP_buffer, PageSIZE );


	/*
	   序号   字节数	名称			  备注
	   1          1    更新标志      1 表示需要更新   0 表示不需要更新
	   2-5			  4   设备类型				 0x0000 0001  ST712   TWA1
	        0x0000 0002   STM32  103  新A1
	        0x0000 0003   STM32  101  简易型
	        0x0000 0004   STM32  A3  sst25
	        0x0000 0005   STM32  行车记录仪
	   6-9		 4	   软件版本      每个设备类型从  0x0000 00001 开始根据版本依次递增
	   10-29      20	日期		' mm-dd-yyyy HH:MM:SS'
	   30-31      2    总页数		   不包括信息页
	   32-35      4    程序入口地址
	   36-200	   165	  预留
	   201-		  n    文件名

	 */
	//------------   Type check  ---------------------
	Device_type		= ( (u32)ISP_buffer[1] << 24 ) + ( (u32)ISP_buffer[2] << 16 ) + ( (u32)ISP_buffer[3] << 8 ) + (u32)ISP_buffer[4];
	Firmware_ver	= ( (u32)ISP_buffer[5] << 24 ) + ( (u32)ISP_buffer[6] << 16 ) + ( (u32)ISP_buffer[7] << 8 ) + (u32)ISP_buffer[8];
	rt_kprintf( "	\r\n 设备类型: %x  软件版本:%x \r\n", Device_type, Firmware_ver );

	if( Device_type != STM32F407_Recoder_32MbitDF )
	{
		rt_kprintf( "\r\n 设备类型不匹配不予更新" );
		ISP_buffer[0] = 0; // 不更新下载的程序
		ISP_Write( ISP_APP_Addr, ISP_buffer, PageSIZE );
	}

	rt_kprintf( "\r\n 文件日期: " );
	rt_kprintf( "%20s", (const char*)( ISP_buffer + 10 ) );
	rt_kprintf( "\r\n" );
	rt_kprintf( "\r\n 文件名: " );
	rt_kprintf( "%100s", (const char*)( ISP_buffer + 201 ) );
	rt_kprintf( "\r\n" );
	if( Device_type == STM32F407_Recoder_32MbitDF )
	{
		Systerm_Reset_counter	= ( Max_SystemCounter - 5 );    // 准备重启更新最新程序
		ISP_resetFlag			= 1;                            //准备重启
		rt_kprintf( "\r\n 准备重启更新程序!\r\n" );
	}
}

//----------------------------------------------------------------------------------
void Stuff_O200_Info_Only( u8* Instr )
{
	u8 Infowr = 0;

	// 1. 告警标志  4
	memcpy( ( char* )Instr + Infowr, ( char* )Warn_Status, 4 );
	Infowr += 4;
	// 2. 状态  4
	memcpy( ( char* )Instr + Infowr, ( char* )Car_Status, 4 );
	Infowr += 4;
	// 3.  纬度
	memcpy( ( char* )Instr + Infowr, ( char* )Gps_Gprs.Latitude, 4 );   //纬度   modify by nathan
	Infowr += 4;
	// 4.  经度
	memcpy( ( char* )Instr + Infowr, ( char* )Gps_Gprs.Longitude, 4 );  //经度    东经  Bit 7->0   西经 Bit 7 -> 1
	Infowr += 4;
	// 5.  高程
	Instr[Infowr++] = (u8)( GPS_Hight << 8 );
	Instr[Infowr++] = (u8)GPS_Hight;
	// 6.  速度    0.1 Km/h
	Instr[Infowr++] = (u8)( Speed_gps >> 8 );
	Instr[Infowr++] = (u8)Speed_gps;
	// 7. 方向   单位 1度
	Instr[Infowr++] = ( GPS_direction >> 8 );   //High
	Instr[Infowr++] = GPS_direction;            // Low
	// 8.  日期时间
	Instr[Infowr++] = ( ( ( Gps_Gprs.Date[0] ) / 10 ) << 4 ) + ( ( Gps_Gprs.Date[0] ) % 10 );
	Instr[Infowr++] = ( ( Gps_Gprs.Date[1] / 10 ) << 4 ) + ( Gps_Gprs.Date[1] % 10 );
	Instr[Infowr++] = ( ( Gps_Gprs.Date[2] / 10 ) << 4 ) + ( Gps_Gprs.Date[2] % 10 );
	Instr[Infowr++] = ( ( Gps_Gprs.Time[0] / 10 ) << 4 ) + ( Gps_Gprs.Time[0] % 10 );
	Instr[Infowr++] = ( ( Gps_Gprs.Time[1] / 10 ) << 4 ) + ( Gps_Gprs.Time[1] % 10 );
	Instr[Infowr++] = ( ( Gps_Gprs.Time[2] / 10 ) << 4 ) + ( Gps_Gprs.Time[2] % 10 );
}

//-----------------------------------------------------
u8  Save_MediaIndex( u8 type, u8* name, u8 ID, u8 Evencode )
{
	u8 i = 0;

	if( ( type != 1 ) && ( type != 0 ) )
	{
		return false;
	}

	//----- 查找无效位置 ----
	for( i = 0; i < 8; i++ )
	{
		if( type == 0 ) // 图像
		{
			Api_RecordNum_Read( pic_index, i, (u8*)&MediaIndex, sizeof( MediaIndex ) );
		}else
		if( type == 1 ) // 音频
		{
			Api_RecordNum_Read( voice_index, 1, (u8*)&MediaIndex, sizeof( MediaIndex ) );
		}
		if( MediaIndex.Effective_Flag == 0 )
		{
			break;
		}
	}
	if( i == 8 ) // 如果都满了则从第一个开始
	{
		i = 0;
	}
	//----  填写信息 -------------
	memset( (u8*)&MediaIndex, 0, sizeof( MediaIndex ) );
	MediaIndex.MediaID			= JT808Conf_struct.Msg_Float_ID;
	MediaIndex.Type				= type;
	MediaIndex.ID				= ID;
	MediaIndex.Effective_Flag	= 1;
	MediaIndex.EventCode		= Evencode;
	memcpy( MediaIndex.FileName, name, strlen( (const char*)name ) );
	Stuff_O200_Info_Only( MediaIndex.PosInfo );

	if( type == 0 ) // 图像
	{
		Api_RecordNum_Write( pic_index, i, (u8*)&MediaIndex, sizeof( MediaIndex ) );
	}else
	if( type == 1 ) // 音频
	{
		Api_RecordNum_Write( voice_index, i, (u8*)&MediaIndex, sizeof( MediaIndex ) );
	}
	return true;
}

//------------------------------------------------------------------
u8  CentreSet_subService_8103H( u32 SubID, u8 infolen, u8 *Content )
{
	u8	i = 0;
	u8	reg_str[80];
	u8	reg_in[20];
	u32 resualtu32 = 0;

	rt_kprintf( "\r\n    收到中心设置命令 SubID=%X \r\n", SubID );

	switch( SubID )
	{
		case 0x0001: // 终端心跳包发送间隔  单位:s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.Heart_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 心跳包间隔: %d s\r\n", JT808Conf_struct.DURATION.Heart_Dur );
			break;
		case 0x0002: // TCP 消息应答超时时间  单位:s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.TCP_ACK_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n TCP消息应答间隔: %d s\r\n", JT808Conf_struct.DURATION.TCP_ACK_Dur );
			break;
		case 0x0003: //  TCP 消息重传次数
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.TCP_ReSD_Num = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n TCP重传次数: %d\r\n", JT808Conf_struct.DURATION.TCP_ReSD_Num );
			break;
		case 0x0004: // UDP 消息应答超时时间  单位:s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.UDP_ACK_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n UDP应答超时: %d\r\n", JT808Conf_struct.DURATION.UDP_ACK_Dur );
			break;
		case 0x0005: //  UDP 消息重传次数
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.UDP_ReSD_Num = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n UDP重传次数: %d\r\n", JT808Conf_struct.DURATION.UDP_ReSD_Num );
			break;
		case 0x0010: //  主服务器APN
			if( infolen == 0 )
			{
				break;
			}
			memset( APN_String, 0, sizeof( APN_String ) );
			memcpy( APN_String, (char*)Content, infolen );
			memset( (u8*)SysConf_struct.APN_str, 0, sizeof( APN_String ) );
			memcpy( (u8*)SysConf_struct.APN_str, (char*)Content, infolen );
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );

			DataLink_APN_Set( APN_String, 1 );

			break;
		case 0x0013: //  主服务器地址  IP 或域名
			memset( reg_in, 0, sizeof( reg_in ) );
			memcpy( reg_in, Content, infolen );
			//----------------------------

			i = str2ip( (char*)reg_in, RemoteIP_main );
			if( i <= 3 )
			{
				rt_kprintf( "\r\n  域名: %s \r\n", reg_in );

				memset( DomainNameStr, 0, sizeof( DomainNameStr ) );
				memset( SysConf_struct.DNSR, 0, sizeof( DomainNameStr ) );
				memcpy( DomainNameStr, (char*)Content, infolen );
				memcpy( SysConf_struct.DNSR, (char*)Content, infolen );
				Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );

				//----- 传给 GSM 模块------
				DataLink_DNSR_Set( SysConf_struct.DNSR, 1 );

				SD_ACKflag.f_CentreCMDack_0001H = 1; // 2 DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
				break;
			}
			memset( reg_str, 0, sizeof( reg_str ) );
			IP_Str( (char*)reg_str, *( u32* )RemoteIP_main );
			strcat( (char*)reg_str, " :" );
			sprintf( (char*)reg_str + strlen( (const char*)reg_str ), "%u\r\n", RemotePort_main );
			memcpy( SysConf_struct.IP_Main, RemoteIP_main, 4 );
			SysConf_struct.Port_main = RemotePort_main;

			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n 中心设置主服务器 IP \r\n" );
			rt_kprintf( "\r\n SOCKET :" );
			rt_kprintf( (char*)reg_str );
			//-----------  Below add by Nathan  ----------------------------
			rt_kprintf( "\r\n		   备用IP: %d.%d.%d.%d : %d \r\n", RemoteIP_aux[0], RemoteIP_aux[1], RemoteIP_aux[2], RemoteIP_aux[3], RemotePort_main );

			//-----------  Below add by Nathan  ----------------------------
			DataLink_MainSocket_set( RemoteIP_main, RemotePort_main, 1 );
			//-------------------------------------------------------------

			SD_ACKflag.f_CentreCMDack_0001H = 1;    //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断

			break;
		case 0x0014:                                // 备份服务器 APN

			break;
		case 0x0017:                                // 备份服务器  IP
			memset( reg_in, 0, sizeof( reg_in ) );
			memcpy( reg_in, Content, infolen );
			//----------------------------
			i = str2ip( (char*)reg_in, RemoteIP_aux );
			if( i <= 3 )
			{
				rt_kprintf( "\r\n  域名aux: %s \r\n", reg_in );
				memset( DomainNameStr_aux, 0, sizeof( DomainNameStr_aux ) );
				memset( SysConf_struct.DNSR_Aux, 0, sizeof( DomainNameStr_aux ) );
				memcpy( DomainNameStr_aux, (char*)Content, infolen );
				memcpy( SysConf_struct.DNSR_Aux, (char*)Content, infolen );
				Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
				//----- 传给 GSM 模块------
				DataLink_DNSR2_Set( SysConf_struct.DNSR_Aux, 1 );

				SD_ACKflag.f_CentreCMDack_0001H = 1; //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
				break;
			}
			memset( reg_str, 0, sizeof( reg_str ) );
			IP_Str( (char*)reg_str, *( u32* )RemoteIP_aux );
			strcat( (char*)reg_str, " :" );
			sprintf( (char*)reg_str + strlen( (const char*)reg_str ), "%u\r\n", RemotePort_aux );

			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n 中心设置备用服务器 IP \r\n" );
			rt_kprintf( "\r\nUDP SOCKET :" );
			rt_kprintf( (char*)reg_str );
			DataLink_AuxSocket_set( RemoteIP_aux, RemotePort_aux, 1 );
			//-----------  Below add by Nathan  ----------------------------
			rt_kprintf( "\r\n       备用IP: %d.%d.%d.%d : %d \r\n", RemoteIP_aux[0], RemoteIP_aux[1], RemoteIP_aux[2], RemoteIP_aux[3], RemotePort_aux );
			break;
		case 0x0018: //  服务器 TCP 端口
			//----------------------------
			if( infolen != 4 )
			{
				break;
			}
			RemotePort_main = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];

			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n 中心设置主服务器 PORT \r\n" );
			rt_kprintf( "\r\nUDP SOCKET :" );
			rt_kprintf( (char*)reg_str );
			//-----------  Below add by Nathan  ----------------------------
			DataLink_MainSocket_set( RemoteIP_main, RemotePort_main, 1 );   //-------------------------------------------------------------
			SD_ACKflag.f_CentreCMDack_0001H = 1;                            //DataLink_EndFlag=1; //AT_End(); 先返回结果再挂断
			break;
		case 0x0019:                                                        //  服务器 UDP 端口

			if( infolen != 4 )
			{
				break;
			}
			RemotePort_aux = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];

			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n 中心设置UDP服务器 PORT \r\n" );
			rt_kprintf( "\r\nUDP SOCKET :" );
			rt_kprintf( (char*)reg_str );
			rt_kprintf( "\r\n		 备用IP: %d.%d.%d.%d : %d \r\n", RemoteIP_aux[0], RemoteIP_aux[1], RemoteIP_aux[2], RemoteIP_aux[3], RemotePort_aux );
			break;
		case 0x0020: //  汇报策略  0 定时汇报  1 定距汇报 2 定时和定距汇报
			if( infolen != 4 )
			{
				break;
			}
			resualtu32 = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			switch( resualtu32 )
			{
				case 0: rt_kprintf( "\r\n 定时汇报 \r\n" );
					break;
				case 1: rt_kprintf( "\r\n 定距汇报 \r\n" );
					break;
				case 2: rt_kprintf( "\r\n 定时和定距汇报\r\n" );
					break;
				default:
					break;
			}
			break;
		case 0x0021:    //  位置汇报方案  0 根据ACC上报  1 根据ACC和登录状态上报

			break;
		//--------

		case 0x0022:    //  驾驶员未登录 汇报时间间隔 单位:s    >0
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.NoDrvLogin_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 驾驶员未登录汇报间隔: %d\r\n", JT808Conf_struct.DURATION.NoDrvLogin_Dur );
			break;
		case 0x0027: //  休眠时汇报时间间隔，单位 s  >0
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.Sleep_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 休眠汇报时间间隔: %d \r\n", JT808Conf_struct.DURATION.Sleep_Dur );
			break;
		case 0x0028: //  紧急报警时汇报时间间隔  单位 s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.Emegence_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 紧急报警时间间隔: %d \r\n", JT808Conf_struct.DURATION.Emegence_Dur );
			break;
		case 0x0029: //  缺省时间汇报间隔  单位 s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.Default_Dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 缺省汇报时间间隔: %d \r\n", JT808Conf_struct.DURATION.Default_Dur );
			break;
		//---------

		case 0x002C: //  缺省距离汇报间隔  单位 米
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DISTANCE.Defalut_DistDelta = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 缺省距离汇报间隔: %d m\r\n", JT808Conf_struct.DISTANCE.Defalut_DistDelta );
			break;
		case 0x002D: //  驾驶员未登录汇报距离间隔 单位 米
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DISTANCE.NoDrvLogin_Dist = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 驾驶员未登录汇报距离: %d m\r\n", JT808Conf_struct.DISTANCE.NoDrvLogin_Dist );
			break;
		case 0x002E: //  休眠时汇报距离间隔  单位 米
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DISTANCE.Sleep_Dist = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 休眠时定距上报间隔: %d m\r\n", JT808Conf_struct.DISTANCE.Sleep_Dist );
			break;
		case 0x002F: //  紧急报警时汇报距离间隔  单位 米
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DISTANCE.Emergen_Dist = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 紧急报警时定距上报间隔: %d m\r\n", JT808Conf_struct.DISTANCE.Emergen_Dist );
			break;
		case 0x0030: //  拐点补传角度 , <180
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.DURATION.SD_Delta_maxAngle = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 拐点补传角度: %d 度\r\n", JT808Conf_struct.DISTANCE.Emergen_Dist );
			break;
		case 0x0040: //   监控平台电话号码
			if( infolen == 0 )
			{
				break;
			}
			i = strlen( (const char*)JT808Conf_struct.LISTEN_Num );
			rt_kprintf( "\r\n old: %s \r\n", JT808Conf_struct.LISTEN_Num );

			memset( JT808Conf_struct.LISTEN_Num, 0, sizeof( JT808Conf_struct.LISTEN_Num ) );
			memcpy( JT808Conf_struct.LISTEN_Num, Content, infolen );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n new: %s \r\n", JT808Conf_struct.LISTEN_Num );

			//CallState=CallState_rdytoDialLis;  // 准备开始拨打监听号码
			rt_kprintf( "\r\n 设置监控平台号码: %s \r\n", JT808Conf_struct.LISTEN_Num );

			break;
		case 0x0041: //   复位电话号码，可采用此电话号码拨打终端电话让终端复位
			if( infolen == 0 )
			{
				break;
			}
			memset( reg_str, 0, sizeof( reg_str ) );
			memcpy( reg_str, Content, infolen );
			rt_kprintf( "\r\n 复位电话号码 %s \r\n", reg_str );
			break;
		case 0x0042:    //   恢复出厂设置电话，可采用该电话号码是终端恢复出厂设置

			break;
		case 0x0045:    //  终端电话接听策略 0 自动接听  1 ACC ON自动接听 OFF时手动接听

			break;
		case 0x0046:    //  每次通话最长时间 ，单位  秒

			break;
		case 0x0047:    //  当月最长通话时间，单位  秒

			break;
		case 0x0048:    //  监听电话号码
			if( infolen == 0 )
			{
				break;
			}
			memset( JT808Conf_struct.LISTEN_Num, 0, sizeof( JT808Conf_struct.LISTEN_Num ) );
			memcpy( JT808Conf_struct.LISTEN_Num, Content, infolen );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			CallState = CallState_rdytoDialLis; // 准备开始拨打监听号码
			rt_kprintf( "\r\n 立即拨打监听号码: %s \r\n", JT808Conf_struct.LISTEN_Num );
			break;

		//----------
		case 0x0050:                            //  报警屏蔽字， 与位置信息中报警标志相对应。相应位为1时报警被屏蔽---

			if( infolen != 4 )
			{
				break;
			}
			resualtu32 = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			rt_kprintf( "\r\n 报警屏蔽字: %x \r\n", resualtu32 );
			break;
		case 0x0052:    //  报警拍照开关， 与报警标志对应的位1时，拍照

			break;
		case 0x0053:    //  报警拍照存储    与报警标志对应的位1时，拍照存储 否则实时上传

			break;
		case 0x0054:    //  关键标志        与报警标志对应的位1  为关键报警

			break;
		//---------

		case 0x0055:    //  最高速度   单位   千米每小时
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.Speed_warn_MAX = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			memset( reg_str, 0, sizeof( reg_str ) );
			memcpy( reg_str, &JT808Conf_struct.Speed_warn_MAX, 4 );
			memcpy( reg_str + 4, &JT808Conf_struct.Spd_Exd_LimitSeconds, 4 );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 最高速度: %d km/h \r\n", JT808Conf_struct.Speed_warn_MAX );
			Spd_ExpInit( );
			break;
		case 0x0056: //  超速持续时间    单位 s
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.Spd_Exd_LimitSeconds = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			memset( reg_str, 0, sizeof( reg_str ) );
			memcpy( reg_str, &JT808Conf_struct.Speed_warn_MAX, 4 );
			memcpy( reg_str + 4, &JT808Conf_struct.Spd_Exd_LimitSeconds, 4 );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 超时持续时间: %d s \r\n", JT808Conf_struct.Spd_Exd_LimitSeconds );
			Spd_ExpInit( );
			break;
		case 0x0057:                    //  连续驾驶时间门限 单位  s
			if( infolen != 4 )
			{
				break;
			}
			TiredConf_struct.TiredDoor.Door_DrvKeepingSec = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( tired_config, 0, (u8*)&TiredConf_struct, sizeof( TiredConf_struct ) );
			Warn_Status[3] &= ~0x04;    //BIT(2)	接触疲劳驾驶报警
			TIRED_Drive_Init( );        // 清除疲劳驾驶状态
			rt_kprintf( "\r\n 连续驾驶时间门限: %d s \r\n", TiredConf_struct.TiredDoor.Door_DrvKeepingSec );
			break;
		case 0x0058:                    //  当天累计驾驶时间门限  单位  s
			if( infolen != 4 )
			{
				break;
			}
			TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( tired_config, 0, (u8*)&TiredConf_struct, sizeof( TiredConf_struct ) );
			TiredConf_struct.Tired_drive.ACC_ONstate_counter	= 0;
			TiredConf_struct.Tired_drive.ACC_Offstate_counter	= 0;
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 当天累计驾驶时间: %d s \r\n", TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec );
			break;
		case 0x0059: //  最小休息时间  单位 s
			if( infolen != 4 )
			{
				break;
			}
			TiredConf_struct.TiredDoor.Door_MinSleepSec = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( tired_config, 0, (u8*)&TiredConf_struct, sizeof( TiredConf_struct ) );
			rt_kprintf( "\r\n 最小休息时间: %d s \r\n", TiredConf_struct.TiredDoor.Door_MinSleepSec );
			break;
		case 0x005A: //  最长停车时间   单位 s
			if( infolen != 4 )
			{
				break;
			}
			TiredConf_struct.TiredDoor.Door_MaxParkingSec = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( tired_config, 0, (u8*)&TiredConf_struct, sizeof( TiredConf_struct ) );
			TiredConf_struct.TiredDoor.Parking_currentcnt	= 0;
			Warn_Status[1]									&= ~0x08;   // 清除超时触发
			rt_kprintf( "\r\n 最长停车时间: %d s \r\n", TiredConf_struct.TiredDoor.Door_MaxParkingSec );
			break;
		//---------
		case  0x0070:                                                   //  图像/视频质量  1-10  1 最好

			break;
		case  0x0071:                                                   //  亮度  0-255

			break;
		case  0x0072:                                                   //  对比度  0-127

			break;
		case  0x0073:                                                   // 饱和度  0-127

			break;
		case  0x0074:                                                   // 色度   0-255

			break;
		//---------
		case  0x0080:                                                   // 车辆里程表读数   1/10 km

			break;
		case  0x0081:                                                   // 车辆所在的省域ID
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.Vechicle_Info.Dev_ProvinceID = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 车辆所在省域ID: 0x%X \r\n", JT808Conf_struct.Vechicle_Info.Dev_ProvinceID );
			break;
		case  0x0082: // 车辆所在市域ID
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.Vechicle_Info.Dev_ProvinceID = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 车辆所在市域ID: 0x%X \r\n", JT808Conf_struct.Vechicle_Info.Dev_ProvinceID );
			break;
		case  0x0083: // 公安交通管理部门颁发的机动车号牌
			if( infolen < 4 )
			{
				break;
			}
			memset( JT808Conf_struct.Vechicle_Info.Vech_Num, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_Num ) );
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_Num, Content, infolen );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 机动车驾驶证号: %s  \r\n", JT808Conf_struct.Vechicle_Info.Vech_Num );
			break;
		case  0x0084: // 车牌颜色  按照国家规定
			if( infolen != 1 )
			{
				break;
			}
			JT808Conf_struct.Vechicle_Info.Dev_Color = Content[0];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 车辆颜色: %d  \r\n", JT808Conf_struct.Vechicle_Info.Dev_Color );
			break;
		//--------------- BD  新增----------------------------------
		case  0x001A: //  IC卡 主服务器或域名
			memset( reg_in, 0, sizeof( reg_in ) );
			memcpy( reg_in, Content, infolen );
			//----------------------------

			i = str2ip( (char*)reg_in, SysConf_struct.BD_IC_main_IP );
			if( i <= 3 )
			{
				rt_kprintf( "\r\n IC  主 域名: %s \r\n", reg_in );

				memset( SysConf_struct.BD_IC_DNSR, 0, sizeof( SysConf_struct.BD_IC_DNSR ) );
				memcpy( SysConf_struct.BD_IC_DNSR, (char*)Content, infolen );
				Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
				break;
			}
			memset( reg_str, 0, sizeof( reg_str ) );
			IP_Str( (char*)reg_str, *( u32* )SysConf_struct.BD_IC_main_IP );
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n IC  主IP: %d.%d.%d.%d \r\n", SysConf_struct.BD_IC_main_IP[0], SysConf_struct.BD_IC_main_IP[1], SysConf_struct.BD_IC_main_IP[2], SysConf_struct.BD_IC_main_IP[3] );
			break;
		case  0x001B:   // IC 卡  主TCP端口
			SysConf_struct.BD_IC_TCP_port = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n IC  TCP Port: %d \r\n", SysConf_struct.BD_IC_TCP_port );
			//  IC 卡中心
			DataLink_IC_Socket_set( SysConf_struct.BD_IC_main_IP, SysConf_struct.BD_IC_TCP_port, 0 );
			break;
		case  0x001C:   // IC卡   主UDP 端口
			SysConf_struct.BD_IC_UDP_port = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n IC  UDP Port : %d \r\n", SysConf_struct.BD_IC_UDP_port );
			break;
		case  0x001D:   // IC 卡备用服务器和端口
			memset( reg_in, 0, sizeof( reg_in ) );
			memcpy( reg_in, Content, infolen );
			//----------------------------
			i = str2ip( (char*)reg_in, SysConf_struct.BD_IC_Aux_IP );
			if( i <= 3 )
			{
				rt_kprintf( "\r\n IC  主 域名: %s \r\n", reg_in );

				memset( SysConf_struct.BD_IC_DNSR_Aux, 0, sizeof( SysConf_struct.BD_IC_DNSR_Aux ) );
				memcpy( SysConf_struct.BD_IC_DNSR_Aux, (char*)Content, infolen );
				Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
				break;
			}
			memset( reg_str, 0, sizeof( reg_str ) );
			IP_Str( (char*)reg_str, *( u32* )SysConf_struct.BD_IC_Aux_IP );
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			rt_kprintf( "\r\n IC  备用IP: %d.%d.%d.%d \r\n", SysConf_struct.BD_IC_Aux_IP[0], SysConf_struct.BD_IC_Aux_IP[1], SysConf_struct.BD_IC_Aux_IP[2], SysConf_struct.BD_IC_Aux_IP[3] );

			break;
		case   0x0031: //  电子围栏半径(非法位移阈值)
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.BD_CycleRadius_DoorValue = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 电子围栏半径(非法移动阈值): %d m\r\n", JT808Conf_struct.BD_CycleRadius_DoorValue );
			break;
		case   0x005B: // 超速报警预警差值   1/10 KM/h
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.BD_MaxSpd_preWarnValue = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 超速报警预警差值: %d x 0.1km/h\r\n", JT808Conf_struct.BD_MaxSpd_preWarnValue );

			break;
		case   0x005C: // 疲劳驾驶阈值  单位:s
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.BD_TiredDrv_preWarnValue = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 疲劳驾驶阈值: %d s\r\n", JT808Conf_struct.BD_TiredDrv_preWarnValue );
			break;
		case   0x005D: // 碰撞报警参数设置
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.BD_Collision_Setting = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 碰撞报警参数设置 : %x \r\n", JT808Conf_struct.BD_MaxSpd_preWarnValue );
			break;
		case   0x005E: // 侧翻报警参数设置   默认30度
			if( infolen != 2 )
			{
				break;
			}
			JT808Conf_struct.BD_Laydown_Setting = ( Content[0] << 8 ) + Content[1];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n侧翻报警参数设置: %x r\n", JT808Conf_struct.BD_Laydown_Setting );
			break;
		//---  CAMERA
		case   0x0064: //  定时拍照控制
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.BD_CameraTakeByTime_Settings = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n 定时拍照控制: %X\r\n", JT808Conf_struct.BD_CameraTakeByTime_Settings );
			break;
		case   0x0065: //  定距离拍照控制
			if( infolen != 4 )
			{
				break;
			}
			JT808Conf_struct.BD_CameraTakeByDistance_Settings = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			rt_kprintf( "\r\n  定距离拍照控制: %X\r\n", JT808Conf_struct.BD_CameraTakeByDistance_Settings );
			break;
		//--- GNSS
		case    0x0090:                         // GNSS 定位模式

			JT808Conf_struct.BD_EXT.GNSS_Mode = Content[0];
			rt_kprintf( "\r\n  GNSS Value= 0x%2X	\r\n", Content[0] );
			switch( JT808Conf_struct.BD_EXT.GNSS_Mode )
			{
				case 0x01:                      // 单 GPS 定位模式
					gps_mode( "2" );
					Car_Status[1]	&= ~0x0C;   // clear bit3 bit2      1100
					Car_Status[1]	|= 0x04;    // Gps mode   0100
					break;
				case  0x02:                     //  单BD2 定位模式
					gps_mode( "1" );
					Car_Status[1]	&= ~0x0C;   // clear bit3 bit2
					Car_Status[1]	|= 0x08;    // BD mode	1000
					break;
				case  0x03:                     //  BD2+GPS 定位模式
					gps_mode( "3" );
					Car_Status[1]	&= ~0x0C;   // clear bit3 bit2
					Car_Status[1]	|= 0x0C;    // BD+GPS  mode	1100
					break;
			}
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			break;
		case    0x0091:                         // GNSS 波特率
			JT808Conf_struct.BD_EXT.GNSS_Baud = Content[0];
			switch( JT808Conf_struct.BD_EXT.GNSS_Baud )
			{
				case 0x00:                      //  4800
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 4800;
					//rt_thread_delay(5);
					//gps_write("$PCAS01,0*1C\r\n",14);
					//rt_thread_delay(5);
					// gps_baud( 4800 );
					break;
				case 0x01:                      //9600   --default
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 9600;
					//rt_thread_delay(5);
					//gps_write("$PCAS01,1*1D\r\n",14);
					//rt_thread_delay(5);
					//gps_baud( 9600 );
					break;
				case  0x02:                     // 19200
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 19200;
					//rt_thread_delay(5);
					//gps_write("$PCAS01,2*1E\r\n",14);
					//rt_thread_delay(5);
					//gps_baud( 19200 );
					break;
				case  0x03:                     //  38400
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 38400;
					//rt_thread_delay(5);
					//gps_write("$PCAS01,3*1F\r\n",14);
					//rt_thread_delay(5);
					//gps_baud( 38400 );
					break;
				case  0x04:                     // 57600
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 57600;
					//rt_thread_delay(5);
					//gps_write("$PCAS01,4*18\r\n",14);
					//rt_thread_delay(5);
					//gps_baud( 57600 );
					break;
				case   0x05:                    // 115200
					JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 115200;
					//rt_thread_delay(5);
					// gps_write("$PCAS01,5*19\r\n",14);
					//rt_thread_delay(5);
					// gps_baud( 115200 );
					break;
			}
			//---UART_GPS_Init(baud);  //   修改串口波特率
			rt_thread_delay( 20 );
			// BD_EXT_Write();
			rt_kprintf( "\r\n 中心设置GNSS 波特率:  %d s\r\n", JT808Conf_struct.BD_EXT.GNSS_Baud_Value );

			break;
		case    0x0092:     // GNSS 模块详细定位数据输出频率

			break;
		case    0x0093:     //  GNSS 模块详细定位数据采集频率  1

			break;
		case    0x0094:     // GNSS 模块详细定位数据上传方式

			break;
		case    0x0095:     //  GNSS 模块详细上传设置

			break;
		//----CAN--
		case      0x0100:   //  CAN  总线通道 1  采集间隔              0   : 表示不采集
			if( infolen != 4 )
			{
				break;
			}
			CAN_trans.can1_sample_dur	= ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			Can_RXnum					= 0;
			Can_sdnum					= 0;
			Can_same					= 0;
			break;
		case    0x0101: //  CAN  总线通道 1 上传时间间隔    0 :  表示不上传
			if( infolen != 2 )
			{
				break;
			}
			CAN_trans.can1_trans_dur = ( Content[0] << 8 ) + Content[1];

			break;
		case    0x0102: //  CAN  总线通道 2  采集间隔              0   : 表示不采集
			if( infolen != 4 )
			{
				break;
			}
			CAN_trans.can2_sample_dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			break;
		case    0x0103: //  CAN  总线通道 2 上传时间间隔    0 :  表示不上传
			if( infolen != 2 )
			{
				break;
			}
			CAN_trans.can2_trans_dur = ( Content[0] << 8 ) + Content[1];
			break;
		case    0x0110: //  CAN 总线ID 单独采集设置
			CAN_trans.canid_2_NotGetID = ( ( Content[4] & 0x1F ) << 24 ) + ( Content[5] << 16 ) + ( Content[6] << 8 ) + Content[7];
			rt_kprintf( "\r\n不采集ID  0x0110= %08X\r\n", CAN_trans.canid_2_NotGetID );

			break;
		case    0x0111: //  CAN 总线ID 单独采集设置 其他
			if( infolen != 8 )
			{
				break;
			}
			OutPrint_HEX( "0x0111", Content, 8 );
			memcpy( CAN_trans.canid_1, Content, 8 );
			memset( CAN_trans.canid_1_Rxbuf, 0, sizeof( CAN_trans.canid_1_Rxbuf ) );
			CAN_trans.canid_1_RxWr		= 0;    // clear  write
			CAN_trans.canid_timer		= 0;
			CAN_trans.canid_0705_sdFlag = 0;

			//------ 解析赋值 --------
			CAN_trans.canid_1_sample_dur = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			if( Content[4] & 0x40 )             // bit 30
			{
				CAN_trans.canid_1_ext_state = 1;
			} else
			{
				CAN_trans.canid_1_ext_state = 0;
			}
			CAN_trans.canid_1_Filter_ID = ( ( Content[4] & 0x1F ) << 24 ) + ( Content[5] << 16 ) + ( Content[6] << 8 ) + Content[7];

			rt_kprintf( "\r\n FilterID=%08X, EXTstate: %d   can1_samle=%d ms   canid_1_sample_dur=%dms    Trans_dur=%d s\r\n", CAN_trans.canid_1_Filter_ID, CAN_trans.canid_1_ext_state, CAN_trans.can1_sample_dur, CAN_trans.canid_1_sample_dur, CAN_trans.can1_trans_dur );

			break;
		default:
			return false;
	}

	return true;
}

//--------------------------------------------------------------------
u8  CentreSet_subService_8105H( u32 Control_ID, u8 infolen, u8 *Content )
{
	switch( Control_ID )
	{
		case 1:                                         //  无线升级参数  参数之间采用分号分隔   指令格式如下:


			/*
			   URL 地址；拨号名称；拨号用户名；拨号密码；地址；TCP端口；UDP端口；制造商ID; 硬件版本；固件版本；连接到指定服务器指定是服务器时限；
			   若某个参数无数值，则放空
			 */
			rt_kprintf( "\r\n 无线升级 \r\n" );
			rt_kprintf( "\r\n 内容: %s\r\n", Content );
			break;
		case 2:                                         // 控制终端连接指定服务器


			/*
			   连接控制；监管平台鉴权码；拨号点名称； 拨号用户名；拨号密码；地址；TCP端口；UDP端口；连接到指定服务器时限
			   若每个参数无数值，则放空
			 */
			rt_kprintf( "\r\n 终端控制连接指定服务器\r\n" );
			rt_kprintf( "\r\n 内容: %s\r\n", Content );
			break;
		case 3:                                         //  终端关机
			SD_ACKflag.f_CentreCMDack_0001H = 5;
			rt_kprintf( "\r\n 终端关机 \r\n" );
			break;
		case 4:                                         //  终端复位
			SD_ACKflag.f_CentreCMDack_0001H = 3;
			rt_kprintf( "\r\n 终端复位 \r\n" );
			break;
		case 5:                                         //   终端恢复出厂设置

			if( SysConf_struct.Version_ID == SYSID )    //  check  wether need  update  or not
			{
				SysConf_struct.Version_ID = SYSID + 1;
				Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
				Systerm_Reset_counter	= Max_SystemCounter;
				ISP_resetFlag			= 2;            //   借助远程下载重启机制复位系统
			}
			rt_kprintf( "\r\n 恢复出厂设置 \r\n" );
			break;
		case 6:                                         //   关闭数据通信
			SD_ACKflag.f_CentreCMDack_0001H = 5;
			rt_kprintf( "\r\n 关闭数据通信 \r\n" );
			break;
		case 7:                                         //   关闭所有无线通信
			SD_ACKflag.f_CentreCMDack_0001H = 5;
			rt_kprintf( "\r\n 关闭所有通信 \r\n" );
			break;
		default:
			return false;
	}
	return true;
}

//-------------------------------------------------------------------
void CenterSet_subService_8701H( u8 cmd, u8*Instr )
{
	switch( cmd )
	{
		case 0x81:                                                                  //	  中心设置 驾驶员代码  驾驶证号码
			memset( JT808Conf_struct.Driver_Info.DriverCard_ID, 0, 18 );
			//  驾驶员代码没有处理 3个字节
			memcpy( JT808Conf_struct.Driver_Info.DriveCode, Instr, 3 );
			memcpy( JT808Conf_struct.Driver_Info.DriverCard_ID, Instr + 3, 18 );    //只要驾驶证号码
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			break;

		case 0x82:                                                                  //	  中心设置车牌号
			memset( (u8*)&JT808Conf_struct.Vechicle_Info, 0, sizeof( JT808Conf_struct.Vechicle_Info ) );

			memcpy( JT808Conf_struct.Vechicle_Info.Vech_VIN, Instr, 17 );
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_Num, Instr + 17, 12 );
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_Type, Instr + 29, 12 );

			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			break;
		case 0xC2:                                                                                                      //设置记录仪时钟
			// 没啥用，给个回复就行，俺有GPS校准就够了

			break;

		case 0xC3:                                                                                                      //车辆速度脉冲系数（特征系数）
			JT808Conf_struct.Vech_Character_Value = (u32)( Instr[0] << 16 ) + (u32)( Instr[1] << 8 ) + (u32)Instr[2];   // 特征系数  速度脉冲系数

			JT808Conf_struct.DF_K_adjustState	= 0;
			ModuleStatus						&= ~Status_Pcheck;
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			break;
		default:
			break;
	}
}

//-----------------------------------------------
u8  CentreSet_subService_FF01H( u32 SubID, u8 infolen, u8 *Content )
{
	u32 baud = 0;
	u8	u3_Txstr[40];
	u8	Destu3_Txstr[40];
	u8	len = 0;
	u8	i	= 0;

	rt_kprintf( "\r\n    收到扩展终端设置命令 SubID=%X \r\n", SubID );

	switch( SubID )
	{
		case 0x0003: //  BNSS NMEA 输出更新率
			if( infolen != 4 )
			{
				break;
			}
			BD_EXT.BD_OutputFreq = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			switch( BD_EXT.BD_OutputFreq )
			{
				case 0x00:
					baud = 500;
					gps_write( "$PCAS02,500*1A\r\n", 16 );
					rt_thread_delay( 5 );
					break;
				case 0x01:
					baud = 1000;
					gps_write( "$PCAS02,1000*2E\r\n", 16 );
					rt_thread_delay( 5 );
					break;
				case 0x02:
					baud = 2000;
					gps_write( "$PCAS02,2000*2D\r\n", 16 );
					rt_thread_delay( 5 );
					break;
				case  0x03:
					baud = 3000;
					gps_write( "$PCAS02,3000*2C\r\n", 16 );
					rt_thread_delay( 5 );
					break;
				case  0x04:
					baud = 4000;
					gps_write( "$PCAS02,4000*2B\r\n", 16 );
					rt_thread_delay( 5 );
					break;
			}
			// BD_EXT_Write();
			rt_kprintf( "\r\n  GNSS 输出更新: %dms\r\n", baud );
			break;
		case 0x0004: // GNSS 采集NMEA 数据频率
			if( infolen != 4 )
			{
				break;
			}
			BD_EXT.BD_SampleFrea = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			rt_kprintf( "\r\n GNSS 采集频率: %d s\r\n", BD_EXT.BD_SampleFrea );
			break;
		case 0x0005: //  CAN 1  参数设置
			if( infolen != 4 )
			{
				break;
			}
			BD_EXT.CAN_1_Mode = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			CAN_App_Init( );
			//BD_EXT_Write();
			rt_kprintf( "\r\n 中心设置CAN1 : 0x%08X\r\n", BD_EXT.CAN_1_Mode );
			break;
		case 0x0006: //  CAN2   参数设置    ----- 要和小板通信
			if( infolen != 4 )
			{
				break;
			}
			BD_EXT.CAN_2_Mode = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			// BD_EXT_Write();
			memset( u3_Txstr, 0, sizeof( u3_Txstr ) );
			memset( Destu3_Txstr, 0, sizeof( Destu3_Txstr ) );
			u3_Txstr[0] = 0x7E; // 头
			u3_Txstr[1] = 0x33; //  CAN 相关
			u3_Txstr[2] = 0x01; //  参数设置
			u3_Txstr[3] = ( BD_EXT.CAN_2_Mode >> 24 );
			u3_Txstr[4] = ( BD_EXT.CAN_2_Mode >> 16 );
			u3_Txstr[5] = ( BD_EXT.CAN_2_Mode >> 8 );
			u3_Txstr[6] = ( BD_EXT.CAN_2_Mode );
			u3_Txstr[7] = 0x7E;

			Destu3_Txstr[0]			= 0x7E;
			len						= Protocol_808_Encode( Destu3_Txstr + 1, u3_Txstr + 1, 6 );
			Destu3_Txstr[len + 1]	= 0x7E;

			rt_kprintf( "\r\n U3_orginal:" );
			for( i = 0; i < 8; i++ )
			{
				rt_kprintf( "% 02X", u3_Txstr[i] );
			}
			rt_kprintf( "\r\nU3_len=%d \r\n", len );
			for( i = 0; i < len + 2; i++ )
			{
				rt_kprintf( "% 02X", Destu3_Txstr[i] );
			}
			// U3_PutData(Destu3_Txstr,len+2);  //  发送给串口
			rt_device_write( &Device_CAN2, 0, Destu3_Txstr, len + 2 );
			rt_kprintf( "\r\n 中心设置CAN2 : 0x%08X\r\n", BD_EXT.CAN_2_Mode );
			break;
		case 0x0007: //  碰撞参数设置
			if( infolen != 4 )
			{
				break;
			}
			BD_EXT.Collision_Check = ( Content[0] << 24 ) + ( Content[1] << 16 ) + ( Content[2] << 8 ) + Content[3];
			//BD_EXT_Write();
			memset( u3_Txstr, 0, sizeof( u3_Txstr ) );
			memset( Destu3_Txstr, 0, sizeof( Destu3_Txstr ) );
			u3_Txstr[0] = 0x7E; // 头
			u3_Txstr[1] = 0x32; //  CAN 相关
			u3_Txstr[2] = 0x01; //  参数设置
			u3_Txstr[3] = ( BD_EXT.Collision_Check >> 24 );
			u3_Txstr[4] = ( BD_EXT.Collision_Check >> 16 );
			u3_Txstr[5] = ( BD_EXT.Collision_Check >> 8 );
			u3_Txstr[6] = BD_EXT.Collision_Check;
			u3_Txstr[7] = 0x7E;

			Destu3_Txstr[0]			= 0x7E;
			len						= Protocol_808_Encode( Destu3_Txstr + 1, u3_Txstr + 1, 6 );
			Destu3_Txstr[len + 1]	= 0x7E;

			//   设置配置碰撞参数
			mma8451_config( (uint16_t)( BD_EXT.Collision_Check >> 16 ), (uint16_t)( BD_EXT.Collision_Check ) );
			rt_kprintf( "\r\n 中心设置碰撞参数: 0x%08X\r\n", BD_EXT.Collision_Check );;
			break;

		default:
			return false;
	}
	rt_thread_delay( 20 );
	return true;
}

//--------------------------------------------------
u8  CentreSet_subService_FF03H( u32 SubID, u8 infolen, u8 *Content )
{
	u8	i = 0;
	u8	u3_Txstr[40];
	u8	Destu3_Txstr[40];
	u8	len = 0;

	rt_kprintf( "\r\n    收到扩展终端参数设置1 命令 SubID=%X \r\n", SubID );

	switch( SubID )
	{
		case 0x0001: // CAN ID 设置
			if( infolen < 4 )
			{
				break;
			}
			// Content[0];  //  设置属性
			//Content[1] ;//本消息中包含CAN ID 的数量
			//------------------------------------
			//Content[2];  //  CAN ID 的ID
			if( Content[3] & 0x80 )                                                                                 //  CAN ID   的属性
			{
				BD_EXT.CAN_2_ID = ( Content[4] << 24 ) + ( Content[5] << 16 ) + ( Content[6] << 8 ) + Content[7];   //
				if( Content[3] & 0x40 )
				{
					BD_EXT.CAN_2_Type = 1;                                                                          // 标准帧
				}else
				{
					BD_EXT.CAN_2_Type = 0;
				}
				BD_EXT.CAN_2_TransDuration = ( Content[9] << 8 ) + Content[10];                                     // 0   表示停止

				//     得有个和小板通信的处理
				memset( Destu3_Txstr, 0, sizeof( Destu3_Txstr ) );
				u3_Txstr[0]		= 0x7E;                                                                             // 头
				u3_Txstr[1]		= 0x33;                                                                             //  CAN 相关
				u3_Txstr[2]		= 0x02;                                                                             //  参数设置
				u3_Txstr[3]		= Content[3];                                                                       // 属性
				u3_Txstr[4]		= Content[4];                                                                       //  ID
				u3_Txstr[5]		= Content[5];
				u3_Txstr[6]		= Content[6];
				u3_Txstr[7]		= Content[7];
				u3_Txstr[8]		= Content[8];                                                                       // ID
				u3_Txstr[9]		= Content[9];                                                                       //
				u3_Txstr[10]	= Content[10];
				u3_Txstr[11]	= 0x7E;

				Destu3_Txstr[0]			= 0x7E;
				len						= Protocol_808_Encode( Destu3_Txstr + 1, u3_Txstr + 1, 10 );
				Destu3_Txstr[len + 1]	= 0x7E;

				rt_kprintf( "\r\nU3_len=%d   :\r\n", len );
				for( i = 0; i < len + 2; i++ )
				{
					rt_kprintf( "% 02X", Destu3_Txstr[i] );
				}

				// U3_PutData(Destu3_Txstr,len+2);  //  发送给串口
				rt_device_write( &Device_CAN2, 0, Destu3_Txstr, len + 2 );
				rt_kprintf( "\r\n 中心设置CAN2 : 0x%08X   Dur: %d s\r\n", BD_EXT.CAN_2_ID, BD_EXT.CAN_2_TransDuration );
				//----------------------------------
			}else
			{
				BD_EXT.CAN_1_ID = ( Content[4] << 24 ) + ( Content[5] << 16 ) + ( Content[6] << 8 ) + Content[7];   //
				if( Content[3] & 0x40 )
				{
					BD_EXT.CAN_1_Type = 1;                                                                          // 标准帧
				}else
				{
					BD_EXT.CAN_1_Type = 0;
				}
				BD_EXT.CAN_1_TransDuration = ( Content[9] << 8 ) + Content[10];                                     // 0   表示停止

				rt_kprintf( "\r\n 中心设置CAN1 : 0x%08X   Dur: %d s\r\n", BD_EXT.CAN_1_ID, BD_EXT.CAN_1_TransDuration );
			}
			BD_EXT_Write( );
			break;
		case 0x0002:                                                                                                // 文本信息标志含义
			                                                                                                        // Content[0]; // 没用
			                                                                                                        //if((Content[1]==0)||(Content[1]==1))  //  部分类型都给显示屏
		{
			memset( TextInfo.TEXT_Content, 0, sizeof( TextInfo.TEXT_Content ) );
			DwinLCD.TxInfolen = AsciiToGb( TextInfo.TEXT_Content, infolen - 2, Content + 2 );
			//memcpy(TextInfo.TEXT_Content,Content+2,infolen-2);
			TextInfo.TEXT_SD_FLAG = 1;                                                                              // 置发送给显示屏标志位  // ||||||||||||||||||||||||||||||||||
			rt_kprintf( "\r\n  CAN 文本:  " );
			for( i = 0; i < DwinLCD.TxInfolen; i++ )
			{
				rt_kprintf( "%c", TextInfo.TEXT_Content[i] );
			}
			rt_kprintf( "\r\n " );
		}

#ifdef LCD_5inch
			//======  信息都在屏幕上显示
			memset( DwinLCD.TXT_content, 0, sizeof( DwinLCD.TXT_content ) );
			DwinLCD.TXT_contentLen	= AsciiToGb( DwinLCD.TXT_content, infolen - 2, Content + 2 );
			DwinLCD.Type			= LCD_SDTXT;

#endif

			break;
		default:
			return false;
	}
	return true;
}

//-------------------------------------------
void  Media_Start_Init( u8 MdType, u8 MdCodeType )
{
	MediaObj.Media_Type				= MdType;       //	指定当前传输多媒体的类型   0  表示图片
	MediaObj.Media_CodeType			= MdCodeType;   //  多媒体编码格式   0  表示JPEG	格式
	MediaObj.SD_media_Flag			= 1;            //  置多媒体事件信息发送标志位  ，开始图片传输
	MediaObj.SD_Eventstate			= 1;            //  开始处于发送状态
	MediaObj.RSD_State				= 0;
	MediaObj.RSD_Timer				= 0;
	MediaObj.RSD_total				= 0;
	MediaObj.RSD_Reader				= 0;
	MediaObj.SD_Data_Flag			= 0;
	MediaObj.Media_transmittingFlag = 0;
	//------------  add  for debuging   --------------
	// Media_Clear_State();  //  clear


	/*  if(0==MediaObj.Media_Type)
	   {
	   Photo_sdState.photo_sending=enable;
	   Photo_sdState.SD_packetNum=1; // 第一包开始
	   rt_kprintf("\r\n 开始上传照片! ....\r\n");
	   }
	 */
	//----------------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Media_Clear_State( void )
{
	// 不清楚Meia Type
	MediaObj.MaxSd_counter			= 0;
	MediaObj.SD_Eventstate			= 0;
	MediaObj.SD_timer				= 0;
	MediaObj.SD_media_Flag			= 0;
	MediaObj.SD_Data_Flag			= 0;
	MediaObj.RSD_State				= 0;
	MediaObj.RSD_Timer				= 0;
	MediaObj.RSD_total				= 0;
	MediaObj.RSD_Reader				= 0;
	MediaObj.Media_transmittingFlag = 0;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  Media_Timer( void )
{
	if( 1 == MediaObj.SD_Eventstate )
	{
		MediaObj.SD_timer++;
		if( MediaObj.SD_timer > 6 )
		{
			MediaObj.SD_timer		= 0;
			MediaObj.SD_media_Flag	= 1;
			MediaObj.MaxSd_counter++;
			if( MediaObj.MaxSd_counter > 5 )
			{
				Media_Clear_State( );
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Media_RSdMode_Timer( void )
{
    //   多媒体列表重传 
	if( ( 1 == MediaObj.RSD_State ) )
	{
		MediaObj.RSD_Timer++;
		if( MediaObj.RSD_Timer > 35 )
		{
			MediaObj.RSD_Timer		= 0;
			MediaObj.SD_Data_Flag	= 1;    // 置重传发送多媒体信息标志位
			switch( MediaObj.Media_Type )   //   图片重传包数
			{
				case 0:                     //  图像
					Photo_sdState.SD_packetNum	= MediaObj.Media_ReSdList[MediaObj.RSD_Reader]; 
					Photo_sdState.SD_flag		= 1;
					break;
				case 1:                     // 音频
					Sound_sdState.SD_packetNum	= MediaObj.Media_ReSdList[MediaObj.RSD_Reader];
					Sound_sdState.SD_flag		= 1;
					break;
				case 2:                     // 视频
					Video_sdState.SD_packetNum	= MediaObj.Media_ReSdList[MediaObj.RSD_Reader];
					Video_sdState.SD_flag		= 1;
					break;
				default:
					break;
			}
			//-----  重传列表 递增 -----------
			MediaObj.RSD_Reader++;
			// rt_kprintf("\r\n	  MediaObj.RSD_Reader++  =%d\r\n",MediaObj.RSD_Reader);
			if( MediaObj.RSD_Reader == MediaObj.RSD_total )
			{
				MediaObj.RSD_State = 2; //  置位等待状态，等待着中心再发重传指令
			}
		}
	}else
	if( 2 == MediaObj.RSD_State )
	{
		MediaObj.RSD_Timer++;
		if( MediaObj.RSD_Timer > 140 )  //   如果状态一直在等待且超过30s择清除状态
		{
			switch( MediaObj.Media_Type )
			{
				case 0:                 // 图像
					Photo_send_end( );  // 拍照上传结束

					break;
				case 1:                 // 音频
					Sound_send_end( );
					break;
				case 2:                 // 视频
					Video_send_end( );
					break;
				default:
					break;
			}
			Media_Clear_State( );
			rt_kprintf( "\r\n 信息重传超时结束! \r\n" );

			//Check_MultiTakeResult_b4Trans();  // 多路摄像头拍照状态检测
		}
	}

    //     记录仪列表重传 定时处理
    
	if( ( 1 == Recode_Obj.RSD_State ) )
	{
		Recode_Obj.RSD_Timer++;
		if( Recode_Obj.RSD_Timer > 35 )
		{
			Recode_Obj.RSD_Timer		= 0;
            Recode_Obj.SD_Data_Flag	= 1; 

            Recode_Obj.Current_pkt_num	= Recode_Obj.Media_ReSdList[Recode_Obj.RSD_Reader];
			//  如果重传列表第一包为1  那么不传第1 包，直接跳过
			if(Recode_Obj.Current_pkt_num==1)
			 {
			    Recode_Obj.RSD_Reader++;
				rt_kprintf( "\r\n --- 重传列表第一项为第1 包 ，jump 不处理 \r\n" );
			 }
			//-----  重传列表 递增 -----------
			Recode_Obj.RSD_Reader++;
			// rt_kprintf("\r\n	  MediaObj.RSD_Reader++  =%d\r\n",MediaObj.RSD_Reader);
			if( Recode_Obj.RSD_Reader > Recode_Obj.RSD_total )
			{
				Recode_Obj.RSD_State = 2; //  置位等待状态，等待着中心再发重传指令
			}
		}
	}	    
	if( 2 == Recode_Obj.RSD_State )
	{
		Recode_Obj.RSD_Timer++;
		if( Recode_Obj.RSD_Timer > 140 )  //   如果状态一直在等待且超过30s择清除状态
		{
		   Recorder_init(0);
           rt_kprintf( "\r\n 记录仪信息重传超时结束! \r\n" );
	       if(Recode_Obj.Transmit_running==1)
        	{
        	  Rcorder_Recover(); 
              rt_kprintf( "\r\n 顺序执行尚未完成  current=%d  total=%d\r\n",Recode_Obj.Current_pkt_num,Recode_Obj.Total_pkt_num);
        	}
		}
	}

	
}

#ifdef   MEDIA
//-------- photo send -------------------------------------
void Photo_send_start( u16 Numpic )
{
//  UINT ByteRead;
	// FIL FileCurrent;

	rt_kprintf( "   \r\n  Photo_send_start =%d \r\n", Numpic );
	Photo_sdState.photo_sending		= disable;
	Photo_sdState.SD_flag			= 0;
	Photo_sdState.SD_packetNum		= 1; // 从1 开始
	Photo_sdState.Exeption_timer	= 0;

	if( Camera_Number == 1 )
	{
		PicFileSize = Api_DFdirectory_Query( camera_1, 1 );
	} else
	if( Camera_Number == 2 )
	{
		PicFileSize = Api_DFdirectory_Query( camera_2, 1 );
	} else
	if( Camera_Number == 3 )
	{
		PicFileSize = Api_DFdirectory_Query( camera_3, 1 );
	} else
	if( Camera_Number == 4 )
	{
		PicFileSize = Api_DFdirectory_Query( camera_4, 1 );
	}

	//	 DF_ReadFlash(pic_current_page, 0,PictureName, 23);
	//      Camera_Number=PictureName[18];
	//      memcpy((u8*)&PicFileSize,PictureName+19,4);
	//pic_current_page++; //指向文件内容
	//pic_PageIn_offset=0;// 图片读取页内偏移地址
	// rt_kprintf("\r\n    open Pic =%s",PictureName);

	if( PicFileSize % 512 )
	{
		Photo_sdState.Total_packetNum = PicFileSize / 512 + 1;
	} else
	{
		Photo_sdState.Total_packetNum = PicFileSize / 512;
	}

	rt_kprintf( "\r\n    Camera %d  ReadpicStart total :%d ，Pagesize: %d Bytes\r\n\r\n", Camera_Number, Photo_sdState.Total_packetNum, PicFileSize );
	if( ( Camera_Number == 0 ) || ( Photo_sdState.Total_packetNum == 0 ) )
	{
		Photo_send_end( ); // clear  state
		rt_kprintf( "\r\n  图片总包数为空 ，摄像头序号为0 ，发送拍照失败到中心 \r\n" );
	}

	// -------  MultiMedia Related --------
	Media_Start_Init( 0, 0 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Sound_send_start( void )
{
	u8	sound_name[20];
	u16 i;
//  u8  oldstate=0;
//  u16 i2,j;
	// u8  WrieEnd=0,LeftLen=0;

	Sound_sdState.photo_sending		= disable;
	Sound_sdState.SD_flag			= 0;
	Sound_sdState.SD_packetNum		= 1; // 从1 开始
	Sound_sdState.Exeption_timer	= 0;

	//---  Speex_Init();	 // speachX 初始化
	// 1. 查找最新的文件


/*  memset((u8*)&MediaIndex,0,sizeof(MediaIndex));
   for(i=0;i<8;i++)
   {
   Api_RecordNum_Read(voice_index, i+1, (u8*)&MediaIndex, sizeof(MediaIndex));
        if(MediaIndex.Effective_Flag==1)
        {
            break;
   }
   }
   if(MediaIndex.Effective_Flag)
   {
   rt_kprintf("\r\n 索引filename:%s\r\n",MediaIndex.FileName);
   }
   else
   {
      rt_kprintf("\r\n 没有已存储的音频文件 \r\n");
      return false	;
   }

 */

	//	2.	创建wav 文件

	//	3. 文件大小
	// file name
	memset( sound_name, 0, sizeof( sound_name ) );
	DF_ReadFlash( SoundStart_offdet, 4, sound_name, 20 );
	SrcFileSize = Api_DFdirectory_Query( voice, 1 );
	//  Sound_sdState.Total_packetNum=(SrcFileSize/512); // 每包100个字节
	if( SrcFileSize % 512 )
	{
		Sound_sdState.Total_packetNum = SrcFileSize / 512 + 1;
	} else
	{
		Sound_sdState.Total_packetNum = SrcFileSize / 512;
	}
	rt_kprintf( "\r\n	文件名: %s大小: %d Bytes  totalpacketnum=%d \r\n", sound_name, SrcFileSize, Sound_sdState.Total_packetNum );

	// -------  MultiMedia Related --------
	//  Media_Start_Init(1,5); // 音频  wav 格式   0:JPEG ;   1: TIF ;   2:MP3;  3:WAV  4: WMV  其他保留
	//    5   amr
	Media_Start_Init( 1, 99 ); // 有word文档要求 对于非mp3  和 wav的实用99
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  MP3_send_start( void )
{
	u8 mp3_name[13];
	mp3_sendstate					= 1;
	Sound_sdState.photo_sending		= disable;
	Sound_sdState.SD_flag			= 0;
	Sound_sdState.SD_packetNum		= 1; // 从1 开始
	Sound_sdState.Exeption_timer	= 0;

	memset( mp3_name, 0, sizeof( mp3_name ) );
	memcpy( mp3_name, "ch12.mp3", 8 );

	if( mp3_fsize % 512 )
	{
		Sound_sdState.Total_packetNum = mp3_fsize / 512 + 1;
	} else
	{
		Sound_sdState.Total_packetNum = mp3_fsize / 512;
	}

	rt_kprintf( "\r\n  mp3文件名称:%s    文件大小  %d Bytes  \r\n", mp3_name, mp3_fsize );

	// -------  MultiMedia Related --------
	Media_Start_Init( 1, 2 ); // 音频  wav 格式
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  Video_send_start( void )
{
	u8 video_name[13];

	wmv_sendstate					= 1;
	Video_sdState.photo_sending		= disable;
	Video_sdState.SD_flag			= 0;
	Video_sdState.SD_packetNum		= 1; // 从1 开始
	Video_sdState.Exeption_timer	= 0;

	memset( video_name, 0, sizeof( video_name ) );
	memcpy( video_name, "ch1.wmv", 7 );

	if( wmv_fsize % 512 )
	{
		Video_sdState.Total_packetNum = wmv_fsize / 512 + 1;
	} else
	{
		Video_sdState.Total_packetNum = wmv_fsize / 512;
	}

	rt_kprintf( "\r\n  wmv文件名称:%s    文件大小  %d Bytes  \r\n", video_name, wmv_fsize );

	// -------  MultiMedia Related --------
	Media_Start_Init( 2, 4 ); // 音频  wav 格式
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8   DrvRecoder_send_start( void )
{
	DrvRecoder_sdState.photo_sending	= disable;
	DrvRecoder_sdState.SD_flag			= 0;
	DrvRecoder_sdState.SD_packetNum		= 1;    // 从1 开始
	DrvRecoder_sdState.Exeption_timer	= 0;

	Media_Start_Init( 3, 0 );                   // 行车记录仪
	return true;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  BlindZoneData_send_start( void )
{
	BlindZone_sdState.photo_sending		= disable;
	BlindZone_sdState.SD_flag			= 0;
	BlindZone_sdState.SD_packetNum		= 1;    // 从1 开始
	BlindZone_sdState.Exeption_timer	= 0;

       BlindZone_sdState.Total_packetNum=BLIND_NUM/MQ_PKNUM;

	MediaObj.Media_Type = 4;                    // 盲区

	// Media_Start_Init(4,0); // 盲区
	return true;
}

//==========================================================
void Photo_send_end( void )
{
	Photo_sdState.photo_sending		= 0;
	Photo_sdState.SD_flag			= 0;
	Photo_sdState.SD_packetNum		= 0;
	Photo_sdState.Total_packetNum	= 0;
	Photo_sdState.Exeption_timer	= 0;
	MediaObj.Media_transmittingFlag = 0; // clear
	Media_Clear_State( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Sound_send_end( void )
{
	Sound_sdState.photo_sending		= 0;
	Sound_sdState.SD_flag			= 0;
	Sound_sdState.SD_packetNum		= 0;
	Sound_sdState.Total_packetNum	= 0;
	Sound_sdState.Exeption_timer	= 0;
	MediaObj.Media_transmittingFlag = 0;    // clear
	mp3_sendstate					= 0;
	VocREC.running					= 0;    // clear
	Media_Clear_State( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Video_send_end( void )
{
	Video_sdState.photo_sending		= 0;
	Video_sdState.SD_flag			= 0;
	Video_sdState.SD_packetNum		= 0;
	Video_sdState.Total_packetNum	= 0;
	Video_sdState.Exeption_timer	= 0;
	MediaObj.Media_transmittingFlag = 0; // clear
	wmv_sendstate					= 0;
	Media_Clear_State( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void   DrvRecoder_send_end( void )
{
	DrvRecoder_sdState.photo_sending	= disable;
	DrvRecoder_sdState.SD_flag			= 0;
	DrvRecoder_sdState.SD_packetNum		= 1;    // 从1 开始
	DrvRecoder_sdState.Exeption_timer	= 0;

	MediaObj.Media_transmittingFlag = 0;        // clear
	Media_Clear_State( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  BlindZoneData_send_end( void )
{
	BlindZone_sdState.photo_sending		= disable;
	BlindZone_sdState.SD_flag			= 0;
	BlindZone_sdState.SD_packetNum		= 1;                                        // 从1 开始
	BlindZone_sdState.Exeption_timer	= 0;

	MediaObj.Media_transmittingFlag = 0;                                            // clear
	Media_Clear_State( );
}

//=============================================
void Video_Timer( void )
{
	if( ( Video_sdState.photo_sending == enable ) && ( 2 == MediaObj.Media_Type ) ) // 视频
	{
		if( ( Video_sdState.SD_packetNum <= Video_sdState.Total_packetNum + 1 ) && ( 2 != MediaObj.RSD_State ) )
		{                                                                           //  一下定时器在	在顺序发送过过程中	 和   收到重传开始后有效
			Video_sdState.Data_SD_counter++;
			if( Video_sdState.Data_SD_counter > 40 )
			{
				Video_sdState.Data_SD_counter	= 0;
				Video_sdState.SD_flag			= 1;
				MediaObj.SD_Data_Flag			= 1;
				Video_sdState.Exeption_timer	= 0;
				//rt_kprintf("\r\n Video send Flag \r\n");
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Sound_Timer( void )
{
	if( ( Sound_sdState.photo_sending == enable ) && ( 1 == MediaObj.Media_Type ) ) // 音频
	{
		if( ( Sound_sdState.SD_packetNum <= Sound_sdState.Total_packetNum + 1 ) && ( 2 != MediaObj.RSD_State ) )
		{                                                                           //  一下定时器在	在顺序发送过过程中	 和   收到重传开始后有效
			Sound_sdState.Data_SD_counter++;
			if( Sound_sdState.Data_SD_counter > 35 )
			{
				Sound_sdState.Data_SD_counter	= 0;
				Sound_sdState.Exeption_timer	= 0;
				Sound_sdState.SD_flag			= 1;
				MediaObj.SD_Data_Flag			= 1;

				//rt_kprintf("\r\n Sound  Transmit set Flag \r\n");
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Photo_Timer( void )
{
	if( ( Photo_sdState.photo_sending == enable ) && ( 0 == MediaObj.Media_Type ) )
	{
		if( ( Photo_sdState.SD_packetNum <= Photo_sdState.Total_packetNum + 1 ) && ( 2 != MediaObj.RSD_State ) )
		{                                               //  一下定时器在   在顺序发送过过程中   和   收到重传开始后有效
			Photo_sdState.Data_SD_counter++;
			if( Photo_sdState.Data_SD_counter > 35 )    //40   12
			{
				Photo_sdState.Data_SD_counter	= 0;
				Photo_sdState.Exeption_timer	= 0;
				Photo_sdState.SD_flag			= 1;
				MediaObj.SD_Data_Flag			= 1;
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void DrvRecoder_Timer( void )
{
	if( ( DrvRecoder_sdState.photo_sending == enable ) && ( 3 == MediaObj.Media_Type ) )
	{
		if( ( DrvRecoder_sdState.SD_packetNum <= DrvRecoder_sdState.Total_packetNum + 1 ) && ( 3 != MediaObj.RSD_State ) )
		{                                                   //  一下定时器在   在顺序发送过过程中   和   收到重传开始后有效
			DrvRecoder_sdState.Data_SD_counter++;
			if( DrvRecoder_sdState.Data_SD_counter > 35 )   //40   12
			{
				DrvRecoder_sdState.Data_SD_counter	= 0;
				DrvRecoder_sdState.Exeption_timer	= 0;
				DrvRecoder_sdState.SD_flag			= 1;
				MediaObj.SD_Data_Flag				= 1;
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void BlindZone_Timer( void )
{
	if( ( BlindZone_sdState.photo_sending == enable ) && ( 4 == MediaObj.Media_Type ) )
	{
		if( ( BlindZone_sdState.SD_packetNum <= BlindZone_sdState.Total_packetNum + 1 ) && ( 4 != MediaObj.RSD_State ) )
		{                                                   //  一下定时器在   在顺序发送过过程中   和   收到重传开始后有效
			BlindZone_sdState.Data_SD_counter++;
			if( BlindZone_sdState.Data_SD_counter > 35 )    //40   12
			{
				BlindZone_sdState.Data_SD_counter	= 0;
				BlindZone_sdState.Exeption_timer	= 0;
				BlindZone_sdState.SD_flag			= 1;
				MediaObj.SD_Data_Flag				= 1;
			}
		}
	}
}

//=============================================
void Meida_Trans_Exception( void )
{
	u8 resualt = 0;

	if( Photo_sdState.photo_sending == enable )
	{
		if( Photo_sdState.Exeption_timer++ > 50 )
		{
			Photo_send_end( );
			resualt = 1;
		}
	}else
	if( Sound_sdState.photo_sending == enable )
	{
		if( Sound_sdState.Exeption_timer++ > 50 )
		{
			Sound_send_end( );
			resualt = 2;
		}
	}else
	if( Video_sdState.photo_sending == enable )
	{
		if( Video_sdState.Exeption_timer++ > 50 )
		{
			Video_send_end( );
			resualt = 2;
		}
	}else
	if( DrvRecoder_sdState.photo_sending == enable )
	{
		if( DrvRecoder_sdState.Exeption_timer++ > 50 )
		{
			DrvRecoder_send_end( );
			resualt = 2;
		}
	}else
	if( BlindZone_sdState.photo_sending == enable )
	{
		if( BlindZone_sdState.Exeption_timer++ > 50 )
		{
			BlindZoneData_send_end( );
			resualt = 2;
		}
	}

	if( resualt )
	{
		rt_kprintf( "\r\n   Media  Trans  Timeout  resualt: %d\r\n", resualt );
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Media_Timer_Service( void )
{
	//----------------------------------
	if( DataLink_Status( ) )
	{
		if( Photo_sdState.photo_sending == enable )
		{
			Photo_Timer( );
		} else
		if( Sound_sdState.photo_sending == enable )
		{
			Sound_Timer( );
		} else
		if( Video_sdState.photo_sending == enable )
		{
			Video_Timer( );
		} else
		if( DrvRecoder_sdState.photo_sending == enable )
		{
			DrvRecoder_Timer( );
		} else
		if( BlindZone_sdState.photo_sending == enable )
		{
			BlindZone_Timer( );
		}

		Media_RSdMode_Timer( );
	}
}

#endif
//------------------------------------------------------------
void DataTrans_Init( void )
{
	DataTrans.Data_RxLen	= 0;
	DataTrans.Data_TxLen	= 0;
	DataTrans.Tx_Wr			= 0;
	memset( DataTrans.DataRx, 0, sizeof( (const char*)DataTrans.DataRx ) );
	memset( DataTrans.Data_Tx, 0, sizeof( (const char*)DataTrans.Data_Tx ) );
}

//------------------------------------------------------------
void DoorCameraInit( void )
{
	DoorOpen.currentState	= 0;
	DoorOpen.BakState		= 0;
}

//-----------------------------------------------------------
void Spd_ExpInit( void )
{
	speed_Exd.current_maxSpd	= 0;
	speed_Exd.dur_seconds		= 0;
	speed_Exd.excd_status		= 0;
	memset( (char*)( speed_Exd.ex_startTime ), 0, 5 );
	speed_Exd.speed_flag = 0;
}

//-----------------------------------------------------------
void  TCP_RX_Process( u8 LinkNum )  //  ---- 808  标准协议
{
	u16 i		= 0, j = 0;         //,DF_PageAddr;
	u16 infolen = 0, contentlen = 0, msg_len = 0;
	// u8   ireg[5];
	u8	Ack_Resualt		= 1;
	u16 Ack_CMDid_8001	= 0;
	u8	Total_ParaNum	= 0;        // 中心设置参数总数
	u8	Process_Resualt = 0;        //  bit 表示   bit0 表示 1  bit 1 表示2
	u8	ContentRdAdd	= 0;        // 当前读取到的地址
	u8	SubInfolen		= 0;        // 子信息长度
	u8	Reg_buf[50];
	//u8   CheckResualt=0;
	u32 reg_u32 = 0;
	//u16  Devide_8003_packet_1stID=0; //  分包上传第一包 ID 
	  u8  devide_value=0; 
	//----------------      行车记录仪808 协议 接收处理   --------------------------

	//  0.  Decode
	Protocol_808_Decode( );
	//  1.  fliter head
	if( UDP_HEX_Rx[0] != 0x7e )                                 //   过滤头
	{
		return;
	}
	//  2.  check Centre Ack
	Centre_CmdID	= ( UDP_HEX_Rx[1] << 8 ) + UDP_HEX_Rx[2];   // 接收到中心消息ID
	Centre_FloatID	= ( UDP_HEX_Rx[11] << 8 ) + UDP_HEX_Rx[12]; // 接收到中心消息流水号

	//  分包判断
	if( UDP_HEX_Rx[3] & 0x20 )
	{                                                           //  分包判断
		;
	}

	//  3.   get infolen    ( 长度为消息体的长度)    不分包的话  消息头长度为12 则参与计算校验的长度 =infolen+12
	infolen		= ( u16 )( ( UDP_HEX_Rx[3] & 0x03 ) << 8 ) + ( u16 )UDP_HEX_Rx[4];
	contentlen	= infolen + 12;                                 //  参与校验字节的长度

	//  4.   Check  Fcs
	FCS_RX_UDP = 0;
	//nop;nop;
	for( i = 0; i < ( UDP_DecodeHex_Len - 3 ); i++ )            //先算出收到数据的异或和
	{
		FCS_RX_UDP ^= UDP_HEX_Rx[1 + i];
	}
	//nop;
	// ------- FCS filter -----------------
#if  0                                                          // 认证专用，暂时 屏蔽掉判断校验
	if( UDP_HEX_Rx[UDP_DecodeHex_Len - 2] != FCS_RX_UDP )       //判断校验结果
	{
		rt_kprintf( "\r\n  infolen=%d ", infolen );
		rt_kprintf( "\r\n808协议校验错误	  Caucate %x  ,RX  %x\r\n", FCS_RX_UDP, UDP_HEX_Rx[UDP_DecodeHex_Len - 2] );
		//-----------------  memset  -------------------------------------
		memset( UDP_HEX_Rx, 0, sizeof( UDP_HEX_Rx ) );
		UDP_hexRx_len = 0;
		return;
	}
#endif
	//  else
	// rt_kprintf("\r\n 808协议校验正确	  Caucate %x  ,RX  %x\r\n",FCS_RX_UDP,UDP_HEX_Rx[UDP_DecodeHex_Len-2]);

	//   5 .  Classify  Process
	rt_kprintf( "\r\n           CentreCMD = 0x%X  \r\n", Centre_CmdID );    // add for  debug

	switch( Centre_CmdID )
	{
		case  0x8001:                                                       //平台通用应答
			// 若没有分包处理的话  消息头长12  从0开始计算第12个字节是消息体得主体

			//  13 14  对应的终端消息流水号
			//  15 16  对应终端的消息
			Ack_CMDid_8001 = ( UDP_HEX_Rx[15] << 8 ) + UDP_HEX_Rx[16];

			switch( Ack_CMDid_8001 )                                        // 判断对应终端消息的ID做区分处理
			{
				case 0x0200:                                                //  对应位置消息的应答
					//  rt_kprintf( "\r\n  0200-1");
					//----- 判断确认是否成功
					//  if(UDP_HEX_Rx[17]!=0x00)
					//	{     rt_kprintf( "\r\n  0200-2"); break;}

					//----- app msg ----------
					if( ( AppQue.sd_enable_flag == 2 ) && ( app_que_enable == 1 ) )
					{
						AppQue.sd_enable_flag = 0;
						rt_kprintf( "\r\n packet= %d  app_msg-ack", AppQue.read_num );
						AppQue.read_num++;
						if( AppQue.read_num >= 420 )
						{
							app_que_enable = 0;
							app_queenable( "0" );
							rt_kprintf( "\r\n DWJD over  =>return normal state\r\n" );
						}
					}
					//----------------------------------------
					if( Warn_Status[1] & 0x10 )         // 进出区域报警
					{
						InOut_Object.TYPE		= 0;    //圆形区域
						InOut_Object.ID			= 0;    //  ID
						InOut_Object.InOutState = 0;    //  进报警
						Warn_Status[1]			&= ~0x10;
						rt_kprintf( "\r\n进出--- 区域----报警清除!\r\n" );
					}

#ifndef   NEW_8203
					//--------------------------------
					if( Warn_Status[3] & 0x01 )         //紧急报警
					{
						StatusReg_WARN_Clear( );
						f_Exigent_warning	= 0;
						warn_flag			= 0;
						Send_warn_times		= 0;
						StatusReg_WARN_Clear( );
						rt_kprintf( "\r\n紧急报警收到应答，得清除!\r\n" );
					}
					if( Warn_Status[3] & 0x08 )         //危险报警-BD
					{
						Warn_Status[3] &= ~0x08;
					}

					//------------------------------------
					if( Warn_Status[1] & 0x10 )         // 进出区域报警
					{
						InOut_Object.TYPE		= 0;    //圆形区域
						InOut_Object.ID			= 0;    //  ID
						InOut_Object.InOutState = 0;    //  进报警
						Warn_Status[1]			&= ~0x10;
						rt_kprintf( "\r\n进出--- 区域----报警清除!\r\n" );
					}


					/* if(Warn_Status[1]&0x20)// 进出路线报警
					                                {
					                   InOut_Object.TYPE=0;//圆形区域
					                   InOut_Object.ID=0; //  ID
					                   InOut_Object.InOutState=0;//  进报警
					                   Warn_Status[1]&=~0x20;
					       rt_kprintf( "\r\n进出-----路线----报警清除!\r\n");
					                                } */
					if( Warn_Status[1] & 0x40 )         // 路段行驶时间不足、过长
					{
						Warn_Status[1] &= ~0x40;
					}
					//-----------------------------------------
					if( Warn_Status[0] & 0x08 )         //车辆非法点火
					{
						Warn_Status[0] &= ~0x08;
					}
					if( Warn_Status[0] & 0x10 )         //车辆非法位移
					{
						Warn_Status[0] &= ~0x10;
					}
					if( Warn_Status[0] & 0x80 )         //非法开门报警(终端未设置区域时，不判断非法开门)
					{
						Warn_Status[0] &= ~0x80;
					}
					if( Warn_Status[0] & 0x60 )
					{
						Warn_Status[0] &= ~0x60;        // 2 个bit    碰撞和侧翻
					}
#endif
					//------------------------------------
					rt_kprintf( "\r\nCentre ACK!\r\n" );
					
					//-------------------------------------------------------------------
					Api_cycle_Update();

					//--------------  多媒体上传相关  --------------
					if( MediaObj.Media_transmittingFlag == 1 ) // clear
					{
						MediaObj.Media_transmittingFlag = 2;
						if( Duomeiti_sdFlag == 1 )
						{
							Duomeiti_sdFlag = 0;
							Media_Clear_State( );
							Photo_send_end( );
							Sound_send_end( );
							Video_send_end( );
							rt_kprintf( "\r\n  手动上报多媒体上传处理\r\n" );
						}
						rt_kprintf( "\r\n  多媒体信息前的多媒体发送完毕 \r\n" );
					}

					break;
				case 0x0002:                                            //  心跳包的应答
					//  不用判结果了  ---
					JT808Conf_struct.DURATION.TCP_ACK_DurCnter	= 0;    //clear
					JT808Conf_struct.DURATION.TCP_SD_state		= 0;    //clear
					rt_kprintf( "\r\n  Centre  Heart ACK!\r\n" );
					break;
				case 0x0101:                                            //  终端注销应答
					if( 0 == UDP_HEX_Rx[17] )
					{                                                   // 注销成功
						memset( Reg_buf, 0, sizeof( Reg_buf ) );
						memcpy( Reg_buf, JT808Conf_struct.ConfirmCode, 20 );
						JT808Conf_struct.Regsiter_Status	= 0;
						Reg_buf[20]							= JT808Conf_struct.Regsiter_Status;
						Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
						rt_kprintf( "\r\n  终端注销成功!  \r\n" );
					}

					break;
				case 0x0102:                            //  终端鉴权

					rt_kprintf( "\r\n 收到鉴权结果: %x \r\n", UDP_HEX_Rx[17] );
					if( 0 == UDP_HEX_Rx[17] )
					{                                   // 鉴权成功
						DEV_Login.Operate_enable = 2;   // 鉴权完成
						if( DataLink_Status( ) )
						{
							DataLinkOK_Process( );
						}
						rt_kprintf( "\r\n  终端鉴权成功!  \r\n" );
					}
					break;
				case 0x0800:                // 多媒体事件信息上传
					rt_kprintf( "\r\n 多媒体事件信息上传回应! \r\n" );
					Media_Clear_State( );   //  clear

					if( 0 == MediaObj.Media_Type )
					{
						MediaObj.Media_transmittingFlag = 1;
						PositionSD_Enable( );
						Current_UDP_sd = 1;

						Photo_sdState.photo_sending = enable;
						Photo_sdState.SD_packetNum	= 1;    // 第一包开始
						PositionSD_Enable( );               //   使能上报
						rt_kprintf( "\r\n 开始上传照片! ....\r\n" );
					}else
					if( 1 == MediaObj.Media_Type )
					{
						MediaObj.Media_transmittingFlag = 1;

						Sound_sdState.photo_sending = enable;
						Sound_sdState.SD_packetNum	= 1;    // 第一包开始
						PositionSD_Enable( );               //   使能上报
						Current_UDP_sd = 1;
						rt_kprintf( "\r\n 开始上传音频! ....\r\n" );
					}else
					if( 2 == MediaObj.Media_Type )
					{
						MediaObj.Media_transmittingFlag = 1;
						PositionSD_Enable( );               //   使能上报
						Current_UDP_sd				= 1;
						Video_sdState.photo_sending = enable;
						Video_sdState.SD_packetNum	= 1;    // 第一包开始
						rt_kprintf( "\r\n 开始上传视频! ....\r\n" );
					}
					break;
				case 0x0702:
					rt_kprintf( "\r\n  驾驶员信息上报---中心应答!  \r\n" );

					break;
				case 0x0701:
					rt_kprintf( "\r\n  电子运单上报---中心应答!  \r\n" );

					break;
				case 0x0705:    //
					rt_kprintf( "\r\n can-ack" );
					break;
				case 0x0704:    // 批量应答
					rt_kprintf( "\r\n MQ packet= %d   0704H-ack", MangQU.PacketNum );
					                   //  本地存储
                                          if(MangQU.Sd_flag==2)
										  { 	  
										       MangQU.Sd_flag=0;
											   MangQU.Sd_timer=0;
											   MangQU.Sd_timer=0;
											   
											   MangQU.PacketNum++;
											   if(MangQU.PacketNum>=(BLIND_NUM/MQ_PKNUM)) 
									  	    	{
									  	    	   MangQU.PacketNum=0;
												   MangQU.Enable_SD_state=0; 
												   rt_kprintf("\r\nReturn Normal\r\n");  
									  	    	}	
                                          }
										//  真正 存储
										    if((MQ_TrueUse.Sd_flag==2)&&(MQ_TrueUse.Enable_SD_state==1)) 
										  { 	  
										       MQ_TrueUse.Sd_flag=0; 
											   MQ_TrueUse.Sd_timer=0;
											   MQ_TrueUse.Sd_timer=0;
											   
											   MQ_TrueUse.PacketNum++;
											   if(MQ_TrueUse.PacketNum>CurrentTotal)  
									  	    	{
									  	    	   MQ_TrueUse.PacketNum=0;
												   MQ_TrueUse.Enable_SD_state=0; 
												   // 每次应答存储相关记录数目
												   DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd);  
												   rt_kprintf("\r\nTrue Return Normal\r\n");  
									  	    	}	
                                          }
							           break; 		   

				default:
					break; 
			}

			//-------------------------------
			break;
		case  0x8100: //  监控中心对终端注册消息的应答
			//-----------------------------------------------------------
			switch( UDP_HEX_Rx[15] )
			{
				case 0: rt_kprintf( "\r\n   ----注册成功\r\n" );
					memset( JT808Conf_struct.ConfirmCode, 0, sizeof( JT808Conf_struct.ConfirmCode ) );
					memcpy( JT808Conf_struct.ConfirmCode, UDP_HEX_Rx + 16, infolen - 3 );

					memset( Reg_buf, 0, sizeof( Reg_buf ) );
					memcpy( Reg_buf, JT808Conf_struct.ConfirmCode, 20 );
					JT808Conf_struct.Regsiter_Status	= 1;
					Reg_buf[20]							= JT808Conf_struct.Regsiter_Status;
					Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
					rt_kprintf( "鉴权码: %s\r\n		   鉴权码长度: %d\r\n", JT808Conf_struct.ConfirmCode, strlen( (const char*)JT808Conf_struct.ConfirmCode ) );
					//-------- 开始鉴权 ------
					DEV_Login.Operate_enable = 1;
					break;
				case 1: rt_kprintf( "\r\n   ----车辆已被注册\r\n" );
					break;
				case 2: rt_kprintf( "\r\n   ----数据库中无该车辆\r\n" );
					break;
				case 3: rt_kprintf( "\r\n   ----终端已被注册\r\n" );
					if( 0 == JT808Conf_struct.Regsiter_Status )
					{
						;                               //JT808Conf_struct.Regsiter_Status=2;  // not  1
						                                //DEV_regist.DeRegst_sd=1;
					}else
					if( 1 == JT808Conf_struct.Regsiter_Status )
					{
						DEV_Login.Operate_enable = 1;   //开始鉴权
					}
					break;
				case 4: rt_kprintf( "\r\n   ----数据库中无该终端\r\n" );
					break;
			}
			break;
		case  0x8103:                                   //  设置终端参数
			//  Ack_Resualt=0;
			if( contentlen )
			{                                           // 和中心商议好了每次只下发操作一项设置
				Total_ParaNum = UDP_HEX_Rx[13];         // 中心设置参数总数
				rt_kprintf( "\r\n Set ParametersNum =%d  \r\n", Total_ParaNum );
				//-------------------------------------------------------------------
				ContentRdAdd	= 14;
				Process_Resualt = 0;                    // clear resualt
				for( i = 0; i < Total_ParaNum; i++ )
				{
					//  数类型是DWORD 4 个字节
					SubCMD_8103H = ( UDP_HEX_Rx[ContentRdAdd] << 24 ) + ( UDP_HEX_Rx[ContentRdAdd + 1] << 16 ) + ( UDP_HEX_Rx[ContentRdAdd + 2] << 8 ) + UDP_HEX_Rx[ContentRdAdd + 3];
					//  子信息长度
					SubInfolen = UDP_HEX_Rx[ContentRdAdd + 4];
					//  处理子信息 如果设置成功把相应Bit 位置为 1 否则保持 0
					if( CentreSet_subService_8103H( SubCMD_8103H, SubInfolen, UDP_HEX_Rx + ContentRdAdd + 5 ) )
					{
						Process_Resualt |= ( 0x01 << i );
					}
					//  移动偏移地址
					ContentRdAdd += 5 + UDP_HEX_Rx[ContentRdAdd + 4]; // 偏移下标
				}

				//--------------判断所有的设置结果  ---------------


				/* for(i=0;i<Total_ParaNum;i++)
				   {
				     if(!((Process_Resualt>>0)&0x01))
				   {
				          Ack_Resualt=1; //  1  表示失败
				          break;
				   }
				   if(i==(Total_ParaNum-1))  //  设置到最后一个确认成功
				   Ack_Resualt=0;  //  成功/确认
				   }*/
				Ack_Resualt = 0;
			}

			//-------------------------------------------------------------------
			if( SD_ACKflag.f_CentreCMDack_0001H == 2 )
			{
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}else
			if( SD_ACKflag.f_CentreCMDack_0001H == 0 )
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			rt_kprintf( "\r\n  Set Device !\r\n" );

			break;
		case  0x8104:                           //  查询终端参数
			SD_ACKflag.f_SettingPram_0104H = 1; // 不管什么内容回复统一结果
			rt_kprintf( "\r\n  中心查询终端参数 !\r\n" );
			break;
		case  0x8105:                           // 终端控制
			// Ack_Resualt=0;
			rt_kprintf( "\r\ny  终端控制 -1!\r\n" );
			if( contentlen )
			{                                   // 和中心商议好了每次只下发操作一项设置
				Total_ParaNum = UDP_HEX_Rx[13]; //  终端控制命令字
				rt_kprintf( "\r\n Set ParametersNum =%d  \r\n", Total_ParaNum );
				//-------------------------------------------------------------------
				if( CentreSet_subService_8105H( Total_ParaNum, contentlen - 1, UDP_HEX_Rx + 14 ) )
				{
					Ack_Resualt = 0;            // 返回成功
				}
			}

			//-------------------------------------------------------------------
			Ack_Resualt = 0;
			if( SD_ACKflag.f_CentreCMDack_0001H == 0 )
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			rt_kprintf( "\r\ny  终端控制 -1!\r\n" );

			break;
		case  0x8201:   // 位置信息查询    位置信息查询消息体为空
			SD_ACKflag.f_CurrentPosition_0201H = 1;
			rt_kprintf( "\r\n  位置信息查询 !\r\n" );
			break;
		case  0x8202:   // 临时位置跟踪控制
			Ack_Resualt = 0;

			//  13 14  时间间隔
			JT808Conf_struct.RT_LOCK.Lock_Dur = ( UDP_HEX_Rx[13] << 8 ) + UDP_HEX_Rx[14];
			//  15 16  17 18 对应终端的消息
			JT808Conf_struct.RT_LOCK.Lock_KeepDur = ( UDP_HEX_Rx[15] << 24 ) + ( UDP_HEX_Rx[16] << 16 ) + ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];

			JT808Conf_struct.RT_LOCK.Lock_state			= 1;                                        // Enable Flag
			JT808Conf_struct.RT_LOCK.Lock_KeepCnter		= 0;                                        //  保持计数器
			Current_SD_Duration							= JT808Conf_struct.RT_LOCK.Lock_KeepDur;    //更改发送间隔
			JT808Conf_struct.SD_MODE.DUR_TOTALMODE		= 1;                                        // 更新定时相关状态位
			JT808Conf_struct.SD_MODE.Dur_DefaultMode	= 1;
			//  保存配置
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			rt_kprintf( "\r\n  临时位置跟踪控制!\r\n" );
			break;
		case  0x8300:                       //  文本信息下发
			Ack_Resualt			= 0;
			TextInfo.TEXT_FLAG	= UDP_HEX_Rx[13];
			if( TextInfo.TEXT_FLAG & 0x09 ) // 检测是否给TTS终端  ，紧急也给TTS播报
			{
				Dev_Voice.CMD_Type = '2';
				memset( Dev_Voice.Play_info, 0, sizeof( Dev_Voice.Play_info ) );
				memcpy( Dev_Voice.Play_info, UDP_HEX_Rx + 14, infolen - 1 );
				Dev_Voice.info_sdFlag = 1;

				//#ifdef LCD_5inch
				//DwinLCD.Type=LCD_SDTXT;
				// memset(DwinLCD.TXT_content,0,sizeof(DwinLCD.TXT_content));
				//DwinLCD.TXT_contentLen=AsciiToGb(DwinLCD.TXT_content,infolen-1,UDP_HEX_Rx+14);
				//#endif
				//  TTS
				TTS_Get_Data( UDP_HEX_Rx + 14, infolen - 1 );
			}
			if( ( TextInfo.TEXT_FLAG & 0x04 ) || ( TextInfo.TEXT_FLAG & 0x01 ) )    // 检测是否给终端显示器
			{
				memset( TextInfo.TEXT_Content, 0, sizeof( TextInfo.TEXT_Content ) );
				memcpy( TextInfo.TEXT_Content, UDP_HEX_Rx + 14, infolen - 1 );
				TextInfo.TEXT_SD_FLAG = 1;                                          // 置发送给显示屏标志位  // ||||||||||||||||||||||||||||||||||

				//========================================
				TextInforCounter++;
				rt_kprintf( "\r\n写入收到的第 %d 条信息,消息长度=%d,消息:%s", TextInforCounter, infolen - 1, TextInfo.TEXT_Content );
				TEXTMSG_Write( TextInforCounter, 1, infolen - 1, TextInfo.TEXT_Content );
				//========================================
			}

#ifdef LCD_5inch
			//======  信息都在屏幕上显示
			DwinLCD.Type = LCD_SDTXT;
			memset( DwinLCD.TXT_content, 0, sizeof( DwinLCD.TXT_content ) );
			DwinLCD.TXT_contentLen = AsciiToGb( DwinLCD.TXT_content, infolen - 1, UDP_HEX_Rx + 14 );
#endif

			//------- 返回 ----
			//  if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			rt_kprintf( "\r\n 文本信息: %s\r\n", TextInfo.TEXT_Content );
			break;
		case  0x8301:                                   //  事件设置
			if( contentlen )
			{
				//--- 设置类型--
				switch( UDP_HEX_Rx[13] )
				{
					case 0:                             //  删除终端现有所有事件，该命令后不带后继字符
						Event_Init( 1 );
						rt_kprintf( "\r\n 删除所有事件\r\n" );
						break;
					case 1:                             // 更新事件
						if( UDP_HEX_Rx[13] == 1 )
						{
							rt_kprintf( "\r\n 更新事件\r\n" );
						}
					//break;
					case 2:                             // 追加事件
						if( UDP_HEX_Rx[13] == 2 )
						{
							rt_kprintf( "\r\n 追加事件\r\n" );
						}
					//break;
					case 3:                             // 修改事件
						if( UDP_HEX_Rx[13] == 3 )
						{
							rt_kprintf( "\r\n 修改事件\r\n" );
						}
					//break;
					case 4:                             // 删除特定事件
						if( UDP_HEX_Rx[13] == 4 )
						{
							rt_kprintf( "\r\n 删除特点事件\r\n" );
						}
						Total_ParaNum = UDP_HEX_Rx[14]; // 中心设置参数总数
						rt_kprintf( "\r\n Set ParametersNum(事件信息项总数) =%d  \r\n", Total_ParaNum );
						if( Total_ParaNum == 1 )
						{
							if( ( UDP_HEX_Rx[15] > 8 ) || ( UDP_HEX_Rx[15] == 0 ) )
							{
								EventObj.Event_ID = 0;
							} else
							{
								EventObj.Event_ID = UDP_HEX_Rx[15];
							}

							EventObj.Event_Len = UDP_HEX_Rx[16];
							memset( EventObj.Event_Str, 0, sizeof( EventObj.Event_Str ) );
							memcpy( EventObj.Event_Str, UDP_HEX_Rx + 17, EventObj.Event_Len );
							EventObj.Event_Effective = 1;
							Api_RecordNum_Write( event_808, EventObj.Event_ID, (u8*)&EventObj, sizeof( EventObj ) );
							rt_kprintf( "\r\n(事件内容1)ID=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", EventObj.Event_ID, EventObj.Event_Len, EventObj.Event_Effective, EventObj.Event_Str );
						}else
						{
							EventObj.Event_ID	= UDP_HEX_Rx[15];
							EventObj.Event_Len	= UDP_HEX_Rx[16];
							memset( EventObj.Event_Str, 0, sizeof( EventObj.Event_Str ) );
							memcpy( EventObj.Event_Str, UDP_HEX_Rx + 17, EventObj.Event_Len );
							EventObj.Event_Effective = 1;
							Api_RecordNum_Write( event_808, EventObj.Event_ID, (u8*)&EventObj, sizeof( EventObj ) );
							rt_kprintf( "\r\n(事件内容2)ID=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", EventObj.Event_ID, EventObj.Event_Len, EventObj.Event_Effective, EventObj.Event_Str );
							//
							EventObj.Event_ID	= UDP_HEX_Rx[17 + EventObj.Event_Len];
							EventObj.Event_Len	= UDP_HEX_Rx[18 + EventObj.Event_Len];
							memset( EventObj.Event_Str, 0, sizeof( EventObj.Event_Str ) );
							memcpy( EventObj.Event_Str, UDP_HEX_Rx + 19, EventObj.Event_Len );
							EventObj.Event_Effective = 1;
							Api_RecordNum_Write( event_808, EventObj.Event_ID, (u8*)&EventObj, sizeof( EventObj ) );
							rt_kprintf( "\r\n(事件内容2)ID=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", EventObj.Event_ID, EventObj.Event_Len, EventObj.Event_Effective, EventObj.Event_Str );
						}
						break;
					default:
						break;
				}

				//---------返回 -------
				// if(SD_ACKflag.f_CentreCMDack_0001H==0) // 一般回复
				{
					SD_ACKflag.f_CentreCMDack_0001H		= 1;
					Ack_Resualt							= 0;
					SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
				}


				/*
				   if(SD_ACKflag.f_CurrentEventACK_0301H==0)
				   {
				   SD_ACKflag.f_CurrentEventACK_0301H=1;
				   }
				 */
			}

			break;
		case  0x8302: // 提问下发
			// if(UDP_HEX_Rx[13]&0x08)  // 检测标志是否给显示终端
			rt_kprintf( "\r\n  中心下发提问 \r\n" );
			{
				ASK_Centre.ASK_infolen = UDP_HEX_Rx[14];
				memset( ASK_Centre.ASK_info, 0, sizeof( ASK_Centre.ASK_info ) );
				memcpy( ASK_Centre.ASK_info, UDP_HEX_Rx + 15, ASK_Centre.ASK_infolen );
				rt_kprintf( "\r\n  问题: %s \r\n", ASK_Centre.ASK_info );
				memset( ASK_Centre.ASK_answer, 0, sizeof( ASK_Centre.ASK_answer ) );
				memcpy( ASK_Centre.ASK_answer, UDP_HEX_Rx + 15 + ASK_Centre.ASK_infolen, infolen - 2 - ASK_Centre.ASK_infolen );

				ASK_Centre.ASK_SdFlag		= 1;                // ||||||||||||||||||||||||||||||||||
				ASK_Centre.ASK_floatID		= Centre_FloatID;   // 备份 FloatID
				ASK_Centre.ASK_disp_Enable	= 1;
				rt_kprintf( "\r\n 提问Answer:%s\r\n", ASK_Centre.ASK_answer + 3 );

				Api_RecordNum_Write( ask_quesstion, 1, (u8*)&ASK_Centre, sizeof( ASK_Centre ) );
			}

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}

			break;
		case  0x8303:                               //  信息点播菜单设置
			                                        //--- 设置类型--
			switch( UDP_HEX_Rx[13] )
			{
				case 0:                             //  删除终端现有所有信息
					MSG_BroadCast_Init( 1 );
					rt_kprintf( "\r\n 删除信息\r\n" );
					break;
				case 1:                             // 更新菜单
					if( UDP_HEX_Rx[13] == 1 )
					{
						rt_kprintf( "\r\n 更新菜单\r\n" );
					}
				//break;
				case 2:                             // 追加菜单
					if( UDP_HEX_Rx[13] == 2 )
					{
						rt_kprintf( "\r\n 追加菜单\r\n" );
					}
				//break;
				case 3:                             // 修改菜单
					if( UDP_HEX_Rx[13] == 3 )
					{
						rt_kprintf( "\r\n 修改菜单\r\n" );
					}
					Total_ParaNum = UDP_HEX_Rx[14]; // 消息项总数
					rt_kprintf( "\r\n Set ParametersNum =%d(信息项总数)\r\n", Total_ParaNum );
					if( Total_ParaNum == 1 )        //追加1条信息
					{
						if( ( UDP_HEX_Rx[15] > 8 ) || ( UDP_HEX_Rx[15] == 0 ) )
						{
							MSG_BroadCast_Obj.INFO_TYPE = 0;
						} else
						{
							MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[15];
							MSG_BroadCast_Obj.INFO_LEN	= ( UDP_HEX_Rx[16] << 8 ) + UDP_HEX_Rx[17];
							memset( MSG_BroadCast_Obj.INFO_STR, 0, sizeof( MSG_BroadCast_Obj.INFO_STR ) );
							memcpy( MSG_BroadCast_Obj.INFO_STR, UDP_HEX_Rx + 18, MSG_BroadCast_Obj.INFO_LEN );
							MSG_BroadCast_Obj.INFO_Effective	= 1;
							MSG_BroadCast_Obj.INFO_PlyCancel	= 1;
							Api_RecordNum_Write( msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8*)&MSG_BroadCast_Obj, sizeof( MSG_BroadCast_Obj ) );
							rt_kprintf( "\r\n(信息点播内容1)TYPE=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", MSG_BroadCast_Obj.INFO_TYPE, MSG_BroadCast_Obj.INFO_LEN, MSG_BroadCast_Obj.INFO_Effective, MSG_BroadCast_Obj.INFO_STR );
						}
					}else
					{
						if( ( UDP_HEX_Rx[15] > 8 ) || ( UDP_HEX_Rx[15] == 0 ) )
						{
							MSG_BroadCast_Obj.INFO_TYPE = 0;
							rt_kprintf( "\r\n(信息点播内容2) type=%d", UDP_HEX_Rx[15] );
						}else
						{
							MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[15];
							MSG_BroadCast_Obj.INFO_LEN	= ( UDP_HEX_Rx[16] << 8 ) + UDP_HEX_Rx[17];
							msg_len						= MSG_BroadCast_Obj.INFO_LEN;
							memset( MSG_BroadCast_Obj.INFO_STR, 0, sizeof( MSG_BroadCast_Obj.INFO_STR ) );
							memcpy( MSG_BroadCast_Obj.INFO_STR, UDP_HEX_Rx + 18, msg_len );
							MSG_BroadCast_Obj.INFO_Effective	= 1;
							MSG_BroadCast_Obj.INFO_PlyCancel	= 1;
							Api_RecordNum_Write( msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8*)&MSG_BroadCast_Obj, sizeof( MSG_BroadCast_Obj ) );
							rt_kprintf( "\r\n(信息点播内容2)TYPE=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", MSG_BroadCast_Obj.INFO_TYPE, MSG_BroadCast_Obj.INFO_LEN, MSG_BroadCast_Obj.INFO_Effective, MSG_BroadCast_Obj.INFO_STR );
						}
						if( ( UDP_HEX_Rx[18 + msg_len] > 8 ) || ( UDP_HEX_Rx[18 + msg_len] == 0 ) )
						{
							MSG_BroadCast_Obj.INFO_TYPE = 0;
							rt_kprintf( "\r\n(信息点播内容2) type=%d", UDP_HEX_Rx[18 + msg_len] );
						}else
						{
							MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[18 + msg_len];
							MSG_BroadCast_Obj.INFO_LEN	= ( UDP_HEX_Rx[19 + msg_len] << 8 ) + UDP_HEX_Rx[20 + msg_len];
							memset( MSG_BroadCast_Obj.INFO_STR, 0, sizeof( MSG_BroadCast_Obj.INFO_STR ) );
							memcpy( MSG_BroadCast_Obj.INFO_STR, UDP_HEX_Rx + 21 + msg_len, MSG_BroadCast_Obj.INFO_LEN );
							MSG_BroadCast_Obj.INFO_Effective	= 1;
							MSG_BroadCast_Obj.INFO_PlyCancel	= 1;
							Api_RecordNum_Write( msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8*)&MSG_BroadCast_Obj, sizeof( MSG_BroadCast_Obj ) );
							rt_kprintf( "\r\n(信息点播内容2)TYPE=%d,Len=%d,有效性=%d(1有效)内容:%s\r\n", MSG_BroadCast_Obj.INFO_TYPE, MSG_BroadCast_Obj.INFO_LEN, MSG_BroadCast_Obj.INFO_Effective, MSG_BroadCast_Obj.INFO_STR );
						}
					}
					break;
				default:
					break;
			}

			//---------返回 -------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0) // 一般回复
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}


			/*
			     if(SD_ACKflag.f_MsgBroadCast_0303H==0)
			   {
			   SD_ACKflag.f_MsgBroadCast_0303H=1;
			     }
			 */
			break;
		case  0x8304:                                       //  信息服务
			Ack_Resualt					= 0;
			MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[13];   //  信息类型
			MSG_BroadCast_Obj.INFO_LEN	= ( UDP_HEX_Rx[14] << 8 ) + UDP_HEX_Rx[15];
			memset( MSG_BroadCast_Obj.INFO_STR, 0, sizeof( MSG_BroadCast_Obj.INFO_STR ) );
			memcpy( MSG_BroadCast_Obj.INFO_STR, UDP_HEX_Rx + 16, infolen - 3 );

			MSG_BroadCast_Obj.INFO_SDFlag = 1;              // ||||||||||||||||||||||||||||||||||
			//------------------------------


			/*  Dev_Voice.CMD_Type='2';
			   memset(Dev_Voice.Play_info,0,sizeof(Dev_Voice.Play_info));
			                        memcpy(Dev_Voice.Play_info,UDP_HEX_Rx+16,infolen-3);
			   Dev_Voice.info_sdFlag=1;
			 */
			rt_kprintf( "\r\n 信息服务内容:%s\r\n", Dev_Voice.Play_info );

			// --------  发送给文本信息  --------------
			memset( TextInfo.TEXT_Content, 0, sizeof( TextInfo.TEXT_Content ) );
			memcpy( TextInfo.TEXT_Content, UDP_HEX_Rx + 16, infolen - 3 );
			TextInfo.TEXT_SD_FLAG = 1; // 置发送给显示屏标志位	// ||||||||||||||||||||||||||||||||||

			//------- 返回 ----
			//  if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8400:                   //  电话回拨

			if( infolen == 0 )
			{
				break;
			}
			if( 0 == UDP_HEX_Rx[13] )   // 普通通话
			{
				Speak_ON; rt_kprintf( "\r\n   电话回拨-->普通通话\r\n" );
			}else
			if( 1 == UDP_HEX_Rx[13] )   //  监听
			{
				Speak_OFF; rt_kprintf( "\r\n   电话回拨-->监听" );
			}else
			{
				break;
			}
			memset( JT808Conf_struct.LISTEN_Num, 0, sizeof( JT808Conf_struct.LISTEN_Num ) );
			memcpy( JT808Conf_struct.LISTEN_Num, UDP_HEX_Rx + 14, infolen - 1 );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			CallState = CallState_rdytoDialLis; // 准备开始拨打监听号码

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				SD_ACKflag.f_CentreCMDack_resualt	= 0;
			}
			break;
		case  0x8401:                                           //   设置电话本

			switch( UDP_HEX_Rx[13] )
			{
				case 0:                                         //  删除终端现有所有信息
					// PhoneBook_Init(1);
					rt_kprintf( "\r\n 删除电话本\r\n" );
					break;
				case 1:                                         // 更新菜单
					if( UDP_HEX_Rx[13] == 1 )
					{
						rt_kprintf( "\r\n 更新电话本\r\n" );
					}
				case 3:                                         // 修改菜单
					if( UDP_HEX_Rx[13] == 3 )
					{
						rt_kprintf( "\r\n 修改电话本\r\n" );
					}
					Rx_PhoneBOOK.CALL_TYPE	= UDP_HEX_Rx[15];   // 标志 ，呼入呼出
					Rx_PhoneBOOK.NumLen		= UDP_HEX_Rx[16];
					memset( Rx_PhoneBOOK.NumberStr, 0, sizeof( Rx_PhoneBOOK.NumberStr ) );
					memcpy( Rx_PhoneBOOK.NumberStr, UDP_HEX_Rx + 17, Rx_PhoneBOOK.NumLen );
					Rx_PhoneBOOK.UserLen = UDP_HEX_Rx[17 + Rx_PhoneBOOK.NumLen];
					memset( Rx_PhoneBOOK.UserStr, 0, sizeof( Rx_PhoneBOOK.UserStr ) );
					memcpy( Rx_PhoneBOOK.UserStr, UDP_HEX_Rx + 18 + Rx_PhoneBOOK.NumLen, Rx_PhoneBOOK.UserLen );

					for( i = 0; i < 8; i++ )
					{
						PhoneBook.CALL_TYPE = 2;                //类型定义为输出
						PhoneBook.NumLen	= 0;                // 号码长度
						memset( PhoneBook.NumberStr, 0, sizeof( PhoneBook.NumberStr ) );
						PhoneBook.UserLen = 0;
						memset( PhoneBook.UserStr, 0, sizeof( PhoneBook.UserStr ) );
						Api_RecordNum_Read( phonebook, i + 1, (u8*)&PhoneBook, sizeof( PhoneBook ) );
						if( strncmp( (char*)PhoneBook.UserStr, (const char*)Rx_PhoneBOOK.UserStr, Rx_PhoneBOOK.UserLen ) == 0 )
						{                                       // 找到相同名字的把以前的删除用新的代替
							Api_RecordNum_Write( phonebook, i + 1, (u8*)&Rx_PhoneBOOK, sizeof( Rx_PhoneBOOK ) );
							break;                              // 跳出for
						}
					}
					break;
				case 2:                                         // 追加菜单
					if( UDP_HEX_Rx[13] == 2 )
					{
						rt_kprintf( "\r\n 追加电话本\r\n" );
					}
					Rx_PhoneBOOK.CALL_TYPE	= UDP_HEX_Rx[15];   // 标志 ，呼入呼出
					Rx_PhoneBOOK.NumLen		= UDP_HEX_Rx[16];
					memset( Rx_PhoneBOOK.NumberStr, 0, sizeof( Rx_PhoneBOOK.NumberStr ) );
					memcpy( Rx_PhoneBOOK.NumberStr, UDP_HEX_Rx + 17, Rx_PhoneBOOK.NumLen );
					Rx_PhoneBOOK.UserLen = UDP_HEX_Rx[17 + Rx_PhoneBOOK.NumLen];
					memset( Rx_PhoneBOOK.UserStr, 0, sizeof( Rx_PhoneBOOK.UserStr ) );
					Rx_PhoneBOOK.Effective_Flag = 1;            // 有效标志位
					memcpy( Rx_PhoneBOOK.UserStr, UDP_HEX_Rx + 18 + Rx_PhoneBOOK.NumLen, Rx_PhoneBOOK.UserLen );
					Api_RecordNum_Read( phonebook, UDP_HEX_Rx[14], (u8*)&Rx_PhoneBOOK, sizeof( Rx_PhoneBOOK ) );
					rt_kprintf( "\r\n Name:%s\r\n", Rx_PhoneBOOK.UserStr );
					rt_kprintf( "\r\n Number:%s\r\n", Rx_PhoneBOOK.NumberStr );
					break;
				default:
					break;
			}

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}

			break;
		case  0x8500:                                       //  车辆控制
			Vech_Control.Control_Flag = UDP_HEX_Rx[13];
			if( UDP_HEX_Rx[13] & 0x01 )
			{                                               // 车门加锁       bit 12
				Car_Status[2] |= 0x10;                      // 需要控制继电器
				rt_kprintf( "\r\n  车辆加锁 \r\n" );
			}else
			{                                               // 车门解锁
				Car_Status[2] &= ~0x10;                     // 需要控制继电器
				rt_kprintf( "\r\n  车辆解锁 \r\n" );
			}
			Vech_Control.ACK_SD_Flag = 1;
			break;
		case  0x8600:                                       //  设置圆形区域
			rt_kprintf( "\r\n  设置圆形区域 \r\n" );
			if( UDP_HEX_Rx[14] == 1 )                       //  现在支持设置一个区域
			{
				switch( UDP_HEX_Rx[13] )
				{
					case 1:                                 // 追加区域
						for( i = 0; i < 8; i++ )
						{
							memset( (u8*)&Rail_Cycle, 0, sizeof( Rail_Cycle ) );
							Api_RecordNum_Write( Rail_cycle, Rail_Cycle.Area_ID, (u8*)&Rail_Cycle, sizeof( Rail_Cycle ) );
                            Rails_Routline_Read(); 
							if( Rail_Cycle.Area_attribute ) // 找出来未使用的
							{
								break;
							}
						}
						if( 8 == i )                        //  如果都满了，那么用 0
						{
							i = 0;
						}

					case 0:                                 // 更新区域
					case 2:                                 // 修改区域
						memset( (u8*)&Rail_Cycle, 0, sizeof( Rail_Cycle ) );
						Rail_Cycle.Area_ID			= ( UDP_HEX_Rx[15] << 24 ) + ( UDP_HEX_Rx[16] << 16 ) + ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];
						Rail_Cycle.Area_attribute	= ( UDP_HEX_Rx[19] << 8 ) + UDP_HEX_Rx[20];
						Rail_Cycle.Center_Latitude	= ( UDP_HEX_Rx[21] << 24 ) + ( UDP_HEX_Rx[22] << 16 ) + ( UDP_HEX_Rx[23] << 8 ) + UDP_HEX_Rx[24];
						Rail_Cycle.Center_Longitude = ( UDP_HEX_Rx[25] << 24 ) + ( UDP_HEX_Rx[26] << 16 ) + ( UDP_HEX_Rx[27] << 8 ) + UDP_HEX_Rx[28];
						Rail_Cycle.Radius			= ( UDP_HEX_Rx[29] << 24 ) + ( UDP_HEX_Rx[30] << 16 ) + ( UDP_HEX_Rx[31] << 8 ) + UDP_HEX_Rx[32];
						memcpy( Rail_Cycle.StartTimeBCD, UDP_HEX_Rx + 33, 6 );
						memcpy( Rail_Cycle.EndTimeBCD, UDP_HEX_Rx + 39, 6 );
						Rail_Cycle.MaxSpd			= ( UDP_HEX_Rx[45] << 8 ) + UDP_HEX_Rx[46];
						Rail_Cycle.KeepDur			= UDP_HEX_Rx[47];
						Rail_Cycle.Effective_flag	= 1;

						if( ( Rail_Cycle.Area_ID > 8 ) || ( Rail_Cycle.Area_ID == 0 ) )
						{
							Rail_Cycle.Area_ID = 1;
						}
						Api_RecordNum_Write( Rail_cycle, Rail_Cycle.Area_ID, (u8*)&Rail_Cycle, sizeof( Rail_Cycle ) );
                        Rails_Routline_Read(); 
						break;
					default:
						break;
				}
			}
			//------- 返回 ----
			//   if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8601:                                                   //  删除圆形区域
			rt_kprintf( "\r\n  删除圆形区域 \r\n" );
			if( 0 == UDP_HEX_Rx[13] )                                   // 区域数
			{
				RailCycle_Init( );                                      // 删除所有区域
			}else
			{
				memset( (u8*)&Rail_Cycle, 0, sizeof( Rail_Cycle ) );    //  clear all  first
				for( i = 0; i < UDP_HEX_Rx[13]; i++ )
				{
					Rail_Cycle.Area_ID = ( UDP_HEX_Rx[14 + i] << 24 ) + ( UDP_HEX_Rx[15 + i] << 16 ) + ( UDP_HEX_Rx[16 + i] << 8 ) + UDP_HEX_Rx[17 + i];
					if( ( Rail_Cycle.Area_ID > 8 ) || ( Rail_Cycle.Area_ID == 0 ) )
					{
						Rail_Cycle.Area_ID = 1;
					}
					Rail_Cycle.Effective_flag = 0;                                                                  // clear
					Api_RecordNum_Write( Rail_cycle, Rail_Cycle.Area_ID, (u8*)&Rail_Cycle, sizeof( Rail_Cycle ) );  // 删除对应的围栏
                    Rails_Routline_Read(); 
				}
			}

			//----------------
			//   if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8602:                   //  设置矩形区域
			rt_kprintf( "\r\n  设置矩形区域 \r\n" );
			if( UDP_HEX_Rx[14] == 1 )   //  现在支持设置一个区域
			{
				switch( UDP_HEX_Rx[13] )
				{
					case 1:             // 追加区域
						rt_kprintf( "\r\n  追加围栏 " );
					case 0:             // 更新区域
					case 2:             // 修改区域
						memset( (u8*)&Rail_Rectangle, 0, sizeof( Rail_Rectangle ) );
						Rail_Rectangle.Area_ID				= ( UDP_HEX_Rx[15] << 24 ) + ( UDP_HEX_Rx[16] << 16 ) + ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];
						Rail_Rectangle.Area_attribute		= ( UDP_HEX_Rx[19] << 8 ) + UDP_HEX_Rx[20];
						Rail_Rectangle.LeftUp_Latitude		= ( UDP_HEX_Rx[21] << 24 ) + ( UDP_HEX_Rx[22] << 16 ) + ( UDP_HEX_Rx[23] << 8 ) + UDP_HEX_Rx[24];
						Rail_Rectangle.LeftUp_Longitude		= ( UDP_HEX_Rx[25] << 24 ) + ( UDP_HEX_Rx[26] << 16 ) + ( UDP_HEX_Rx[27] << 8 ) + UDP_HEX_Rx[28];
						Rail_Rectangle.RightDown_Latitude	= ( UDP_HEX_Rx[29] << 24 ) + ( UDP_HEX_Rx[30] << 16 ) + ( UDP_HEX_Rx[31] << 8 ) + UDP_HEX_Rx[32];
						Rail_Rectangle.RightDown_Longitude	= ( UDP_HEX_Rx[33] << 24 ) + ( UDP_HEX_Rx[34] << 16 ) + ( UDP_HEX_Rx[35] << 8 ) + UDP_HEX_Rx[36];
						memcpy( Rail_Rectangle.StartTimeBCD, UDP_HEX_Rx + 37, 6 );
						memcpy( Rail_Rectangle.EndTimeBCD, UDP_HEX_Rx + 43, 6 );
						Rail_Rectangle.MaxSpd			= ( UDP_HEX_Rx[49] << 8 ) + UDP_HEX_Rx[50];
						Rail_Rectangle.KeepDur			= UDP_HEX_Rx[51];
						Rail_Rectangle.Effective_flag	= 1;

						if( ( Rail_Rectangle.Area_ID > 8 ) || ( Rail_Rectangle.Area_ID == 0 ) )
						{
							Rail_Rectangle.Area_ID = 1;
						}
						Api_RecordNum_Write( Rail_rect, Rail_Rectangle.Area_ID, (u8*)&Rail_Rectangle, sizeof( Rail_Rectangle ) );
                        Rails_Routline_Read(); 
						rt_kprintf( "\r\n   中心设置  矩形围栏 leftLati=%d leftlongi=%d rightLati=%d rightLong=%d \r\n", Rail_Rectangle.LeftUp_Latitude, Rail_Rectangle.LeftUp_Longitude, Rail_Rectangle.RightDown_Latitude, Rail_Rectangle.RightDown_Longitude );

						if( Rail_Rectangle.Area_attribute & 0x4000 )
						{
							rt_kprintf( "\r\n   围栏 attribute=0x%4X  id=%d\r\n", Rail_Rectangle.Area_attribute, Rail_Rectangle.Area_ID );
						}

						break;
					default:
						break;
				}
			}
			//----------------
			//if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8603:                                                           //  删除矩形区域
			rt_kprintf( "\r\n  删除矩形区域 \r\n" );
			if( 0 == UDP_HEX_Rx[13] )                                           // 区域数
			{
				RailRect_Init( );                                               // 删除所有区域
                Rails_Routline_Read(); 
			}else
			{
				memset( (u8*)&Rail_Rectangle, 0, sizeof( Rail_Rectangle ) );    //  clear all  first
				for( i = 0; i < UDP_HEX_Rx[13]; i++ )
				{
					Rail_Rectangle.Area_ID = ( UDP_HEX_Rx[14 + i] << 24 ) + ( UDP_HEX_Rx[15 + i] << 16 ) + ( UDP_HEX_Rx[16 + i] << 8 ) + UDP_HEX_Rx[17 + i];
					if( ( Rail_Rectangle.Area_ID > 8 ) || ( Rail_Rectangle.Area_ID == 0 ) )
					{
						Rail_Rectangle.Area_ID = 1;
					}
					Rail_Rectangle.Effective_flag = 0;
					Api_RecordNum_Write( Rail_rect, Rail_Rectangle.Area_ID, (u8*)&Rail_Rectangle, sizeof( Rail_Rectangle ) ); // 删除对应的围栏
                    Rails_Routline_Read(); 
				}
			}

			//----------------
			//  if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8604:                   //  多边形区域
			rt_kprintf( "\r\n  设置多边形区域 \r\n" );
			if( UDP_HEX_Rx[14] == 1 )   //  现在支持设置一个区域
			{
				switch( UDP_HEX_Rx[13] )
				{
					case 1:             // 追加区域
					case 0:             // 更新区域
					case 2:             // 修改区域
						memset( (u8*)&Rail_Polygen, 0, sizeof( Rail_Polygen ) );
						Rail_Polygen.Area_ID		= ( UDP_HEX_Rx[15] << 24 ) + ( UDP_HEX_Rx[16] << 16 ) + ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];
						Rail_Polygen.Area_attribute = ( UDP_HEX_Rx[19] << 8 ) + UDP_HEX_Rx[20];
						memcpy( Rail_Polygen.StartTimeBCD, UDP_HEX_Rx + 20, 6 );
						memcpy( Rail_Polygen.EndTimeBCD, UDP_HEX_Rx + 26, 6 );
						Rail_Polygen.MaxSpd				= ( UDP_HEX_Rx[32] << 8 ) + UDP_HEX_Rx[33];
						Rail_Polygen.KeepDur			= UDP_HEX_Rx[34];
						Rail_Polygen.Acme_Num			= UDP_HEX_Rx[35];
						Rail_Polygen.Acme1_Latitude		= ( UDP_HEX_Rx[36] << 24 ) + ( UDP_HEX_Rx[37] << 16 ) + ( UDP_HEX_Rx[38] << 8 ) + UDP_HEX_Rx[39];
						Rail_Polygen.Acme1_Longitude	= ( UDP_HEX_Rx[40] << 24 ) + ( UDP_HEX_Rx[41] << 16 ) + ( UDP_HEX_Rx[42] << 8 ) + UDP_HEX_Rx[43];
						Rail_Polygen.Acme2_Latitude		= ( UDP_HEX_Rx[44] << 24 ) + ( UDP_HEX_Rx[45] << 16 ) + ( UDP_HEX_Rx[46] << 8 ) + UDP_HEX_Rx[47];
						Rail_Polygen.Acme2_Longitude	= ( UDP_HEX_Rx[48] << 24 ) + ( UDP_HEX_Rx[49] << 16 ) + ( UDP_HEX_Rx[50] << 8 ) + UDP_HEX_Rx[51];
						Rail_Polygen.Acme3_Latitude		= ( UDP_HEX_Rx[52] << 24 ) + ( UDP_HEX_Rx[53] << 16 ) + ( UDP_HEX_Rx[54] << 8 ) + UDP_HEX_Rx[55];
						Rail_Polygen.Acme3_Longitude	= ( UDP_HEX_Rx[56] << 24 ) + ( UDP_HEX_Rx[57] << 16 ) + ( UDP_HEX_Rx[58] << 8 ) + UDP_HEX_Rx[59];

						if( ( Rail_Polygen.Area_ID > 8 ) || ( Rail_Polygen.Area_ID == 0 ) )
						{
							Rail_Polygen.Area_ID = 1;
						}
						Rail_Polygen.Effective_flag = 1;
						Api_RecordNum_Write( Rail_polygen, Rail_Polygen.Area_ID, (u8*)&Rail_Polygen, sizeof( Rail_Polygen ) );
						break;
					default:
						break;
				}
			}

			//----------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8605:                                                       //  删除多边区域
			rt_kprintf( "\r\n  删除多边形区域 \r\n" );
			if( 0 == UDP_HEX_Rx[13] )                                       // 区域数
			{
				RailPolygen_Init( );                                        // 删除所有区域
			}else
			{
				memset( (u8*)&Rail_Polygen, 0, sizeof( Rail_Polygen ) );    //  clear all  first
				for( i = 0; i < UDP_HEX_Rx[13]; i++ )
				{
					Rail_Polygen.Area_ID = ( UDP_HEX_Rx[14 + i] << 24 ) + ( UDP_HEX_Rx[15 + i] << 16 ) + ( UDP_HEX_Rx[16 + i] << 8 ) + UDP_HEX_Rx[17 + i];
					if( ( Rail_Polygen.Area_ID > 8 ) || ( Rail_Polygen.Area_ID == 0 ) )
					{
						Rail_Polygen.Area_ID = 1;
					}
					Rail_Polygen.Effective_flag = 0;
					Api_RecordNum_Write( Rail_polygen, Rail_Polygen.Area_ID, (u8*)&Rail_Polygen, sizeof( Rail_Polygen ) );
				}
			}

			//----------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8606:                                           //  设置路线
			rt_kprintf( "\r\n  设置路线 \r\n" );
			memset( (u8*)&ROUTE_Obj, 0, sizeof( ROUTE_Obj ) );  //  clear all  first
			ROUTE_Obj.Route_ID			= ( UDP_HEX_Rx[13] << 24 ) + ( UDP_HEX_Rx[14] << 16 ) + ( UDP_HEX_Rx[15] << 8 ) + UDP_HEX_Rx[16];
			ROUTE_Obj.Route_attribute	= ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];
			memcpy( ROUTE_Obj.StartTimeBCD, UDP_HEX_Rx + 19, 6 );
			memcpy( ROUTE_Obj.EndTimeBCD, UDP_HEX_Rx + 25, 6 );
			ROUTE_Obj.Points_Num = ( UDP_HEX_Rx[31] << 8 ) + UDP_HEX_Rx[32];
			rt_kprintf( "\r\n ROUTE_Obj.ID:  %d  \r\n ", ROUTE_Obj.Route_ID );
			rt_kprintf( "\r\n ROUTE_Obj.attribute:  %04X  \r\n ", ROUTE_Obj.Route_attribute );
			rt_kprintf( "\r\n ROUTE_Obj.Points_Num:  %d  \r\n ", ROUTE_Obj.Points_Num );
			//if(ROUTE_Obj.Points_Num<3)       // cheat  mask
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;

				line_warn_enable = 1;
				rt_kprintf( "\r\n  line_enable start \r\n" );

				break;
			}
			reg_u32 = 33;
			for( i = 0; i < 6; i++ ) // 拐点数目
			{
				// if((infolen+32)<reg_u32)
				//	 break;

				ROUTE_Obj.RoutePoints[i].POINT_ID	= ( UDP_HEX_Rx[reg_u32] << 24 ) + ( UDP_HEX_Rx[reg_u32 + 1] << 16 ) + ( UDP_HEX_Rx[reg_u32 + 2] << 8 ) + UDP_HEX_Rx[reg_u32 + 3];
				reg_u32								+= 4;
				rt_kprintf( "\r\n PointID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_ID );
				ROUTE_Obj.RoutePoints[i].Line_ID	= ( UDP_HEX_Rx[reg_u32] << 24 ) + ( UDP_HEX_Rx[reg_u32 + 1] << 16 ) + ( UDP_HEX_Rx[reg_u32 + 2] << 8 ) + UDP_HEX_Rx[reg_u32 + 3];
				reg_u32								+= 4;
				rt_kprintf( "\r\n LineID=%08x\r\n", ROUTE_Obj.RoutePoints[i].Line_ID );
				ROUTE_Obj.RoutePoints[i].POINT_Latitude = ( UDP_HEX_Rx[reg_u32] << 24 ) + ( UDP_HEX_Rx[reg_u32 + 1] << 16 ) + ( UDP_HEX_Rx[reg_u32 + 2] << 8 ) + UDP_HEX_Rx[reg_u32 + 3];
				reg_u32									+= 4;
				rt_kprintf( "\r\n LatiID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_Latitude );
				ROUTE_Obj.RoutePoints[i].POINT_Longitude	= ( UDP_HEX_Rx[reg_u32] << 24 ) + ( UDP_HEX_Rx[reg_u32 + 1] << 16 ) + ( UDP_HEX_Rx[reg_u32 + 2] << 8 ) + UDP_HEX_Rx[reg_u32 + 3];
				reg_u32										+= 4;
				rt_kprintf( "\r\n LongID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_Longitude );
				ROUTE_Obj.RoutePoints[i].Width = UDP_HEX_Rx[reg_u32++];
				rt_kprintf( "\r\n Width=%02x\r\n", ROUTE_Obj.RoutePoints[i].Width );
				ROUTE_Obj.RoutePoints[i].Atribute = UDP_HEX_Rx[reg_u32++];
				rt_kprintf( "\r\n atrit=%02x\r\n\r\n", ROUTE_Obj.RoutePoints[i].Atribute );
				if( ROUTE_Obj.RoutePoints[i].Atribute == 0 )
				{
					;
				} else
				if( ROUTE_Obj.RoutePoints[i].Atribute == 1 )
				{
					ROUTE_Obj.RoutePoints[i].MaxSpd = ( UDP_HEX_Rx[reg_u32++] << 8 ) + UDP_HEX_Rx[reg_u32++];
				} else
				{
					ROUTE_Obj.RoutePoints[i].TooLongValue	= ( UDP_HEX_Rx[reg_u32++] << 8 ) + UDP_HEX_Rx[reg_u32++];
					ROUTE_Obj.RoutePoints[i].TooLessValue	= ( UDP_HEX_Rx[reg_u32++] << 8 ) + UDP_HEX_Rx[reg_u32++];
					ROUTE_Obj.RoutePoints[i].MaxSpd			= ( UDP_HEX_Rx[reg_u32++] << 8 ) + UDP_HEX_Rx[reg_u32++];
					ROUTE_Obj.RoutePoints[i].KeepDur		= ( UDP_HEX_Rx[reg_u32++] << 8 ) + UDP_HEX_Rx[reg_u32++];
				}
			}

			if( ( ROUTE_Obj.Route_ID > Route_Mum ) || ( ROUTE_Obj.Route_ID == 0 ) )
			{
				ROUTE_Obj.Route_ID = 1;
			}
			ROUTE_Obj.Effective_flag = 1;
			Api_RecordNum_Write( route_line, ROUTE_Obj.Route_ID, (u8*)&ROUTE_Obj, sizeof( ROUTE_Obj ) ); // 删除对应的围栏

			//----------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8607:                                               //  删除路线
			rt_kprintf( "\r\n  删除路线 \r\n" );
			if( 0 == UDP_HEX_Rx[13] )                               // 区域数
			{
				RouteLine_Init( );                                  // 删除所有区域
			}else
			{
				memset( (u8*)&ROUTE_Obj, 0, sizeof( ROUTE_Obj ) );  //  clear all  first
				for( i = 0; i < UDP_HEX_Rx[13]; i++ )
				{
					ROUTE_Obj.Route_ID = ( UDP_HEX_Rx[14 + i] << 24 ) + ( UDP_HEX_Rx[15 + i] << 16 ) + ( UDP_HEX_Rx[16 + i] << 8 ) + UDP_HEX_Rx[17 + i];
					if( ( ROUTE_Obj.Route_ID > Route_Mum ) || ( ROUTE_Obj.Route_ID == 0 ) )
					{
						ROUTE_Obj.Route_ID = 1;
					}
					ROUTE_Obj.Effective_flag = 0;
					Api_RecordNum_Write( route_line, ROUTE_Obj.Route_ID, (u8*)&ROUTE_Obj, sizeof( ROUTE_Obj ) ); // 删除对应的围栏
				}
			}

			//----------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8700: //  行车记录仪数据采集命令


			/*
			                           行车记录仪发送出发开始
			 */
			rt_kprintf( "\r\n  记录仪采集命令 \r\n" );
			Recode_Obj.Float_ID = Centre_FloatID;
			//Recode_Obj.CMD=UDP_HEX_Rx[13];
			//    Stuff  Hardly
			switch( UDP_HEX_Rx[13] )
			{
				case 0x00: Recode_Obj.CMD	= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					break;
				//--------- Lagre  block -------


				/*
				      cmd         maxnum
				        15H	  2
				        12H	  10
				   11H	  10
				   10H	  100
				   09H	  360
				   08H	  576
				 */
				case  0x08:  Recode_Obj.Total_pkt_num	= 720;
					Recode_Obj.Current_pkt_num			= 1;
					Recode_Obj.Bak_current_num=1;
					Recode_Obj.Transmit_running=1;  // enable transmit
					Recode_Obj.Devide_Flag				= 1;
					//--------------------------------
					Recode_Obj.CMD			= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					MediaObj.Media_Type		= 3; //行驶记录仪 只是利用类型填充ID 时候有用
					break;

				case  0x09:  Recode_Obj.Total_pkt_num	= 720;
					Recode_Obj.Current_pkt_num			= 0; //0
					Recode_Obj.Devide_Flag				= 1;
					Recode_Obj.Bak_current_num=0;
					Recode_Obj.Transmit_running=1;  // enable transmit
					//--------------------------------
					Recode_Obj.CMD			= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					MediaObj.Media_Type		= 3; //行驶记录仪
					break;
				case  0x10:
					Recode_Obj.Total_pkt_num	= 100;
					Recode_Obj.Current_pkt_num	= 1;
					Recode_Obj.Devide_Flag		= 1;
					Recode_Obj.Bak_current_num=1;
					Recode_Obj.Transmit_running=1;  // enable transmit
					//--------------------------------
					Recode_Obj.CMD			= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					Recode_Obj.fcs				= 0;
					MediaObj.Media_Type		= 3; //行驶记录仪
					break;
				case  0x11:
					Recode_Obj.Total_pkt_num	= 100;
					Recode_Obj.Current_pkt_num	= 1;
					Recode_Obj.Devide_Flag		= 1;
					Recode_Obj.Bak_current_num=1;
					Recode_Obj.Transmit_running=1;  // enable transmit
					//------------------------------------
					Recode_Obj.CMD				= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag		= 1;
					Recode_Obj.CountStep		= 1;
					Recode_Obj.fcs				= 0;
					MediaObj.Media_Type			= 3; //行驶记录仪
					break;
				case  0x12:
					Recode_Obj.Total_pkt_num	= 10;
					Recode_Obj.Current_pkt_num	= 1;
					Recode_Obj.Devide_Flag		= 1;
					Recode_Obj.Bak_current_num=1;
					Recode_Obj.Transmit_running=1;	// enable transmit

					//------------------------------------
					Recode_Obj.CMD				= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag		= 1;
					Recode_Obj.CountStep		= 1;
					Recode_Obj.fcs				= 0;
					MediaObj.Media_Type			= 3; //行驶记录仪
					break;
				case  0x13:
					//--------------------------------
					Recode_Obj.CMD			= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					break;
				case  0x14:
					//--------------------------------
					Recode_Obj.CMD			= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag = 1;
					Recode_Obj.CountStep	= 1;
					break;
				case  0x15:
					Recode_Obj.Current_pkt_num	= 1;
					Recode_Obj.Devide_Flag		= 1;
					Recode_Obj.Bak_current_num=1;
					Recode_Obj.Transmit_running=1;  // enable transmit
					Recode_Obj.CMD				= UDP_HEX_Rx[13];
					Recode_Obj.SD_Data_Flag		= 1;
					Recode_Obj.CountStep		= 1;
					Recode_Obj.Total_pkt_num	= 10;
					Recode_Obj.fcs				= 0;
					MediaObj.Media_Type			= 3;                        //行驶记录仪
					break;
				default:
					break;
			}

			break;
		case  0x8701:                                                       //  行驶记录仪参数下传命令
			rt_kprintf( "\r\n  记录仪参数下传 \r\n" );
			Recode_Obj.Float_ID		= Centre_FloatID;
			Recode_Obj.CMD			= UDP_HEX_Rx[13];
			Recode_Obj.SD_Data_Flag = 1;
			CenterSet_subService_8701H( Recode_Obj.CMD, UDP_HEX_Rx + 14 );  //跳过2B长度和1 保留字
			break;
		case  0x8800:                                                       //   多媒体数据上传应答
			if( infolen == 5 )
			{                                                               //  判断是否有重传ID列表，如果没有则表示中心接收完成!
				switch( MediaObj.Media_Type )
				{
					case 0:                                                 // 图像
						Photo_send_end( );                                  // 拍照上传结束
						rt_kprintf( "\r\n 图像传输结束! \r\n" );
						//------------多路拍照处理  -------
						// CheckResualt=Check_MultiTakeResult_b4Trans();

						break;
					case 1:                                                 // 音频
						Sound_send_end( );
						rt_kprintf( "\r\n 音频传输结束! \r\n" );
						break;
					case 2:                                                 // 视频
						Video_send_end( );
						rt_kprintf( "\r\n 视频传输结束! \r\n" );
						break;
					default:
						break;
				}
				// if(CheckResualt==0)
				Media_Clear_State( );
			}
			else
			{                                               //  重传包ID 列表
				if( UDP_HEX_Rx[17] != 0 )
				{
					MediaObj.RSD_State	= 1;                //   进入重传状态
					MediaObj.RSD_Timer	= 0;                //   清除重传定时器
					MediaObj.RSD_Reader = 0;
					MediaObj.RSD_total	= UDP_HEX_Rx[17];   // 重传包总数
					//   获取重传列表
					j = 0;
					for( i = 0; i < MediaObj.RSD_total; i++ )
					{
						MediaObj.Media_ReSdList[i]	= ( UDP_HEX_Rx[18 + j] << 8 ) + UDP_HEX_Rx[19 + j];
						j							+= 2;
					}
					rt_kprintf( "\r\n  8003 重传列表Total=%d: ", MediaObj.RSD_total );
					for( i = 0; i < MediaObj.RSD_total; i++ )
					{
						rt_kprintf( "%d,", MediaObj.Media_ReSdList[i] );
					}
					rt_kprintf( "\r\n" );
				}
			}

			break;
		case  0x8801:                                                               //    摄像头立即拍照

			Camera_Obj.Channel_ID		= UDP_HEX_Rx[13];                           //   通道
			Camera_Obj.Operate_state	= UDP_HEX_Rx[18];                           //   是否保存标志位
			//----------------------------------

			if( ( Camera_Take_Enable( ) ) && ( Photo_sdState.photo_sending == 0 ) ) //图片传输中不能拍
			{
				Camera_Number = UDP_HEX_Rx[13];
				if( ( Camera_Number > Max_CameraNum ) && ( Camera_Number < 1 ) )
				{
					break;
				}

				Start_Camera( Camera_Number );  //开始拍照
				SingleCamera_TakeRetry = 0;     // clear
			}
			rt_kprintf( "\r\n   中心及时拍照  Camera: %d    \r\n", Camera_Number );

			if( UDP_HEX_Rx[18] == 0x01 )        // 拍照不上传
			{
				Camera_Take_not_trans = 1;
				rt_kprintf( "\r\n   拍照不上传\r\n" );
			}else
			{
				Camera_Take_not_trans = 0;
			}

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)   //mask below  use   0805H
			//		 {
			//		 SD_ACKflag.f_CentreCMDack_0001H=1;
			//	 Ack_Resualt=0;
			// SD_ACKflag.f_CentreCMDack_resualt=Ack_Resualt;
			//	 }
			break;
		case  0x8802:   //    存储多媒体数据检索
			SD_ACKflag.f_QueryEventCode = UDP_HEX_Rx[15];
			switch( UDP_HEX_Rx[13] )
			{
				case 0: // 图像
					SD_ACKflag.f_MediaIndexACK_0802H = 1;
					rt_kprintf( "\r\n  中心查询图像索引 \r\n" );
					break;
				case 1: //  音频
					SD_ACKflag.f_MediaIndexACK_0802H = 2;
					rt_kprintf( "\r\n  中心查询音频索引 \r\n" );
				case 2: //  视频
					SD_ACKflag.f_MediaIndexACK_0802H = 3;
				default:
					break;
			}

			break;
		case  0x8803:   //    存储多媒体数据上传命令
			rt_kprintf( "\r\n 多媒体数据上传\r\n" );
			switch( UDP_HEX_Rx[13] )
			{
				case 0: // 图像
					rt_kprintf( "\r\n   上传固有图片\r\n" );
					break;
				case 1: //  音频
					MP3_send_start( );
					rt_kprintf( "\r\n  上传固有音频 \r\n" );
					break;
				case 2: //  视频
					    // Video_send_start();
					    // MP3_send_start();
					rt_kprintf( "\r\n  上传固有视频  不操作了 用音频\r\n" );
					break;
				default:
					break;
			}

			//----------------------------------------------------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8804:       //    录音开始命令

			//#if  1
			switch( UDP_HEX_Rx[13] )
			{
				case 0x00:  // 停录音
					// VOC_REC_STOP(void);
					//   VOC_REC_Stop();


					/*
					                               Dev_Voice.CMD_Type='0';
					    Dev_Voice.info_sdFlag=0;
					    Dev_Voice.Voice_FileOperateFlag=0;
					    Dev_Voice.Centre_RecordFlag=0; // 清空中心录音标志位
					    if(TF_Card_Status()==1)
					     {
					       //memset(Dev_Voice.Voice_Reg,0,512);//clear left
					       //edit_file(Dev_Voice.FileName,Dev_Voice.Voice_Reg,512); //写信息到TF
					       //Dev_Voice.Voice_FileSize+=500; //add
					       rt_kprintf("\r\n ---------   语音文件的最后地址	  VoiceFileSize %d Bytes  \r\n",Dev_Voice.Voice_FileSize);
					     }
					   Api_DFdirectory_Write(voice, Dev_Voice.Voice_Reg,500);
					                            Dev_Voice.Voice_FileSize+=500;
					                            Sound_SaveEnd();  */
					break;
				case 0x01:  // 开始录音

					VOC_REC_Start(1); 


					/*
					          if(MMedia2_Flag==0)
					           {
					                                 MP3_send_start();
					      rt_kprintf("\r\n  上传固有音频 \r\n");
					      MMedia2_Flag=1;
					      break;
					           }
					                              Dev_Voice.Rec_Dur=(UDP_HEX_Rx[14]<<8)+UDP_HEX_Rx[15];
					   Dev_Voice.SaveOrNotFlag=UDP_HEX_Rx[16];

					                              // ------------   录音文件名 -----------
					                              if(TF_Card_Status()==1)
					                           {
					                               memset(Dev_Voice.FileName,0,sizeof(Dev_Voice.FileName));
					    sprintf((char*)Dev_Voice.FileName,"%d%d%d%d.spx",time_now.day,time_now.hour,time_now.min,time_now.sec);
					                              // creat_file(Dev_Voice.FileName); //创建文件名了
					                               Sound_SaveStart();
					    rt_kprintf("\r\n			  中心录音文件名称: %s \r\n",Dev_Voice.FileName);
					    Save_MediaIndex(1,Dev_Voice.FileName,0,0);
					                               }
					                              Dev_Voice.Centre_RecordFlag=1;//中心开始录音标志位

					   Dev_Voice.CMD_Type='1';
					   Dev_Voice.info_sdFlag=2;
					   Dev_Voice.Voice_FileSize=0; //clear size
					   Dev_Voice.Voice_FileOperateFlag=1;
					   MMedia2_Flag=0;
					 */
					break;
			}

			//------------------------------
			//if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8900:                   //    数据下行透传
			switch( UDP_HEX_Rx[13] )    // 透传消息类型  // BD
			{
				case  0x41:             // 串口1  透传消息

					break;
				case  0x42:             //  串口2 透传消息

					break;
				case  0x0B:             //  IC  卡 信息
					memcpy( IC_MOD.IC_Rx, UDP_HEX_Rx + 14, infolen - 1 );
					rt_kprintf( "\r\n IC 卡透传len=%dBytes  RX:", infolen - 1 );
					for( i = 0; i < infolen - 1; i++ )
					{
						rt_kprintf( "%2X ", UDP_HEX_Rx[14 + i] );
					}
					rt_kprintf( "\r\n" );
					//------ 直接发送给IC 卡模块-----
					Reg_buf[0] = 0x00;
					memcpy( Reg_buf + 1, IC_MOD.IC_Rx, infolen - 1 );
					DeviceData_Encode_Send( 0x0B, 0x40, Reg_buf, infolen );
					return;
			}
			//---------------------------------------------------
			if( LinkNum == 0 )
			{
				DataTrans.TYPE = UDP_HEX_Rx[13];
				memset( DataTrans.DataRx, 0, sizeof( DataTrans.DataRx ) );
				memcpy( DataTrans.DataRx, UDP_HEX_Rx + 14, infolen - 1 );
				DataTrans.Data_RxLen = infolen - 1;

				//--------- 送给小屏幕----------
				memset( TextInfo.TEXT_Content, 0, sizeof( TextInfo.TEXT_Content ) );
				AsciiToGb( TextInfo.TEXT_Content, infolen - 1, UDP_HEX_Rx + 14 );
				TextInfo.TEXT_SD_FLAG = 1; // 置发送给显示屏标志位  // ||||||||||||||||||||||||||||||||||

				//========================================

#ifdef LCD_5inch

				DwinLCD.Type = LCD_SDTXT;
				memset( DwinLCD.TXT_content, 0, sizeof( DwinLCD.TXT_content ) );
				DwinLCD.TXT_contentLen = AsciiToGb( DwinLCD.TXT_content, infolen - 1, UDP_HEX_Rx + 14 );

#endif
				// if(SD_ACKflag.f_CentreCMDack_0001H==0)
				{
					SD_ACKflag.f_CentreCMDack_0001H		= 1;
					Ack_Resualt							= 0;
					SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
				}
			}
			break;
		case  0x8A00: //    平台RSA公钥

			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
		{
			SD_ACKflag.f_CentreCMDack_0001H		= 1;
			Ack_Resualt							= 0;
			SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
		}
		break;
		case  0x8106:                                                                   //BD--8.11 查询指定终端参数
			Setting_Qry.Num_pram	= UDP_HEX_Rx[13];                                   // 个数
			reg_u32					= 0;                                                // 借用做下标计数器
			for( i = 0; i < ( infolen - 1 ) / 2; i++ )
			{
				Detach_PKG.List_Resend[reg_u32] = ( UDP_HEX_Rx[16 + 2 * reg_u32] << 8 ) + UDP_HEX_Rx[17 + 2 * reg_u32 + 1];
				reg_u32++;
			}

			SD_ACKflag.f_SettingPram_0104H = 2;                                         // 中心查询指定参数
			break;
		case  0x8107:                                                                   // BD--8.14 查询终端属性
			SD_ACKflag.f_BD_DeviceAttribute_8107 = 1;                                   // 消息体为空
			break;
		case   0x8108:                                                                  //BD--8.16 下发终端升级包   远程升级  (重要)

			//   1.  分包判断bit 位
			if( UDP_HEX_Rx[3] & 0x20 )
			{                                                                           //  分包判断
				BD_ISP.Total_PacketNum		= ( UDP_HEX_Rx[13] << 8 ) + UDP_HEX_Rx[14]; // 总包数
				BD_ISP.CurrentPacket_Num	= ( UDP_HEX_Rx[15] << 8 ) + UDP_HEX_Rx[16]; // 当前包序号从1 开始
			}else
			{
				//------------   ACK   Flag -----------------
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			//   2.  检测
			if( BD_ISP.CurrentPacket_Num == 1 )
			{
				BD_ISP.Update_Type = UDP_HEX_Rx[17]; //升级包类型
				//----  Debug ------
				switch( BD_ISP.Update_Type )
				{
					case  0:
						rt_kprintf( "\r\n 升级类型: 终端\r\n" );
						break;
					case  12:
						rt_kprintf( "\r\n 升级类型: IC 卡读卡器\r\n" );
						break;
					case  52:
						rt_kprintf( "\r\n 升级类型: 北斗模块\r\n" );
						break;
				}
				//----------------------
				memcpy( BD_ISP.ProductID, UDP_HEX_Rx + 18, 5 );                                                                                 // 制造商ID
				BD_ISP.Version_len = UDP_HEX_Rx[23];                                                                                            // 版本号长度
				memcpy( BD_ISP.VersionStr, UDP_HEX_Rx + 20, BD_ISP.Version_len );                                                               // 软件版本号
				i					= 24 + BD_ISP.Version_len;
				BD_ISP.Content_len	= ( UDP_HEX_Rx[i] << 24 ) + ( UDP_HEX_Rx[i + 1] << 16 ) + ( UDP_HEX_Rx[i + 2] << 8 ) + UDP_HEX_Rx[i + 3];   // 升级包长度
				i					+= 4;
				rt_kprintf( "\r\n 升级包长度:%d Bytes\r\n", BD_ISP.Content_len );

				infolen = infolen - 11 - BD_ISP.Version_len;                                                                                    // infolen

				BD_ISP.PacketRX_wr	= 0;                                                                                                        // clear
				BD_ISP.ISP_running	= 1;
			}
			//--------  升级包内容  -------------
			if( infolen )                                                                                                                       //  升级内容以后根据类型往不同的存储区域写
			{
				memcpy( BD_ISP.ContentData, UDP_HEX_Rx + i + 4, infolen );                                                                      // 升级包内容
				BD_ISP.PacketRX_wr += infolen;
			}
			//------------   ACK   Flag -----------------
			SD_ACKflag.f_CentreCMDack_0001H		= 1;
			Ack_Resualt							= 0;
			SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			//------------------------------------------------------------------------
			break;
		case   0x8203:                                                                                  //BD--8.22  人工确认报警消息
			HumanConfirmWarn.Warn_FloatID		= ( UDP_HEX_Rx[13] << 8 ) + UDP_HEX_Rx[14];
			HumanConfirmWarn.ConfirmWarnType	= ( UDP_HEX_Rx[15] << 24 ) + ( UDP_HEX_Rx[16] << 16 ) + ( UDP_HEX_Rx[17] << 8 ) + UDP_HEX_Rx[18];
			if( HumanConfirmWarn.Warn_FloatID == 0x00 )
			{                                                                                           //  如果为0 表示所有消息
				Warn_Status[3]	&= ~0x01;                                                               // bit 0
				Warn_Status[3]	&= ~0x08;                                                               // bit3
				Warn_Status[1]	&= ~0x10;                                                               // bit20 进出区域报警
				Warn_Status[1]	&= ~0x20;                                                               // bit 21 进出线路报警
				Warn_Status[1]	&= ~0x40;                                                               // bit 22  路段行驶时间不足报警
				Warn_Status[0]	&= ~0x08;                                                               // bit 27  确认车辆非法点火报警
				Warn_Status[0]	&= ~0x10;                                                               // bit 28  确认车辆非法报警
				Warn_Status[0]	&= ~0x80;                                                               //bit 31  非法开门 (终端不设置区域时，车门开关无效)
			}else
			{
				//--------------------------------
				if( ( Warn_Status[3] & 0x01 ) && ( 0x00000001 & HumanConfirmWarn.ConfirmWarnType ) )    //紧急报警
				{
					StatusReg_WARN_Clear( );
					f_Exigent_warning	= 0;
					warn_flag			= 0;
					Send_warn_times		= 0;
					StatusReg_WARN_Clear( );
					rt_kprintf( "\r\n紧急报警收到应答，得清除!\r\n" );
				}
				if( ( Warn_Status[3] & 0x08 ) && ( 0x00000008 & HumanConfirmWarn.ConfirmWarnType ) )    //危险报警-BD
				{
					Warn_Status[3] &= ~0x08;
				}

				//------------------------------------
				if( ( Warn_Status[1] & 0x10 ) && ( 0x00001000 & HumanConfirmWarn.ConfirmWarnType ) )    // 进出区域报警
				{
					InOut_Object.TYPE		= 0;                                                        //圆形区域
					InOut_Object.ID			= 0;                                                        //  ID
					InOut_Object.InOutState = 0;                                                        //  进报警
					Warn_Status[1]			&= ~0x10;
				}
				if( ( Warn_Status[1] & 0x20 ) && ( 0x00002000 & HumanConfirmWarn.ConfirmWarnType ) )    // 进出路线报警
				{
					InOut_Object.TYPE		= 0;                                                        //圆形区域
					InOut_Object.ID			= 0;                                                        //  ID
					InOut_Object.InOutState = 0;                                                        //  进报警
					Warn_Status[1]			&= ~0x20;
				}
				if( ( Warn_Status[1] & 0x40 ) && ( 0x00004000 & HumanConfirmWarn.ConfirmWarnType ) )    // 路段行驶时间不足、过长
				{
					Warn_Status[1] &= ~0x40;
				}
				//-----------------------------------------
				if( ( Warn_Status[0] & 0x08 ) && ( 0x08000000 & HumanConfirmWarn.ConfirmWarnType ) )    //车辆非法点火
				{
					Warn_Status[0] &= ~0x08;
				}
				if( ( Warn_Status[0] & 0x10 ) && ( 0x10000000 & HumanConfirmWarn.ConfirmWarnType ) )    //车辆非法位移
				{
					Warn_Status[0] &= ~0x10;
				}
				if( ( Warn_Status[0] & 0x80 ) && ( 0x80000000 & HumanConfirmWarn.ConfirmWarnType ) )    //非法开门报警(终端未设置区域时，不判断非法开门)
				{
					Warn_Status[0] &= ~0x80;
				}
				//------------------------------------
			}

			//---------  报警确认后发送，立即发送定位包
			PositionSD_Enable( );
			Current_UDP_sd = 1;
			break;
		case   0x8702:                              // BD-8.47-  上报驾驶员身份信息请求
			SD_ACKflag.f_DriverInfoSD_0702H = 1;    // 消息体为空
			break;
		case  0x8805:                               //BD--8.55    单条存储多媒体数据检索上传命令	---- 补充协议要求

			reg_u32 = ( UDP_HEX_Rx[13] << 24 ) + ( UDP_HEX_Rx[14] << 16 ) + ( UDP_HEX_Rx[15] << 8 ) + UDP_HEX_Rx[16];
			rt_kprintf( "\r\n单条存储多媒体数据检索上传 MeidiaID=%d 删除标志: %d ", reg_u32, UDP_HEX_Rx[17] );

			Camera_Number = 1;
			Photo_send_start( Camera_Number );      //在不是多路拍照的情况下拍完就可以上传了
			//------------------------------
			// if(SD_ACKflag.f_CentreCMDack_0001H==0)
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			break;
		case  0x8003:                                //    补传分包请求

			    //Devide_8003_packet_1stID=( UDP_HEX_Rx[13] << 8 ) + UDP_HEX_Rx[14]; //高字节在前，在后 
			                                                                      // 这里通过高字节判断

  		       devide_value=0;
			   switch(UDP_HEX_Rx[13]) //根据中心下发的流水号判断
			   {
				   case 0x80:
				   	         Recode_Obj.CMD=0x08;
							 Recode_Obj.Total_pkt_num	= 720;
							Recode_Obj.Devide_Flag				= 1;
							MediaObj.Media_Type		= 3; //行驶记录仪 只是利用类型填充ID 时候有用
							  break;
				   case 0x90: 
				   	        Recode_Obj.CMD=0x09;
							Recode_Obj.Total_pkt_num	= 720;
							Recode_Obj.Devide_Flag				= 1;					
							MediaObj.Media_Type		= 3; //行驶记录仪
							  break;
				   case 0xA0:
				   	        Recode_Obj.CMD=0x10;
							Recode_Obj.Total_pkt_num	= 100;
							Recode_Obj.Devide_Flag				= 1;					
							MediaObj.Media_Type		= 3; //行驶记录仪
							  break;
				   case 0xB0:
				   	         Recode_Obj.CMD=0x11;
							 Recode_Obj.Total_pkt_num	= 100;
					        Recode_Obj.Devide_Flag				= 1;					
							MediaObj.Media_Type		= 3; //行驶记录仪
							  break;
				   case 0xC0:
				   	        Recode_Obj.CMD=0x12;
							Recode_Obj.Total_pkt_num	= 10;						
					        Recode_Obj.Devide_Flag				= 1;					
							MediaObj.Media_Type		= 3; //行驶记录仪
							  break;
							  break;
				   case  0xD0:
				   	        Recode_Obj.CMD=0x15;
							Recode_Obj.Total_pkt_num	= 10;						
					        Recode_Obj.Devide_Flag				= 1;					
							MediaObj.Media_Type		= 3; //行驶记录仪
							  break;
							  break;  
				   default:
                              devide_value=1; // 多媒体分包处理方式
					          break;
                 }

                 


				  if(0==devide_value)
				  	{  // 记录仪分包处理
				       	Recode_Obj.RSD_State	= 1;                //   进入重传状态
						Recode_Obj.RSD_Timer	= 0;                //   清除重传定时器
						Recode_Obj.RSD_Reader = 0;
						Recode_Obj.RSD_total	= UDP_HEX_Rx[15];   // 重传包总数

						//   获取重传列表
						j = 0;
						for( i = 0; i < Recode_Obj.RSD_total; i++ )
						{
							Recode_Obj.Media_ReSdList[i]	= ( UDP_HEX_Rx[16 + j] << 8 ) + UDP_HEX_Rx[17 + j];
							j							+= 2;
						}
						rt_kprintf( "\r\n 记录仪  8003 重传列表Total=%d: ", Recode_Obj.RSD_total );  
							for( i = 0; i < Recode_Obj.RSD_total; i++ )
							{
								rt_kprintf( "%d,", Recode_Obj.Media_ReSdList[i] ); 
							}
							rt_kprintf( "\r\n" );


				  	}
                   else
		            {  //  多媒体分包  
		       
					MediaObj.RSD_State	= 1;                //   进入重传状态
					MediaObj.RSD_Timer	= 0;                //   清除重传定时器
					MediaObj.RSD_Reader = 0;
					MediaObj.RSD_total	= UDP_HEX_Rx[15];   // 重传包总数

					//   获取重传列表
					j = 0;
					for( i = 0; i < MediaObj.RSD_total; i++ )
					{
						MediaObj.Media_ReSdList[i]	= ( UDP_HEX_Rx[16 + j] << 8 ) + UDP_HEX_Rx[17 + j];
						j							+= 2;
					}
					rt_kprintf( "\r\n  8003 重传列表Total=%d: ", MediaObj.RSD_total );   
					if( MediaObj.RSD_total<=100)
					{
						for( i = 0; i < MediaObj.RSD_total; i++ )
						{
							rt_kprintf( "%d,", MediaObj.Media_ReSdList[i] );
						}
						rt_kprintf( "\r\n" );
			         }
		           }
			//==================================================================
           /*      列表重传不要应答 
                     if(Recode_Obj.RSD_total)  // 重传列表项为0 不返回 
			{
				SD_ACKflag.f_CentreCMDack_0001H		= 1;
				Ack_Resualt							= 0;
				SD_ACKflag.f_CentreCMDack_resualt	= Ack_Resualt;
			}
			*/

			break;
		//--------  BD  add  Over  --------------------------------
		default:
			break;
	}

	//-----------------  memset  -------------------------------------
	//memset(UDP_HEX_Rx, 0, sizeof(UDP_HEX_Rx));
	//UDP_hexRx_len= 0;
	return;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Time2BCD( u8 *dest )
{
	dest[0] = ( ( time_now.year / 10 ) << 4 ) + ( time_now.year % 10 );
	dest[1] = ( ( time_now.month / 10 ) << 4 ) + ( time_now.month % 10 );
	dest[2] = ( ( time_now.day / 10 ) << 4 ) + ( time_now.day % 10 );
	dest[3] = ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
	dest[4] = ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
	dest[5] = ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void AvrgSpd_MintProcess( u8 hour, u8 min, u8 sec )
{
	//------- 系统启动时处理   若 标志位为0 且年月日都为 0，表面是系统刚刚启动，这时获取当前时间把当前时间给BAK------
	if( ( 0 == Avrgspd_Mint.saveFlag ) && ( Avrgspd_Mint.datetime_Bak[0] == 0 ) && ( Avrgspd_Mint.datetime_Bak[1] == 0 ) && ( Avrgspd_Mint.datetime_Bak[2] == 0 ) )
	{
		Time2BCD( Avrgspd_Mint.datetime );
		memcpy( Avrgspd_Mint.datetime_Bak, Avrgspd_Mint.datetime, 6 );              //将当前时间给BAK
		avgspd_Mint_Wr = time_now.min;                                              //分钟既是下标	 5-----4
	}else
	{                                                                               // 若不是系统启动开始
		if( min == 0 )                                                              //  把秒数写为非0 ，以便留出时间去存储上1小时的数据
		{
			avgspd_Mint_Wr = 0;                                                     //(xj)分钟数为0，清0
			if( sec == 2 )                                                          //存储上一小时每分钟的平均速度 ==2是因为要保证第59分钟速度已经被存储到相应的寄存器中
			{
				memcpy( Avrgspd_Mint.datetime_Bak, Avrgspd_Mint.datetime, 6 );      //将当前时间给BAK
				Avrgspd_Mint.datetime_Bak[4]	= 0;                                // 分钟为0
				Avrgspd_Mint.datetime_Bak[5]	= 0;                                // 秒也为0
				Avrgspd_Mint.saveFlag			= 1;                                //使能存储      到NandFlag  标志位
				Time2BCD( Avrgspd_Mint.datetime );                                  //获取新的时间
			}
			if( sec == 8 )                                                          //存储完后  清除时间下标
			{
				avgspd_Mint_Wr = 0;
			}
		}
	}
	if( sec == 0 )                                                                  // 秒为0时存储上1分钟的数据
	{
		//-----------------------------------------------------------------------------------------
		//avgspd_Mint_Wr=(Avrgspd_Sec.datetime_Bak[4]>>4)*10+(0x0f&Avrgspd_Sec.datetime_Bak[4]);
		if( AspdCounter )
		{
			Avrgspd_Mint.avgrspd[avgspd_Mint_Wr] = PerMinSpdTotal / AspdCounter;    //第几分钟的平均速度
		}else
		{
			Avrgspd_Mint.avgrspd[avgspd_Mint_Wr] = 0;
		}
		//---------------------------------------------------------------------------------------------
		// rt_kprintf("\r\n	  %2X:%2X  每分钟的平均速度  avrgSpd=%d \r\n",Avrgspd_Sec.datetime_Bak[3],Avrgspd_Sec.datetime_Bak[4], Avrgspd_Mint.avgrspd[avgspd_Mint_Wr]);
		//--------------------------------------------------
		PerMinSpdTotal	= 0;                    // clear
		AspdCounter		= 0;                    // clear
	}else
	{
		if( UDP_dataPacket_flag == 0x02 )
		{
			AspdCounter++;
			PerMinSpdTotal += GPS_speed / 10;   // 只要求精确到 km/h   所以要除 10
		}
	}
}

//==================================================================================================
// 第四部分 :   以下是行车记录仪相关协议 即 附录A
//==================================================================================================

//  1.  上载数据命令字相关


/*
   u8 In_Type  :   传输类型
   u8* InStr   :   参数字符串
   u8 TransType:   传输方式   串口  或  GPRS

 */
void Device_Data( u8 In_Type, u8 TransType )
{
	u8	UpReg[500];
	u16 Up_wr	= 0, SregLen = 0, Greglen = 0, Swr = 0; //,Gwr=0; // S:serial  G: GPRS
	u8	Sfcs	= 0, Gfcs = 0;
	u16 i		= 0;
//  u16  packet_len=0;
	u8	Reg[70];
	u32 regdis		= 0, reg2 = 0;
	u8	QueryRecNum = 0;

	memset( UpReg, 0, 500 );
	Up_wr = 0;

	if( TransType )                                                                     //-------  GPRS  得添加 无线协议头
	{
		memcpy( UpReg + Up_wr, "*GB", 3 );                                              // GPRS 起始头
		Up_wr += 3;

		UpReg[Up_wr++]	= 0x00;                                                         // SIM 号码   最先两个字节填写0x00
		UpReg[Up_wr++]	= 0x00;
		memcpy( UpReg + Up_wr, SIM_code, 6 );
		Up_wr += 6;

		Greglen = Up_wr;                                                                // 长度
		Up_wr	+= 2;

		UpReg[Up_wr++] = 0x00;                                                          //消息序号 默认 0x00

		UpReg[Up_wr++] = 0x20;                                                          //参数  bit5 bit4 10 选择应答遵照协议

		UpReg[Up_wr++] = 0xF0;                                                          // 命令字  无线传输RS232数据

		//   以下是数据内容
	}
	//---------------填写 A 协议头 ------------------------
	Swr				= Up_wr;                                                            // reg save
	UpReg[Up_wr++]	= 0xAA;                                                             // 起始头
	UpReg[Up_wr++]	= 0x75;
	//---------------根据类型分类填写内容------------------
	switch( In_Type )
	{
		//---------------- 上传数类型  -------------------------------------
		case  A_Up_DrvInfo:                                                             //  当前驾驶人信息
			UpReg[Up_wr++] = In_Type;                                                   //命令字

			SregLen			= 0x00;                                                     // 信息长度
			UpReg[Up_wr++]	= 0x00;                                                     // Hi
			UpReg[Up_wr++]	= 39;                                                       // Lo

			UpReg[Up_wr++] = 0x00;                                                      // 保留字

			memcpy( UpReg + Up_wr, JT808Conf_struct.Driver_Info.DriverCard_ID, 18 );    //信息内容
			Up_wr += 18;
			memcpy( UpReg + Up_wr, JT808Conf_struct.Driver_Info.DriveName, 21 );
			Up_wr += 21;

			break;
		case  A_Up_RTC:                                                                 //  采集记录仪的实时时钟
			UpReg[Up_wr++] = In_Type;                                                   //命令字

			SregLen			= 0x00;                                                     // 信息长度
			UpReg[Up_wr++]	= 0x00;                                                     // Hi
			UpReg[Up_wr++]	= 6;                                                        // Lo

			UpReg[Up_wr++] = 0x00;                                                      // 保留字

			Time2BCD( UpReg + Up_wr );                                                  //信息内容
			Up_wr += 6;
			break;
		case  A_Up_Dist:                                                                //  采集里程
			UpReg[Up_wr++] = In_Type;                                                   //命令字

			SregLen			= 0x00;                                                     // 信息长度
			UpReg[Up_wr++]	= 0x00;                                                     // Hi
			UpReg[Up_wr++]	= 16;                                                       // Lo

			UpReg[Up_wr++] = 0x00;                                                      // 保留字
			//   信息内容
			Time2BCD( UpReg + Up_wr );
			Up_wr += 6;
			memcpy( UpReg + Up_wr, (u8*)JT808Conf_struct.FirstSetupDate, 6 );
			Up_wr			+= 6;
			regdis			= JT808Conf_struct.Distance_m_u32 / 100;                    //单位0.1km
			reg2			= regdis / 10000000;
			UpReg[Up_wr++]	= ( reg2 << 4 ) + ( ( regdis % 10000000 ) / 1000000 );
			UpReg[Up_wr++]	= ( ( regdis % 1000000 / 100000 ) << 4 ) + ( regdis % 100000 / 10000 );
			UpReg[Up_wr++]	= ( ( regdis % 10000 / 1000 ) << 4 ) + ( regdis % 1000 / 100 );
			UpReg[Up_wr++]	= ( ( regdis % 100 / 10 ) << 4 ) + ( regdis % 10 );

		case  A_Up_PLUS:                                                                //  采集记录仪速度脉冲系数
			UpReg[Up_wr++] = In_Type;                                                   //命令字

			SregLen			= 0x00;                                                     // 信息长度
			UpReg[Up_wr++]	= 0x00;                                                     // Hi
			UpReg[Up_wr++]	= 10;                                                       // Lo

			UpReg[Up_wr++] = 0x00;                                                      // 保留字

			//  信息内容
			Time2BCD( UpReg + Up_wr );
			Up_wr			+= 6;
			UpReg[Up_wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value << 24 );
			UpReg[Up_wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value << 16 );
			UpReg[Up_wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value << 8 );
			UpReg[Up_wr++]	= (u8)( JT808Conf_struct.Vech_Character_Value );

			break;
		case  A_Up_VechInfo:                                                        //  车辆信息
			UpReg[Up_wr++] = In_Type;                                               //命令字

			SregLen			= 0x00;                                                 // 信息长度
			UpReg[Up_wr++]	= 0x00;                                                 // Hi
			UpReg[Up_wr++]	= 41;                                                   // Lo

			UpReg[Up_wr++] = 0x00;                                                  // 保留字

			memcpy( UpReg + Up_wr, JT808Conf_struct.Vechicle_Info.Vech_VIN, 17 );   //信息内容
			Up_wr += 17;
			memcpy( UpReg + Up_wr, JT808Conf_struct.Vechicle_Info.Vech_Num, 12 );
			Up_wr += 12;
			memcpy( UpReg + Up_wr, JT808Conf_struct.Vechicle_Info.Vech_Type, 12 );
			Up_wr += 12;

			break;
		case  A_Up_AvrgMin:                                         //  每分钟平均速度记录      // 默认填写最新7分钟记录
			UpReg[Up_wr++] = In_Type;                               //命令字

			SregLen			= 455;                                  // 信息长度
			UpReg[Up_wr++]	= (u8)( SregLen >> 8 );                 // Hi
			UpReg[Up_wr++]	= (u8)SregLen;                          // Lo	65x7

			UpReg[Up_wr++] = 0x00;                                  // 保留字
			//-----------  信息内容  --------------
			//----------------------
			QueryRecNum = Api_DFdirectory_Query( spdpermin, 0 );    //查询当前疲劳驾驶记录数目
			if( QueryRecNum > 7 )
			{
				QueryRecNum = 7;
			}

			SregLen				= QueryRecNum * 65;                 // 改写信息长度
			UpReg[Up_wr - 3]	= (u8)( SregLen >> 8 );             // Hi
			UpReg[Up_wr - 2]	= (u8)SregLen;                      // Lo    65x7

			for( i = 0; i < QueryRecNum; i++ )                      // 从最新处读取存储填写
			{
				Api_DFdirectory_Read( spdpermin, Reg, 70, 0, i );   // 从new-->old  读取
				memcpy( UpReg + Up_wr, Reg + 5, 60 );               // 只填写速度
				Up_wr += 65;
			}
			//------------------------------

			break;
		case  A_Up_Tired:                                           //  疲劳驾驶记录
			UpReg[Up_wr++] = In_Type;                               //命令字

			SregLen			= 180;                                  // 信息长度
			UpReg[Up_wr++]	= (u8)( SregLen >> 8 );                 // Hi
			UpReg[Up_wr++]	= (u8)SregLen;                          // Lo	30x6

			UpReg[Up_wr++] = 0x00;                                  // 保留字

			//----------------------------------
			QueryRecNum = Api_DFdirectory_Query( tired_warn, 0 );   //查询当前疲劳驾驶记录数目
			if( QueryRecNum > 6 )
			{
				QueryRecNum = 6;
			}

			SregLen				= 30 * QueryRecNum;                 // 信息长度
			UpReg[Up_wr - 3]	= (u8)( SregLen >> 8 );             // Hi
			UpReg[Up_wr - 2]	= (u8)SregLen;                      // Lo	65x7

			for( i = 0; i < QueryRecNum; i++ )                      // 从最新处读取存储填写
			{
				Api_DFdirectory_Read( tired_warn, Reg, 31, 0, i );  // 从new-->old  读取
				memcpy( UpReg + Up_wr, Reg, 30 );
				Up_wr += 30;
			}
			//----------------------------------

			break;
		//------------------------ 下传数据相关命令 ---------------------
		case  A_Dn_DrvInfo: //  设置车辆信息

		//-----------------------------------------------------------------------


		/*  memset(JT808Conf_struct.Vechicle_Info.Vech_VIN,0,18);
		   memset(JT808Conf_struct.Vechicle_Info.Vech_Num,0,13);
		   memset(JT808Conf_struct.Vechicle_Info.Vech_Type,0,13);

		   memcpy(JT808Conf_struct.Vechicle_Info.Vech_VIN,InStr,17);
		   memcpy(JT808Conf_struct.Vechicle_Info.Vech_Num,InStr+17,12);
		   memcpy(JT808Conf_struct.Vechicle_Info.Vech_Type,InStr+29,12);

		   Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
		 */
		// break;

		case  A_Dn_RTC:                                     //  设置记录仪时间

			UpReg[Up_wr++] = In_Type;                       //命令字

			// 信息长度
			UpReg[Up_wr++]	= 0x00;                         // Hi
			UpReg[Up_wr++]	= 0;                            // Lo   20x8

			UpReg[Up_wr++] = 0x00;                          // 保留字

			break;
		case  A_Dn_Plus:                                    //  设置速度脉冲系数

			//	 JT808Conf_struct.Vech_Character_Value=((u32)(*InStr)<<24)+((u32)(*InStr+1)<<16)+((u32)(*InStr+2)<<8)+(u32)(*InStr+3); // 特征系数	速度脉冲系数
			//	 Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
			//-------------------------------------------------------------
			UpReg[Up_wr++] = In_Type;                       //命令字
			// 信息长度
			UpReg[Up_wr++]	= 0x00;                         // Hi
			UpReg[Up_wr++]	= 0;                            // Lo   20x8
			UpReg[Up_wr++]	= 0x00;                         // 保留字
			break;
		default:
			rt_kprintf( "Error:   Device Type Error! \r\n" );
			return;
	}
	//---------------  填写计算 A 协议  Serial Data   校验位  -------------------------------------
	Sfcs = 0;                                               //	计算S校验 从OxAA 开始
	for( i = Swr; i < Up_wr; i++ )
	{
		Sfcs ^= UpReg[i];
	}
	UpReg[Up_wr++] = Sfcs;                                  // 填写FCS
	//-----------------------------------------------------------------------------
	if( TransType )                                         //-------通过  GPRS
	{
		//----stuff Ginfolen ----
		UpReg[Greglen]		= (u8)( ( Up_wr - 2 ) << 8 );   //长度不算头
		UpReg[Greglen + 1]	= (u8)( Up_wr - 2 );

		Gfcs = 0;                                           //  计算从电话号码开始到校验前数据的异或和  G 协议校验
		for( i = 3; i < Up_wr; i++ )
		{
			Gfcs ^= UpReg[i];
		}

		UpReg[Up_wr++] = Gfcs;                              // 填写G校验位

		UpReg[Up_wr++]	= 0x0D;                             // G 协议尾
		UpReg[Up_wr++]	= 0x0A;
		//=================================================================================================
		//---------通过 GPRS 发送  --------------
		GPRS_infoWr_Tx = 0;                                 //--------------  clear  ---------
		//-----------------------------------------------------------------------
		memcpy( GPRS_info + GPRS_infoWr_Tx, UpReg, Up_wr ); // 拷贝内容到发送缓存区
		GPRS_infoWr_Tx += Up_wr;
		//-----------  Add for Debug ---------------------------------
#if  0
		memset( UDP_AsciiTx, 0, sizeof( UDP_AsciiTx ) );
		strcat( (char*)UDP_AsciiTx, "AT%IPSENDX=1,\"" );
		packet_len		= 14;                               //strlen((const char*)UDP_AsciiTx);
		UDP_AsciiTx_len = HextoAscii( GPRS_info, GPRS_infoWr_Tx, UDP_AsciiTx + packet_len );

		packet_len += UDP_AsciiTx_len;
		strcat( (char*)UDP_AsciiTx, "\"\r\n" );

		以后再说吧
		if( DispContent == 2 )
		{
			Uart1_PutData( (uint8_t*)UDP_AsciiTx, packet_len + 3 );
		}
		GSM_PutData( (uint8_t*)UDP_AsciiTx, packet_len + 3 );
		rt_kprintf( "\r\n	SEND DriveRecord -UP-Data! \r\n");
#endif
	}else
	{   //-----  通过串口输出
		for( i = 0; i < Up_wr; i++ )
		{
			rt_kprintf( "%c", UpReg[i] );
		}
	}
}

//------------------------------------------------------------------
void  Process_GPRSIN_DeviceData( u8 *instr, u16 infolen )
{
	u8	fcs = 0;
	u16 i	= 0;

	//   caculate  and   check fcs
	for( i = 0; i < infolen - 1; i++ )
	{
		fcs ^= instr[i];
	}

	if( fcs != instr[infolen - 1] )
	{
		return;
	}
	//  classify  cmd
	switch( instr[2] )                                          // AAH 75H CMD
	{
		//  上行
		case 0x00:                                              // 采集行车记录仪执行标准版本号
			Adata_ACKflag.A_Flag__Up_Ver_00H = 0xff;
			break;
		case 0x01:                                              // 采集当前驾驶人信息
			Adata_ACKflag.A_Flag_Up_DrvInfo_01H = instr[2];
			break;
		case 0x02:                                              // 采集记录仪的实时时钟
			Adata_ACKflag.A_Flag_Up_RTC_02H = instr[2];
			break;
		case 0x03:                                              // 采集行驶里程
			Adata_ACKflag.A_Flag_Up_Dist_03H = instr[2];
			break;
		case 0x04:                                              // 采集记录仪速度脉冲系数
			Adata_ACKflag.A_Flag_Up_PLUS_04H = instr[2];
			break;
		case 0x06:                                              // 采集车辆信息
			Adata_ACKflag.A_Flag_Up_VechInfo_06H = instr[2];
			break;
		case 0x08:                                              // 采集记录仪状态信号配置信息
			Adata_ACKflag.A_Flag_Up_SetInfo_08H = instr[2];
			break;
		case 0x16:                                              // 采集记录仪唯一编号
			Adata_ACKflag.A_Flag_Up_DevID_16H = instr[2];
			break;
		case 0x09:                                              // 采集指定的每秒钟平均速度记录
			Adata_ACKflag.A_Flag_Up_AvrgSec_09H = instr[2];     // 有起始结束时间
			break;
		case 0x05:                                              // 采集指定的每分钟平均速度记录
			Adata_ACKflag.A_Flag_Up_AvrgMin_05H = instr[2];     // 有起始结束时间
			break;
		case 0x13:                                              // 采集指定的位置信息记录
			Adata_ACKflag.A_Flag_Up_Posit_13H = instr[2];
			break;
		case 0x07:                                              // 采集事故疑点记录
			Adata_ACKflag.A_Flag_Up_Doubt_07H = instr[2];       // 有起始结束时间
			break;
		case 0x11:                                              // 采集指定的疲劳驾驶记录
			Adata_ACKflag.A_Flag_Up_Tired_11H = instr[2];       // 有起始结束时间
			break;
		case 0x10:                                              // 采集指定的登录退出记录
			Adata_ACKflag.A_Flag_Up_LogIn_10H = instr[2];       // 有起始结束时间
			break;
		case 0x14:                                              // 采集指定的记录仪外部供电记录
			Adata_ACKflag.A_Flag_Up_Powercut_14H = instr[2];    // 有起始结束时间
			break;
		case 0x15:                                              // 采集指定的记录仪参数修改记录
			Adata_ACKflag.A_Flag_Up_SetMdfy_15H = instr[2];
			break;
		//  下行
		case 0x82:                                              // 设置车辆信息
			memset( JT808Conf_struct.Vechicle_Info.Vech_VIN, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_VIN ) );
			memset( JT808Conf_struct.Vechicle_Info.Vech_Num, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_Num ) );
			memset( JT808Conf_struct.Vechicle_Info.Vech_Type, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_Type ) );

			//-----------------------------------------------------------------------
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_VIN, instr, 17 );
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_Num, instr + 17, 12 );
			memcpy( JT808Conf_struct.Vechicle_Info.Vech_Type, instr + 29, 12 );

			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );

			Adata_ACKflag.A_Flag_Dn_DrvInfo_82H = instr[2];

			Settingchg_Status				= 0x82;                                                                                                 //设置车辆信息
			NandsaveFlg.Setting_SaveFlag	= 1;                                                                                                    //存储参数修改记录
		case 0x83:                                                                                                                                  // 设置初次安装日期
			memcpy( JT808Conf_struct.FirstSetupDate, instr, 6 );
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			Adata_ACKflag.A_Flag_Dn_SetupDate_83H	= instr[2];
			Settingchg_Status						= 0x83;                                                                                         //设置车辆信息
			NandsaveFlg.Setting_SaveFlag			= 1;                                                                                            //存储参数修改记录
			break;
		case 0x84:                                                                                                                                  // 设置状态量信息
			Settingchg_Status				= 0x84;                                                                                                 //设置车辆信息
			NandsaveFlg.Setting_SaveFlag	= 1;                                                                                                    //存储参数修改记录
			break;
		case 0xc2:                                                                                                                                  // 设置记录仪时钟
			Adata_ACKflag.A_Flag_Dn_RTC_C2H = instr[2];
			Settingchg_Status				= 0xc2;                                                                                                 //设置车辆信息
			NandsaveFlg.Setting_SaveFlag	= 1;                                                                                                    //存储参数修改记录
			break;
		case 0xc3:                                                                                                                                  // 设置记录仪速度脉冲系数
			JT808Conf_struct.Vech_Character_Value = (u32)( instr[0] << 24 ) + (u32)( instr[1] << 16 ) + (u32)( instr[2] << 8 ) + (u32)( instr[3] ); // 特征系数	速度脉冲系数
			Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
			Adata_ACKflag.A_Flag_Dn_Plus_C3H	= instr[2];
			Settingchg_Status					= 0xc3;                                                                                             //设置车辆信息
			NandsaveFlg.Setting_SaveFlag		= 1;                                                                                                //存储参数修改记录
			break;
		default:

			break;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8 Send_Device_Data( void )  //  给行车记录仪发送数据
{
	u8 j = 0, i = 0, reg = 0;;

	j = sizeof( Adata_ACKflag );
	for( i = 0; i < j; i++ )
	{
		reg = ( *( (u8*)&Adata_ACKflag + i ) );
		if( reg )
		{
			if( reg != 0xff )
			{
				Device_Data( reg, 1 );
			}else
			{
				Device_Data( 0x00, 1 );             // 采集行车记录仪执行标准版本号 命令为0x00 置位时填写0xFF,所以单独处理
			}
			( *( (u8*)&Adata_ACKflag + i ) ) = 0;   // clear flag
			return true;
		}
	}
	return false;
}

#if 0


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8 RecordSerial_output_Str( const char *fmt, ... )
{
	u8		regstr[100], fcs = 0;
	u16		reglen = 0, i = 0;

	va_list args;
	va_start( args, fmt );
	memset( regstr, 0, sizeof( regstr ) );
	regstr[0]	= 0x55;                 // 协议头
	regstr[1]	= 0x7A;
	regstr[2]	= 0xFE;                 // 命令字 ，用预留命令字表示调试输出
	//  3,4 为长度字节最后填写
	regstr[5] = 0x00;                   // 备用字  0x00

	reglen = vsprintf( (char*)regstr + 6, fmt, args );
	va_end( args );
	regstr[3]	= (u8)( reglen >> 8 );  // 填写长度  ，长度为信息内容的长度
	regstr[4]	= (u8)reglen;

	reglen	+= 6;
	fcs		= 0;
	for( i = 0; i < reglen; i++ )
	{
		fcs ^= regstr[i];
	}
	regstr[reglen] = fcs;
	reglen++;
	for( i = 0; i < reglen; i++ )
	{
		rt_kprintf( "%c", regstr[i] );
	}

	return 1;
}

#endif


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void SpeedWarnJudge( void )                     //  速度报警判断
{
	//--------  速度报警  -------
	if( JT808Conf_struct.Speed_warn_MAX > 0 )   //> 0
	{
		//----- GPS  即时速度	0.1 km/h  ---------
		if( GPS_speed > ( JT808Conf_struct.Speed_warn_MAX * 10 ) )
		// if( DebugSpd > ( JT808Conf_struct.Speed_warn_MAX*10) )
		{
			speed_Exd.dur_seconds++;
			if( speed_Exd.dur_seconds > JT808Conf_struct.Spd_Exd_LimitSeconds )
			{
				speed_Exd.dur_seconds = 0;
				if( speed_Exd.speed_flag != 1 )
				{
					speed_Exd.speed_flag = 1;
					//PositionsSD_Enable();	  //  回报GPS 信息

					StatusReg_SPD_WARN( );                  //  超速报警状态
					rt_kprintf( "\r\n  超速报警\r\n" );
					TTS_play( " 超速报警触发" );
				}
				//---------------------------------------------
				Time2BCD( speed_Exd.ex_startTime );         //记录超速报警起始时间
				if( speed_Exd.current_maxSpd < GPS_speed )  //找最大速度
				{
					speed_Exd.current_maxSpd = GPS_speed;
				}
				speed_Exd.excd_status = 1;
				speed_Exd.dur_seconds++;

				//----------------------------------------------
			}

			if( speed_Exd.excd_status == 1 )                // 使能flag 后开始计时
			{
				speed_Exd.dur_seconds++;
				if( speed_Exd.current_maxSpd < GPS_speed )  //找最大速度
				{
					speed_Exd.current_maxSpd = GPS_speed;
				}
			}
		}else
		{
			StatusReg_SPD_NORMAL( );                        //  清除速度报警状态寄存器

			if( speed_Exd.excd_status != 2 )
			{
				StatusReg_SPD_NORMAL( );                    //  清除速度报警状态寄存器
				speed_Exd.dur_seconds	= 0;
				speed_Exd.speed_flag	= 0;
			}
			//----------------------------------------------
			if( speed_Exd.excd_status == 1 )
			{
				Time2BCD( speed_Exd.ex_endTime );           //记录超速报警结束时间
				speed_Exd.excd_status = 2;
			}else
			if( speed_Exd.excd_status == 0 )
			{
				Spd_ExpInit( );
			}
			//----------------------------------------------
		}
	} //------- 速度报警 over	 ---
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u16  Protocol_808_Encode( u8 *Dest, u8 *Src, u16 srclen )
{
	u16 lencnt = 0, destcnt = 0;

	for( lencnt = 0; lencnt < srclen; lencnt++ )
	{
		if( Src[lencnt] == 0x7e )           // 7e 转义
		{
			Dest[destcnt++] = 0x7d;
			Dest[destcnt++] = 0x02;
		}else
		if( Src[lencnt] == 0x7d )           //  7d  转义
		{
			Dest[destcnt++] = 0x7d;
			Dest[destcnt++] = 0x01;
		}else
		{
			Dest[destcnt++] = Src[lencnt];  // 原始信息
		}
	}

	return destcnt;                         //返回转义后的长度
}

//-------------------------------------------------------------------------------
void Protocol_808_Decode( void )            // 解析指定buffer :  UDP_HEX_Rx
{
	//-----------------------------------
	u16 i = 0;

	// 1.  clear  write_counter
	UDP_DecodeHex_Len = 0;                  //clear DecodeLen

	// 2   decode process
	for( i = 0; i < UDP_hexRx_len; i++ )
	{
		if( ( UDP_HEX_Rx[i] == 0x7d ) && ( UDP_HEX_Rx[i + 1] == 0x02 ) )
		{
			UDP_HEX_Rx[UDP_DecodeHex_Len] = 0x7e;
			i++;
		}else
		if( ( UDP_HEX_Rx[i] == 0x7d ) && ( UDP_HEX_Rx[i + 1] == 0x01 ) )
		{
			UDP_HEX_Rx[UDP_DecodeHex_Len] = 0x7d;
			i++;
		}else
		{
			UDP_HEX_Rx[UDP_DecodeHex_Len] = UDP_HEX_Rx[i];
		}
		UDP_DecodeHex_Len++;
	}
	//  3.  The  End
}

//---------  拐点补传测试程序  ---------------------------
//#if 0


/*
   void Inflexion_Process(void)
   {            //
   u16  once_delta=0;


   Inflexion_Current=GPS_direction;  //  update new
   //-----------------------------------------------------------------------
    if(Inflexion_Current>Inflexion_Bak)   // 初步判断大小
     {  // 增大
            if((Inflexion_Current-Inflexion_Bak)>300)  // 判断是否倒置减小
   {   //  如果差值大于300 ，说明是小于
        once_delta=Inflexion_Bak+360-Inflexion_Current;  //判断差值绝对值
        InflexDelta_Accumulate+=once_delta;
     if((once_delta>=15)&&(once_delta<=60)) // 角度最小变化率不得小于15度  拐点补传角度不大于180 要求连续3s  所以每秒不大于60
     {
         if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==2))  //判断之前是否一直是小于
          {
                          Inflexion_chgcnter++;
        if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
         {     //要求至少持续3s	累计拐角补传角度不得大于180
                                           InflexLarge_or_Small=0;
             Inflexion_chgcnter=0;
             InflexDelta_Accumulate=0;
             PositionSD_Enable=1; // 发送拐点标志位
             rt_kprintf("\r\n 拐点上报 --1\r\n");
         }
        else
          InflexLarge_or_Small=2; // 这次是小于
          }
      else
       {
          InflexLarge_or_Small=2;  // 这是第一次小于
          Inflexion_chgcnter=1;
          InflexDelta_Accumulate=once_delta;
       }
     }
     else
     {    //  小于 15 就算等于
       InflexLarge_or_Small=0;
       Inflexion_chgcnter=0;
       InflexDelta_Accumulate=0;
     }

            }
   else		// current真真正正的比Bak 大
   {
      once_delta=Inflexion_Current-Inflexion_Bak;  //判断差值绝对值
      InflexDelta_Accumulate+=once_delta;
      if((once_delta>=15)&&(once_delta<=60)) // 角度最小变化率不得小于15度  拐点补传角度不大于180 要求连续3s  所以每秒不大于60
      {
                if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==1)) //判断之前是否一直大于
       {
           Inflexion_chgcnter++;
        if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
         {	 //要求至少持续3s	累计拐角补传角度不得大于180
          InflexLarge_or_Small=0;
          Inflexion_chgcnter=0;
          InflexDelta_Accumulate=0;
          PositionSD_Enable(); // 发送拐点标志位
          rt_kprintf("\r\n 拐点上报 --2\r\n");
         }
        else
          InflexLarge_or_Small=1; // 这次是大于
                }
       else
        {
                       InflexLarge_or_Small=1;  // 这是第一次大于
        Inflexion_chgcnter=1;
        InflexDelta_Accumulate=once_delta;
        }
      }
       else
     {     // 小于15度就算等于
       InflexLarge_or_Small=0;
       Inflexion_chgcnter=0;
       InflexDelta_Accumulate=0;
     }

   }
     }
   else
   if(Inflexion_Current<Inflexion_Bak)
   {  // 减小
               if((Inflexion_Bak-Inflexion_Current)>300)  // 判断是否倒置增大
               { //  如果差值大于300 ，说明是大于
                  once_delta=Inflexion_Current+360-Inflexion_Bak;  //判断差值绝对值
         InflexDelta_Accumulate+=once_delta;
      if((once_delta>=15)&&(once_delta<=60)) // 角度最小变化率不得小于15度	拐点补传角度不大于180 要求连续3s  所以每秒不大于60
                  {   // 最小变化率 不小于 15
                     if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==1))  //判断之前是否一直是大于
          {
                          Inflexion_chgcnter++;
        if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
         {     //要求至少持续3s	累计拐角补传角度不得大于180
                                           InflexLarge_or_Small=0;
             Inflexion_chgcnter=0;
             InflexDelta_Accumulate=0;
             PositionSD_Enable(); // 发送拐点标志位
             rt_kprintf("\r\n 拐点上报 --3\r\n");
         }
        else
          InflexLarge_or_Small=1; // 这次是大于
          }
      else
       {
          InflexLarge_or_Small=1;  // 这是第一次大于
          Inflexion_chgcnter=1;
          InflexDelta_Accumulate=once_delta;
       }

                  }
      else
      {    //  小于 15 就算等于
       InflexLarge_or_Small=0;
       Inflexion_chgcnter=0;
       InflexDelta_Accumulate=0;
      }
     }//---------------------------
      else     // current 真真正正的比Bak 小
      {
      once_delta=Inflexion_Bak-Inflexion_Current;  //判断差值绝对值
      InflexDelta_Accumulate+=once_delta;
      if((once_delta>=15)&&(once_delta<=60)) // 角度最小变化率不得小于15度	拐点补传角度不大于180 要求连续3s  所以每秒不大于60
      {
       if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==2)) //判断之前是否一直小于
       {
        Inflexion_chgcnter++;
        if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
         {  //要求至少持续3s	累计拐角补传角度不得大于180
          InflexLarge_or_Small=0;
          Inflexion_chgcnter=0;
          InflexDelta_Accumulate=0;
          PositionSD_Enable(); // 发送拐点标志位
          rt_kprintf("\r\n 拐点上报 --4\r\n");
         }
        else
         InflexLarge_or_Small=2; // 这次是小于
       }
       else
        {
        InflexLarge_or_Small=2;  // 这是第一次小于
        Inflexion_chgcnter=1;
        InflexDelta_Accumulate=once_delta;
        }
      }
       else
     {	  // 小于15度就算等于
       InflexLarge_or_Small=0;
       Inflexion_chgcnter=0;
       InflexDelta_Accumulate=0;
     }

      }



   }
   else
   {
    InflexLarge_or_Small=0;
    Inflexion_chgcnter=0;
    InflexDelta_Accumulate=0;
   }

    //--------------------------------------------------------
    Inflexion_Bak=Inflexion_Current; //  throw  old	 to  Bak

   }*/
//#endif

void  Sleep_Mode_ConfigEnter( void )
{
	if( SleepState == 0 )
	{
		SleepCounter++;
		if( SleepCounter > 15 )                                             // 防抖处理
		{
			SleepCounter	= 0;
			SleepState		= 1;
			if( JT808Conf_struct.RT_LOCK.Lock_state != 1 )
			{
				Current_SD_Duration = JT808Conf_struct.DURATION.Sleep_Dur;  // 5分钟
			}
			//   修改间隔后立即发送
			PositionSD_Enable( );                                           //发送
			Current_UDP_sd = 1;
			memcpy( BakTime, CurrentTime, 3 );                              // update

			//JT808Conf_struct.DURATION.Heart_SDCnter=25;
			//JT808Conf_struct.DURATION.Heart_Dur=320;


			/*   天地通要求休眠不重新计时  ，不
			   memcpy(BakTime,CurrentTime,3); // update  //gps开始计时
			   systemTick_trigGPS_counter=0; // 系统定时 清除
			 */
			if( DataLink_Status( ) )
			{
				PositionSD_Enable( ); //  在线就发送    河北天地通要求数据完整
			}
			rt_kprintf( "\r\n 进入休眠状态! \r\n" );
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  Sleep_Mode_ConfigExit( void )
{
	if( SleepState == 1 )
	{
		SleepState = 0;
		rt_kprintf( "\r\n 车台唤醒! \r\n" );
	}
	if( JT808Conf_struct.RT_LOCK.Lock_state != 1 )
	{
		Current_SD_Duration = JT808Conf_struct.DURATION.Default_Dur;
	}
	JT808Conf_struct.DURATION.Heart_Dur = 90;
	SleepState							= 0;
	SleepCounter						= 0;
}

#if 0


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u16 WaveFile_EncodeHeader( u32 inFilesize, u8* DestStr )
{
	u32 Filesize = 0, i = 0;                // Header Len  =44Bytes

	//  1. RIFF
	memcpy( DestStr, "RIFF", 4 );
	i += 4;
	//  2. Wave 文件 大小  小端模式
	Filesize = 0x24 + ( inFilesize << 3 );  // 乘以16 加 36 wave 文件大小
	rt_kprintf( "\r\n .wav 文件大小: %d Rawdata: %d \r\n ", Filesize, ( inFilesize << 3 ) );
	DestStr[i++]	= Filesize;             // LL
	DestStr[i++]	= ( Filesize >> 8 );    //LH
	DestStr[i++]	= ( Filesize >> 16 );   //HL
	DestStr[i++]	= ( Filesize >> 24 );   //HH
	//  3. WAVE string
	memcpy( DestStr + i, "WAVE", 4 );
	i += 4;
	//  4. fmt string
	memcpy( DestStr + i, "fmt ", 4 );
	i += 4;
	//  5. PCM Code
	DestStr[i++]	= 0x10;                 // LL
	DestStr[i++]	= 0x00;                 //LH
	DestStr[i++]	= 0x00;                 //HL
	DestStr[i++]	= 0x00;                 //HH
	//  6. Audio Format  PCM=1
	DestStr[i++]	= 0x01;                 // L
	DestStr[i++]	= 0x00;                 //H
	//  7. NumChannels  通道数
	DestStr[i++]	= 0x01;                 // L
	DestStr[i++]	= 0x00;                 //H
	//  8. SampleRate     8000<=>0x00001F40    16000<=>0x00003E80
	DestStr[i++]	= 0x40;                 //0x40; // LL
	DestStr[i++]	= 0x1F;                 //0x1F;//LH
	DestStr[i++]	= 0x00;                 //HL
	DestStr[i++]	= 0x00;                 //HH
	//  9.ByteRate       == SampleRate * NumChannels * BitsPerSample/8  ==8000x1x8/8
	DestStr[i++]	= 0x40;                 //0x40; // LL
	DestStr[i++]	= 0x1F;                 //0x1F;//LH
	DestStr[i++]	= 0x00;                 //HL
	DestStr[i++]	= 0x00;                 //HH

	// 10.BlockAlign    == NumChannels * BitsPerSample/8
	DestStr[i++]	= 0x01;                 //0x02;//0x01; // L
	DestStr[i++]	= 0x00;                 //H
	// 11.BitsPerSample
	DestStr[i++]	= 0x08;                 //0x10;//0x08; // L
	DestStr[i++]	= 0x00;                 //H
	// 12.data string
	memcpy( DestStr + i, "data", 4 );
	i += 4;
	// 13 .datasize
	Filesize		= ( inFilesize << 3 );  // 乘以16 加 36 wave 文件大小
	DestStr[i++]	= Filesize;             // LL
	DestStr[i++]	= ( Filesize >> 8 );    //LH
	DestStr[i++]	= ( Filesize >> 16 );   //HL
	DestStr[i++]	= ( Filesize >> 24 );   //HH

	return i;
}

#endif

//-----------  starttime[6]
u8 CurrentTime_Judge( u8*startTime, u8* endTime )
{
	u32 daycaculate_current		= 0, daycaculate_start = 0, daycaculate_end = 0;
	u32 secondcaculate_current	= 0, secondcaculate_start = 0, secondcaculate_end = 0;

	daycaculate_start		= ( ( startTime[0] >> 4 ) * 10 + ( startTime[0] & 0x0f ) ) * 365 + ( ( startTime[1] >> 4 ) * 10 + ( startTime[1] & 0x0f ) ) * 30 + ( ( startTime[2] >> 4 ) * 10 + ( startTime[2] & 0x0f ) );
	secondcaculate_start	= ( ( startTime[3] >> 4 ) * 10 + ( startTime[3] & 0x0f ) ) * 60 + ( ( startTime[4] >> 4 ) * 10 + ( startTime[4] & 0x0f ) ) * 60 + ( ( startTime[5] >> 4 ) * 10 + ( startTime[5] & 0x0f ) );

	daycaculate_end		= ( ( endTime[0] >> 4 ) * 10 + ( endTime[0] & 0x0f ) ) * 365 + ( ( endTime[1] >> 4 ) * 10 + ( endTime[1] & 0x0f ) ) * 30 + ( ( endTime[2] >> 4 ) * 10 + ( endTime[2] & 0x0f ) );
	secondcaculate_end	= ( ( endTime[3] >> 4 ) * 10 + ( endTime[3] & 0x0f ) ) * 60 + ( ( endTime[4] >> 4 ) * 10 + ( endTime[4] & 0x0f ) ) * 60 + ( ( endTime[5] >> 4 ) * 10 + ( endTime[5] & 0x0f ) );

	daycaculate_current		= ( time_now.year ) * 365 + time_now.month * 30 + time_now.day;
	secondcaculate_current	= time_now.hour * 60 + time_now.min * 60 + time_now.sec;

	if( ( daycaculate_current > daycaculate_start ) && ( daycaculate_current < daycaculate_end ) )
	{
		return true;
	}else
	if( ( secondcaculate_current >= secondcaculate_start ) && ( secondcaculate_current <= secondcaculate_end ) )
	{
		return true;
	}else
	{
		return false;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void CycleRail_Judge( u8* LatiStr, u8* LongiStr )
{
#if 0


	/*
	    纬度没有差值    1纬度  111km
	    40度纬度上 1经度为  85.3km   (北京地区)
	 */
	u8	i				= 0;
	u32 Latitude		= 0, Longitude = 0;
	u32 DeltaLatiDis	= 0, DeltaLongiDis = 0, CacuDist = 0;
	u8	InOutState		= 0; //   0 表示 in   1  表示Out

	//  1. get value
	Latitude	= ( LatiStr[0] << 24 ) + ( LatiStr[1] << 16 ) + ( LatiStr[2] << 8 ) + LatiStr[3];
	Longitude	= ( LongiStr[0] << 24 ) + ( LongiStr[1] << 16 ) + ( LongiStr[2] << 8 ) + LongiStr[3];

	for( i = 0; i < 8; i++ )
	{
		InOutState = 0;
		memset( (u8*)&Rail_Cycle, 0, sizeof( Rail_Cycle ) );
		Api_RecordNum_Read( Rail_cycle, i + 1, (u8*)&Rail_Cycle, sizeof( Rail_Cycle ) );
		// rt_kprintf("\r\n\r\n 圆形围栏 有效状态:%d  TYPE: %d    atrri=%d  lati: %d  longiti:%d  radicus:%d  maxspd: %d  keepdur:%d \r\n",Rail_Cycle.Effective_flag,Rail_Cycle.Area_ID,Rail_Cycle.Area_attribute,Rail_Cycle.Center_Latitude,Rail_Cycle.Center_Longitude,Rail_Cycle.Radius,Rail_Cycle.MaxSpd,Rail_Cycle.KeepDur);

		if( Rail_Cycle.Effective_flag == 1 )
		{
			DeltaLatiDis = abs( Latitude - Rail_Cycle.Center_Latitude ) / 9;                //  a/1000000*111000=a/9.009	除以一百万得到度数 再乘以 111000 米得到实际距离

			DeltaLongiDis = abs( Longitude - Rail_Cycle.Center_Longitude ) * 853 / 10000;   // a/1000000*85300=a 853/10000 m

			CacuDist = sqrt( ( DeltaLatiDis * DeltaLatiDis ) + ( DeltaLongiDis * DeltaLongiDis ) );

			rt_kprintf( "\r\n  TemperLati  %d  TemperLongi	%d	  Centerlati %d  center longi %d\r\n", Latitude, Longitude, Rail_Cycle.Center_Latitude, Rail_Cycle.Center_Longitude );
			rt_kprintf( "\r\n  he=%d heng=%d   shu=%d   juli=%d\r\n", abs( Longitude - Rail_Cycle.Center_Longitude ), DeltaLongiDis, DeltaLatiDis, CacuDist );

			if( DeltaLatiDis > Rail_Cycle.Radius )
			{                                                                                   // 如果纬度距离大于 半径肯定出
				InOutState = 1;
			}else
			{
				DeltaLongiDis = abs( Longitude - Rail_Cycle.Center_Longitude ) * 853 / 10000;   // a/1000000*85300=a 853/10000 m
				if( DeltaLongiDis > Rail_Cycle.Radius )
				{                                                                               // 如果经度距离大于半径肯定出
					InOutState = 1;
				}else //  计算两点见距离
				{
					CacuDist = sqrt( ( DeltaLatiDis * DeltaLatiDis ) + ( DeltaLongiDis * DeltaLongiDis ) );
				}
			}

			// 1. 判断属性
			if( Rail_Cycle.Area_attribute & 0x0001 ) //Bit 0 根据时间
			{
				if( CurrentTime_Judge( Rail_Cycle.StartTimeBCD, Rail_Cycle.EndTimeBCD ) == false )
				{
					rt_kprintf( "\r\n 时段没在区间内 \r\n" );
					return;
				}
				//continue;
			}
			if( Rail_Cycle.Area_attribute & 0x0002 )    //Bit 1 限速
			{
				if( GPS_speed > Rail_Cycle.MaxSpd )
				{
					StatusReg_SPD_WARN( );              //  超速报警状态
					rt_kprintf( "\r\n  设定围栏超速报警\r\n" );
				}else
				{
					StatusReg_SPD_NORMAL( );
				}
				//continue;
			}
			if( Rail_Cycle.Area_attribute & 0x0004 )                                                    //Bit 2 进区域报警给驾驶员
			{
				//continue;
			}
			if( Rail_Cycle.Area_attribute & 0x0008 )                                                    //Bit 3 进区域报警给平台
			{
				if( ( InOutState == 0 ) && ( CacuDist < Rail_Cycle.Radius ) && ( Rail_Cycle.MaxSpd > ( Speed_gps / 10 ) ) )
				{
					if( InOut_Object.Keep_state != 1 )                                                  // 如果没在in ，报一次
					{
						Warn_Status[1]			|= 0x10;                                                // 进出区域报警
						InOut_Object.TYPE		= 1;                                                    //圆形区域
						InOut_Object.ID			= i;                                                    //  ID
						InOut_Object.InOutState = 0;                                                    //  进报警
						rt_kprintf( "\r\n -----圆形电子围栏--入报警" );
					}
					break;
				}
				//continue;
			}
			if( Rail_Cycle.Area_attribute & 0x0010 )                                                    //Bit 4 出区域报警给司机
			{
				;
				//continue;
			}
			if( ( Rail_Cycle.Area_attribute & 0x0020 ) && ( Rail_Cycle.MaxSpd > ( Speed_gps / 10 ) ) )  //Bit 5 出区域报警给平台
			{
				if( ( InOutState == 1 ) || ( CacuDist > Rail_Cycle.Radius ) )
				{
					Warn_Status[1]			|= 0x10;                                                    // 进出区域报警
					InOut_Object.TYPE		= 1;                                                        //圆形区域
					InOut_Object.ID			= i;                                                        //  ID
					InOut_Object.InOutState = 1;                                                        //  出报警
					rt_kprintf( "\r\n -----圆形电子围栏--出报警" );
					break;
				}

				//continue;
			}
			if( Rail_Cycle.Area_attribute & 0x0100 )                                                    //bit8  --BD 允许开门
			{
				;                                                                                       // continue
			}
			if( Rail_Cycle.Area_attribute & 0x4000 )                                                    //bit14  --BD  进区域 开启通信模块
			{
				;                                                                                       // continue
			}
			if( Rail_Cycle.Area_attribute & 0x8000 )                                                    //bit15  --BD  进区域 采集详细信息
			{
				;                                                                                       // continue
			}
		}
	}

#endif
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void RectangleRail_Judge( u8* LatiStr, u8* LongiStr )
{
	u8	i			= 0;
	u32 Latitude	= 0, Longitude = 0;
//	u32 DeltaLatiDis=0,DeltaLongiDis=0,CacuDist=0;
	u8	InOutState = 1; //	 0 表示 in	 1	表示Out

	//  1. get value
	Latitude	= ( LatiStr[0] << 24 ) + ( LatiStr[1] << 16 ) + ( LatiStr[2] << 8 ) + LatiStr[3];
	Longitude	= ( LongiStr[0] << 24 ) + ( LongiStr[1] << 16 ) + ( LongiStr[2] << 8 ) + LongiStr[3];

	// rt_kprintf("\r\n  1---TemperLati  %d  TemperLongi	%d	 res %d\r\n",Latitude,Longitude,InOutState);

	for( i = 0; i < 8; i++ )
	{
		InOutState = 1;
		//Api_RecordNum_Read( Rail_rect, i + 1, (u8*)&Rail_Rectangle, sizeof( Rail_Rectangle ) );		
		memcpy((u8*)&Rail_Rectangle,(u8*)&Rail_Rectangle_multi[i],sizeof(Rail_Rectangle));	
		

		if( Rail_Rectangle.Effective_flag == 1 )
		{
			//  rt_kprintf("\r\n\r\n 判断矩形形围栏 有效:%d ID: %d  atrri=%X  leftlati: %d  leftlongiti:%d    rightLati:%d   rightLongitu: %d	\r\n",Rail_Rectangle.Effective_flag,i+1,Rail_Rectangle.Area_attribute,Rail_Rectangle.LeftUp_Latitude,Rail_Rectangle.LeftUp_Longitude,Rail_Rectangle.RightDown_Latitude,Rail_Rectangle.RightDown_Longitude);
			if( ( Latitude > Rail_Rectangle.RightDown_Latitude ) && ( Latitude < Rail_Rectangle.LeftUp_Latitude ) && ( Longitude > Rail_Rectangle.LeftUp_Longitude ) && ( Longitude < Rail_Rectangle.RightDown_Longitude ) )
			{
				InOutState = 0;
			}

			//rt_kprintf("\r\n  TemperLati  %d  TemperLongi  %d   res %d\r\n",Latitude,Longitude,InOutState);

			// 1. 判断属性
			if( Rail_Rectangle.Area_attribute & 0x0001 )    //Bit 0 根据时间
			{
				//continue;
			}
			if( Rail_Rectangle.Area_attribute & 0x0002 )    //Bit 1 限速
			{
				//continue;
			}
			if( Rail_Rectangle.Area_attribute & 0x0004 )    //Bit 2 进区域报警给驾驶员
			{
				//continue;
			}
			if( Rail_Rectangle.Area_attribute & 0x0008 )    //Bit 3 进区域报警给平台
			{
				if( InOutState == 0 )
				{
					if( InOut_Object.Keep_state == 2 )      // 如果没在in ，报一次
					{
						Warn_Status[1]			|= 0x10;    // 进出区域报警
						InOut_Object.TYPE		= 2;        //矩形区域
						InOut_Object.ID			= i;        //	ID
						InOut_Object.InOutState = 0;        //  进报警
						InOut_Object.Keep_state = 1;        //在围栏内
						PositionSD_Enable( );
						Current_UDP_sd = 1;
						rt_kprintf( "\r\n -----矩形电子围栏--入报警" );
						gps_raw( "3" );  // 使能原始GPS 数据信息上传
					}
					InOut_Object.Keep_state = 1;            //在围栏内	//
					break;
				}
				//continue;
			}
			if( Rail_Rectangle.Area_attribute & 0x0010 )    //Bit 4 出区域报警给司机
			{
				// continue;
			}
			if( Rail_Rectangle.Area_attribute & 0x0020 )    //Bit 5 出区域报警给平台
			{
				if( InOutState == 1 )
				{
					if( InOut_Object.Keep_state == 1 )      // 如果没在out ，报一次
					{
						Warn_Status[1]			|= 0x10;    // 进出区域报警
						InOut_Object.TYPE		= 2;        //矩形区域
						InOut_Object.ID			= i;        //	ID
						InOut_Object.InOutState = 1;        //  出报警
						InOut_Object.Keep_state = 2;        //在围栏外
						PositionSD_Enable( );
						Current_UDP_sd = 1;
						rt_kprintf( "\r\n -----矩形电子围栏--出报警" );
						gps_raw( "0" );
					}
					InOut_Object.Keep_state = 2;            //在围栏外
					break;
				}
			}
			// continue;
			if( Rail_Rectangle.Area_attribute & 0x4000 )    //Bit 14 开启关闭通信模块
			{
				if( InOutState == 0 )
				{
					;                                       //  关闭通信


					   if( JT808Conf_struct.Close_CommunicateFlag==0)
					           {
					                  close_com();
									  mq_true_enable(2);  //  使能盲区补报存储
					                rt_kprintf("\r\n -----矩形电子围栏--关闭通信");

					           }	
					break;
				}else
				if( InOutState == 1 )
				{
					; // 开启通信
					if( JT808Conf_struct.Close_CommunicateFlag == 1 )
					{
						 open_com( "1" );
						 mq_true_enable(1); // 使能盲区补报上报 
						rt_kprintf( "\r\n -----矩形电子围栏--开启通信" );
					}
					break;
				}
			}
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void RouteLineWarn_judge( u8* LatiStr, u8* LongiStr )
{
	//  u32 Latitude=0,Longitude=0;

	//  1. get value
	// Latitude=(LatiStr[0]<<24)+(LatiStr[1]<<16)+(LatiStr[2]<<8)+LatiStr[3];
	// Longitude=(LongiStr[0]<<24)+(LongiStr[1]<<16)+(LongiStr[2]<<8)+LongiStr[3];

	//13 03 05 10 15 32 A 39587815N116000189E   内-> 外
	if( ( Temp_Gps_Gprs.Date[0] == 13 ) && ( Temp_Gps_Gprs.Date[1] == 3 ) && ( Temp_Gps_Gprs.Date[2] == 5 ) && \
	    ( Temp_Gps_Gprs.Time[0] == 18 ) && ( Temp_Gps_Gprs.Time[1] == 15 ) && \
	    ( ( Temp_Gps_Gprs.Time[2] == 32 ) || ( Temp_Gps_Gprs.Time[2] == 33 ) ) )
	{
		if( ( Warn_Status[1] & 0x80 ) == 0 )    //  如果以前没触发，那么及时上报
		{
			PositionSD_Enable( );
			Current_UDP_sd = 1;
			rt_kprintf( "\r\n     出线路 !\r\n" );
		}
		Warn_Status[1] |= 0x80;                 // 路线偏航报警
	}

	//13 03 05 10 21 32 A 39578028N116000001E	   外-> 内
	if( ( Temp_Gps_Gprs.Date[0] == 13 ) && ( Temp_Gps_Gprs.Date[1] == 3 ) && ( Temp_Gps_Gprs.Date[2] == 5 ) && \
	    ( Temp_Gps_Gprs.Time[0] == 18 ) && ( Temp_Gps_Gprs.Time[1] == 21 ) && \
	    ( ( Temp_Gps_Gprs.Time[2] == 32 ) || ( Temp_Gps_Gprs.Time[2] == 33 ) ) )
	{
		if( Warn_Status[1] & 0x80 ) //  如果以前没触发，那么及时上报
		{
			PositionSD_Enable( );
			Current_UDP_sd = 1;
			rt_kprintf( "\r\n     进 线路 !\r\n" );
		}
		Warn_Status[1] &= ~0x80;    // 路线偏航报警
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void RouteRail_Judge( u8* LatiStr, u8* LongiStr )
{
	/*
	    纬度没有差值    1纬度  111km
	    40度纬度上 1经度为  85.3km   (北京地区)
	 */
	u8	i			= 0;
	u8	route_cout	= 0, seg_count = 0, seg_num = 0;
	u32 Latitude	= 0, Longitude = 0;
	// u32 DeltaLatiDis=0,DeltaLongiDis=0,CacuDist=0;
	// u8  InOutState=0;   //   0 表示 in   1  表示Out
	u32 Route_Status	= 0;    // 每个bit 表示 一个路线 偏航状态默认为0
	u32 Segment_Status	= 0;    //  当前线路中，对应端的偏航情况， 默认为0
	u32 Distance		= 0;
//     u8    InAreaJudge=0; //  判断是否在判断区域 bit 0 经度范围 bit  1 纬度范围
	u32 Distance_Array[6];      //存储当条线路的最小距离，默认是个大数值

	//  1. get value
	Latitude	= ( LatiStr[0] << 24 ) + ( LatiStr[1] << 16 ) + ( LatiStr[2] << 8 ) + LatiStr[3];
	Longitude	= ( LongiStr[0] << 24 ) + ( LongiStr[1] << 16 ) + ( LongiStr[2] << 8 ) + LongiStr[3];

	// rt_kprintf("\r\n 当前---->  Latitude:   %d     Longitude: %d\r\n",Latitude,Longitude);

	//  2.  Judge
	for( route_cout = 0; route_cout < Route_Mum; route_cout++ )                 // 读取路线
	{
		// 2.1  --------   读取路线-----------
		memset( (u8*)&ROUTE_Obj, 0, sizeof( ROUTE_Obj ) );                      //  clear all  first
		DF_ReadFlash( DF_Route_Page + route_cout, 0, (u8*)&ROUTE_Obj, sizeof( ROUTE_Obj ) );
		DF_delay_us( 20 );
		//rt_kprintf("\r\n -----> ROUTE_Obj.RouteID:   %d \r\n",ROUTE_Obj.Route_ID);
		// 2.2  -----  判断是否有效  -------
		if( ( ROUTE_Obj.Effective_flag == 1 ) && ( ROUTE_Obj.Points_Num > 1 ) ) //  判断是否有效且有拐点，若无效不处理
		{
			// 2.2.0    当前段距离付给一个大的数值
			for( i = 0; i < 6; i++ )
			{
				Distance_Array[i] = ROUTE_DIS_Default;
			}
			// 2.2.1      计算段数
			seg_num = ROUTE_Obj.Points_Num - 1; // 线路段数目
			//  2.2.2    判断路线中每一段的状态
			Segment_Status = 0;                 // 清除段判断状态，每个线路重新开始一次
			for( seg_count = 0; seg_count < seg_num; seg_count++ )
			{
				if( ( ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Latitude == 0 ) && ( ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Longitude == 0 ) )
				{
					rt_kprintf( "\r\n  该点为0 ，jump\r\n" );
					continue;
				}
				//----- 开始做距离计算, 在没在区域在函数里边做了判断
				Distance_Array[seg_count] = Distance_Point2Line( Latitude, Longitude, ROUTE_Obj.RoutePoints[seg_count].POINT_Latitude, ROUTE_Obj.RoutePoints[seg_count].POINT_Longitude, ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Latitude, ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Longitude );
			}
			//=========================================================
			//  2.4 ------  打印显示距离，找出最小数值----
			Distance = Distance_Array[0]; // 最小距离
			for( i = 0; i < 6; i++ )
			{
				if( Distance >= Distance_Array[i] )
				{
					Distance = Distance_Array[i];
				}
				// rt_kprintf("\r\n  Distance[%d]=%d",i,Distance_Array[i]);
			}
			rt_kprintf( "\r\n MinDistance =%d  Width=%d \r\n", Distance, ( ROUTE_Obj.RoutePoints[seg_num].Width >> 1 ) ); //

			if( Distance < ROUTE_DIS_Default )
			{
				//  ---- 和路段宽度做对比
				if( Distance > ( ROUTE_Obj.RoutePoints[seg_num].Width >> 1 ) )
				{
					rt_kprintf( "\r\n 路线偏离\r\n" );
					Segment_Status |= ( 1 << seg_num ); //  把相应的bit  置位
				}
			}

			//
		}
		// 2.4  根据 2.2 结果判断当期路线状态
		if( Segment_Status )
		{
			Route_Status |= ( 1 << route_cout );    //  把相应的bit  置位
		}
	}
	// 3.  Result
	if( Route_Status )
	{
		if( ( Warn_Status[1] & 0x80 ) == 0 )        //  如果以前没触发，那么及时上报
		{
			PositionSD_Enable( );
			Current_UDP_sd = 1;
		}

		Warn_Status[1] |= 0x80;                     // 路线偏航报警
		rt_kprintf( "\r\n    路径偏航触发 !\r\n" );
	}else
	{
		if( Warn_Status[1] & 0x80 )                 //  如果以前没触发，那么及时上报
		{
			PositionSD_Enable( );
			Current_UDP_sd = 1;
		}

		Warn_Status[1] &= ~0x80;                    // 路线偏航报警
	}
}

//--------  D点到直线距离计算-------


/*
     P1(x1,y1)   P2(x2,y2)  ,把点P(x1,y2)作为坐标原点，即x1=0，y2=0；

     那么两点P1，P2 确定的直线方程(两点式)为:
             (x-x1)/(x2-x1) =(y-y1)/(y2-y1)                          (1)

    注:  标准式直线方程为 AX+BY+C=0;
             那么平面上任意一点P(x0,y0) 到直线的距离表示为
             d=abs(Ax0+By0+C)/sqrt(A^2+B^2)

    其中把方程式(1) 转换成标准式为:
            (y2-y1)x+(x1-x2)y+x1(y1-y2)+y1(x2-x1)=0;

   由于点(x1,y2)为原点  即x1=0，y2=0；  P1(0,y1) , P2(x2,0)
    所以   A=-y1 ,  B=-x2, C=y1x2
    那么 直线的方程:
                  -y1x-x2y+y1x2=0;  (2)

   =>     d=abs(-y1x0-x2y0+y1x2)/sqrt(y1^2+x2^2)       (3)

         其中 (3)  为最终应用的公式

        注:  先根据经纬度折合计算出 x0，y0，x1,y1,x2,y2  的数值单位为: 米
   =>  区域判断:
           根据(2) 可以求出  过 P1(0,y1) , P2(x2,0) 点与已知直线垂直的两条直线方程
              P1(0,y1) :      x2x-y1y+y1^2=0  (4)
              P2(x2,0) :      x2x-y1y-x2^2=0  (5)

          如果 y1 >=0      直线(4)    在直线(5)  的上边
          那么 点在线段区域内的判断方法是
                       (4) <=  0    且  (5)  >=0
       另
           如果 y1 <=0      直线(5)    在直线(4)  的上边
          那么 点在线段区域内的判断方法是
                       (4) >=  0    且  (5)  <=0
   //------------------------------------------------------------------------------------------

       纬度没有差值    1纬度  111km
       40度纬度上 1经度为  85.3km   (北京地区)

       X 轴为 经度(longitude) 差值
       Y 轴为纬度 (latitude)  差值


   //------------------------------------------------------------------------------------------
 */

u32   Distance_Point2Line( u32 Cur_Lat, u32 Cur_Longi, u32 P1_Lat, u32 P1_Longi, u32 P2_Lat, u32 P2_Longi )
{                                                                           //   输入当前点 ，返回点到既有直线的距离
	long	x0			= 0, y0 = 0, Line4_Resualt = 0, Line5_Resualt = 0;  // 单位: 米
	long	y1			= 0;
	long	x2			= 0;
	long	distance	= 0;
	// long  Rabs=0;
//      long  Rsqrt=0;
	long	DeltaA1 = 0, DeltaA2 = 0, DeltaO1 = 0, DeltaO2 = 0;             //  DeltaA : Latitude     DeltaO:  Longitude
	// u32   Line4_Resualt2=0,Line5_Resualt2=0;
	double	fx0				= 0, fy0 = 0, fy1 = 0, fx2 = 0;
	double	FLine4_Resualt2 = 0, FLine5_Resualt2 = 0, fRabs = 0, fRsqrt = 0;

	// 0.   先粗略的判断
	DeltaA1 = abs( Cur_Lat - P1_Lat );
	DeltaA2 = abs( Cur_Lat - P2_Lat );
	DeltaO1 = abs( Cur_Lat - P1_Longi );
	DeltaO2 = abs( Cur_Lat - P2_Longi );


	/* if((DeltaA1>1000000) &&(DeltaA2>1000000))
	        {
	            rt_kprintf("\r\n  Latitude 差太大\r\n");
	            return   ROUTE_DIS_Default;
	   }
	   if((DeltaO1>1000000) &&(DeltaO2>1000000))
	        {
	            rt_kprintf("\r\n  Longitude 差太大\r\n");
	            return   ROUTE_DIS_Default;
	   }
	 */
	// 1.  获取  P1(0,y1)   P2(x2,0) ,和P(x0,y0)    P(x1,y2)为原点  即x1=0，y2=0；  P1(0,y1) , P2(x2,0)
	x2 = abs( P2_Longi - P1_Longi ); // a/1000000*85300=a 853/10000 m =a x 0.0853
	if( P2_Longi < P1_Longi )
	{
		x2 = 0 - x2;
	}
	fx2 = (double)( (double)x2 / 1000 );
	//rt_kprintf("\r\n P2_L=%d,P1_L=%d   delta=%d \r\n",P2_Longi,P1_Longi,(P2_Longi-P1_Longi));
	// if(P2_Longi
	y1 = abs( P2_Lat - P1_Lat ); //  a/1000000*111000=a/9.009	除以一百万得到度数 再乘以 111000 米得到实际距离
	if( P2_Lat < P1_Lat )
	{
		y1 = 0 - y1;
	}
	fy1 = (double)( (double)y1 / 1000 );
	//rt_kprintf("\r\n P2_LA=%d,P1_LA=%d   delta=%d \r\n",P2_Lat,P1_Lat,(P2_Lat-P1_Lat));

	//   rt_kprintf("\r\n 已知两点坐标: P1(0,%d)   P2(%d,0) \r\n", y1,x2);
	//    当前点
	x0 = abs( Cur_Longi - P1_Longi );
	if( Cur_Longi < P1_Longi )
	{
		x0 = 0 - x0;
	}
	fx0 = (double)( (double)x0 / 1000 );
	//rt_kprintf("\r\n Cur_L=%d,P1_L=%d   delta=%d \r\n",Cur_Longi,P1_Longi,(Cur_Longi-P1_Longi));

	y0 = abs( Cur_Lat - P2_Lat ); //  a/1000000*111000=a/9.009
	if( Cur_Lat < P2_Lat )
	{
		y0 = 0 - y0;
	}
	fy0 = (double)( (double)y0 / 1000 );
	// rt_kprintf("\r\n Cur_La=%d,P2_La=%d   delta=%d \r\n",Cur_Lat,P2_Lat,(Cur_Lat-P2_Lat));
	//   rt_kprintf("\r\n当前点坐标: P0(%d,%d)    \r\n", x0,y0);
	// 2. 判断y1  的大小， 求出过 P1(0,y1)   P2(x2,0) ,和已知直线的方程，并判断
	//     当前点是否在路段垂直范围内

	//  2.1   将当前点带入， 过 P1(0,y1)   的 直线方程(4)  求出结果
	Line4_Resualt	= ( x2 * x0 ) - ( y1 * y0 ) + ( y1 * y1 );
	FLine4_Resualt2 = fx2 * fx0 - fy1 * fy0 + fy1 * fy1;
	//     rt_kprintf("\r\n Line4=x2*x0-y1*y0+y1*y1=(%d)*(%d)-(%d)*(%d)+(%d)*(%d)=%ld     x2*x0=%d    y1*y0=%d   y1*y1=%d  \r\n",x2,x0,y1,y0,y1,y1,Line4_Resualt,x2*x0,y1*y0,y1*y1);
	//     rt_kprintf("\r\n FLine4=fx2*fx0-fy1*fy0+fy1*fy1=(%f)*(%f)-(%f)*(%f)+(%f)*(%f)=%f      fx2*fx0=%f    fy1*fy0=%f   fy1*fy1=%f  \r\n",fx2,fx0,fy1,fy0,fy1,fy1,FLine4_Resualt2,fx2*fx0,fy1*fy0,fy1*fy1);

	//   2.2   将当前点带入， 过P2(x2,0) 的 直线方程(5)  求出结果
	Line5_Resualt	= ( x2 * x0 ) - y1 * y0 - x2 * x2;
	FLine5_Resualt2 = fx2 * fx0 - fy1 * fy0 - fx2 * fx2;
	//rt_kprintf("\r\n Line5=x2*x0-y1*y0-x2*x2=(%d)*(%d)-(%d)*(%d)-(%d)*(%d)=%ld     Se : %ld   \r\n",x2,x0,y1,y0,x2,x2,Line5_Resualt,Line5_Resualt2);
	//    rt_kprintf("\r\n FLine5=fx2*fx0-fy1*fy0-fx2*fx2=(%f)*(%f)-(%f)*(%f)-(%f)*(%f)=%f      fx2*fx0=%f    fy1*fy0=%f   fx2*fx2=%f  \r\n",fx2,fx0,fy1,fy0,fx2,fx2,FLine5_Resualt2,fx2*fx0,fy1*fy0,fx2*fx2);
	// rt_kprintf("\r\n  Line4_Resualt=%d     Line5_Resualt=%d  \r\n",Line4_Resualt,Line5_Resualt);

	if( fy1 >= 0 )                      //  直线(4) 在上发
	{
		//   2.3   判断区域    (4) <=  0    且  (5)  >=0     // 判断条件取反
		if( ( FLine4_Resualt2 > 0 ) || ( FLine5_Resualt2 < 0 ) )
		{
			return ROUTE_DIS_Default;   //  不满足条件返回最大数值
		}
	} else
	{                                   //  直线(5)
		//   2.4   判断区域     (4) >=  0    且  (5)  <=0     // 判断条件取反
		if( ( FLine4_Resualt2 < 0 ) || ( FLine5_Resualt2 > 0 ) )
		{
			return ROUTE_DIS_Default;   //  不满足条件返回最大数值
		}
	}

	rt_kprintf( "\r\n In judge area \r\n" );
	//rt_kprintf("\r\n   Current== Latitude:   %d     Longitude: %d     Point1== Latitude:   %d     Longitude: %d     Point2== Latitude:   %d     Longitude: %d\r\n",Cur_Lat,Cur_Longi,P1_Lat,P1_Longi,P2_Lat,P2_Longi);

	//  3. 将差值差算成实际距离
#if 0
	x2	= x2 * 0.0853;  // a/1000000*85300=a 853/10000 m =a x 0.0853
	y1	= y1 / 9;       //  a/1000000*111000=a/9.009	除以一百万得到度数 再乘以 111000 米得到实际距离
	x0	= x0 * 0.0853;
	y0	= y0 / 9;       //  a/1000000*111000=a/9.009
#else
	fx2 = fx2 * 0.0853; // a/1000000*85300=a 853/10000 m =a x 0.0853
	fy1 = fy1 / 9;      //  a/1000000*111000=a/9.009	除以一百万得到度数 再乘以 111000 米得到实际距离
	fx0 = fx0 * 0.0853;
	fy0 = fy0 / 9;      //  a/1000000*111000=a/9.009
#endif

	//  4. 计算距离
	//Rabs=0-y1*x0-x2*y0+y1*x2;
	// rt_kprintf("\r\n Test -y1*x0=%d -y0*x2=%d  y1*x2=%d   Rabs=%d  \r\n",0-y1*x0,0-y0*x2,0-y1*x2,Rabs);
#if 0
	Rabs	= abs( -y1 * x0 - x2 * y0 + y1 * x2 );
	Rsqrt	= sqrt( y1 * y1 + x2 * x2 );
	// distance=abs(-y1*x0-x2*y0-y1*x2)/sqrt(y1*y1+x2*x2);
	distance = Rabs / Rsqrt;
	// rt_kprintf("\r\n Rabs=%d    Rsqrt=%d   d=%d",Rabs,Rsqrt,distance);
#else
	fRabs	= abs( -fy1 * fx0 - fx2 * fy0 + fy1 * fx2 );
	fRsqrt	= sqrt( fy1 * fy1 + fx2 * fx2 );
	// distance=abs(-y1*x0-x2*y0-y1*x2)/sqrt(y1*y1+x2*x2);
	distance = (long)( ( fRabs / fRsqrt ) * 1000 );
	// rt_kprintf("\r\n Rabs=%d    Rsqrt=%d   d=%d",Rabs,Rsqrt,distance);
#endif

	return distance;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
unsigned short int CRC16_file( unsigned short int num )
{
#if  0
	unsigned short int	i = 0, j = 0;
	unsigned char		buffer_temp[514];
	memset( buffer_temp, 0, sizeof( buffer_temp ) );
	for( i = 0; i < num; i++ )
	{
		if( i == 0 )                //第一包
		{
			Last_crc		= 0;    // clear first
			crc_fcs			= 0;
			buffer_temp[0]	= 0;
			buffer_temp[1]	= 51;
			DF_ReadFlash( 51, 0, &buffer_temp[2], PageSIZE );
			WatchDog_Feed( );

			Last_crc = CRC16_1( buffer_temp, 514, 0xffff );
			rt_kprintf( "\r\ni=%d,j=%d,Last_crc=%x", i, j, Last_crc );
		}else if( i == ( num - 1 ) ) //最后一包
		{
			buffer_temp[0]	= 0;
			buffer_temp[1]	= 50;
			DF_ReadFlash( 50, 0, &buffer_temp[2], PageSIZE );
			FileTCB_CRC16	= ( (unsigned short int)buffer_temp[512] << 8 ) + (unsigned short int)buffer_temp[513];
			crc_fcs			= CRC16_1( buffer_temp, 512, Last_crc );
			rt_kprintf( "\r\ni=%d,j=%d,Last_crc=%x ReadCrc=%x ", i, j, crc_fcs, FileTCB_CRC16 );
		}else
		{   // 中间的包
			j				= i + 51;
			buffer_temp[0]	= (char)( j >> 8 );
			buffer_temp[1]	= (char)j;
			DF_ReadFlash( j, 0, &buffer_temp[2], PageSIZE );
			WatchDog_Feed( );
			Last_crc = CRC16_1( buffer_temp, 514, Last_crc );
			//rt_kprintf("\r\ni=%d,j=%d,Last_crc=%d",i,j,Last_crc);
		}
	}
	rt_kprintf( "\r\n  校验结果 %x", crc_fcs );
	return crc_fcs;
#endif
	return 1;
}

//-------  JT808  Related   Save  Process---------
void  Save_Status( u8 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec )
{    // 存储事故疑点 最近20s车辆的状态字，和当前最近的有效位置信息
	u16				wr_add = 3, j = 0, cur = 0;
	u8				FCS;
	u8				regDateTime[6];
	static uint8_t	Statustmp[500];
//		u32  latitude=0,longitude=0;

	Time2BCD( regDateTime );
	//----------------------------------------------------------------------------
	wr_add = 0;
	memcpy( Statustmp, regDateTime, 6 );
	wr_add += 6;
	//-----------------------  Status Register   --------------------------------
	cur = save_sensorCounter;                               //20s的事故疑点
	for( j = 0; j < 100; j++ )
	{
		Statustmp[wr_add++] = Sensor_buf[cur].DOUBTspeed;   //速度
		Statustmp[wr_add++] = Sensor_buf[cur].DOUBTstatus;  //状态
		cur++;
		if( cur > 100 )
		{
			cur = 0;
		}
	}
	//---------------------------------------------------------------------------------------
	FCS = 0;
	for( j = 0; j < wr_add; j++ )
	{
		FCS ^= Statustmp[j];
	} //求上边数据的异或和      不算校验 信息长度为   206
	  //------------------------------------------------------------------------------
	Statustmp[wr_add++] = FCS;
	//-----------------------------------------------------------------------------
	rt_kprintf( "\r\n 存储行车记录  Record  write: %d    SaveLen: %d    \r\n", Recorder_write, wr_add );
	Api_DFdirectory_Write( doubt_data, Statustmp, wr_add );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Spd_Exp_Wr( void )
{
	u8	content[40];
	u8	wr_add = 0, i = 0, FCS = 0;

	memset( content, 0, sizeof( content ) );
	memcpy( content + wr_add, JT808Conf_struct.Driver_Info.DriverCard_ID, 18 );
	wr_add += 18;
	memcpy( content + wr_add, speed_Exd.ex_startTime, 6 );
	wr_add += 6;
	memcpy( content + wr_add, speed_Exd.ex_endTime, 6 );
	wr_add				+= 6;
	content[wr_add++]	= speed_Exd.current_maxSpd / 10;

	FCS = 0;
	for( i = 0; i < 32; i++ )
	{
		FCS ^= content[i];
	}                           //求上边数据的异或和
	content[wr_add++] = FCS;    // 第31字节

	Api_DFdirectory_Write( spd_warn, (u8*)content, 32 );
	//----------- debug -----------------------
	rt_kprintf( "\r\n 超速报警  %X-%X-%X %X:%X:%X,MaxSpd=%d\r\n", speed_Exd.ex_endTime[0], speed_Exd.ex_endTime[1], speed_Exd.ex_endTime[2], speed_Exd.ex_endTime[3], speed_Exd.ex_endTime[4], speed_Exd.ex_endTime[5], speed_Exd.current_maxSpd );
	//--------- clear status ----------------------------
	Spd_ExpInit( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  JT808_Related_Save_Process( void )
{
	if( DF_LOCK )
	{
		return;
	}

	if( Dev_Voice.CMD_Type != '1' ) // 录音时不做以下处理
	{
		//-------------------------
		//    存储疑点数据
		if( 1 == NandsaveFlg.Doubt_SaveFlag )
		{
			if( sensor_writeOverFlag == 1 )
			{
				time_now = Get_RTC( ); //  RTC  相关
				Save_Status( time_now.year, time_now.month, time_now.day, time_now.hour, time_now.min, time_now.sec );
				NandsaveFlg.Doubt_SaveFlag = 0;
				return;
			}
		}

		//-----------------  超速报警 ----------------------
		if( speed_Exd.excd_status == 2 )
		{
			Spd_Exp_Wr( );
			return;
		}


		/*
		   if(NandsaveFlg.Setting_SaveFlag==1)   // 参数 修改记录
		   {
		   Save_Common(Settingchg_write,TYPE_SettingChgAdd);
		   Settingchg_write++;
		   if(Settingchg_write>=Max_CommonNum)
		   Settingchg_write=0;
		   DF_Write_RecordAdd(Settingchg_write,Settingchg_read,TYPE_SettingChgAdd);
		   NandsaveFlg.Setting_SaveFlag=0; // clear flag
		   }
		 */
	}
	//	 定时存储里程
	if( ( Vehicle_RunStatus ) && ( ( Systerm_Reset_counter & 0xff ) == 0xff ) )
	{   //  如果车辆在行驶过程中，每255 秒存储一次里程数据
		Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
		return;
	}

	//--------------------------------------------------------------
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8   Time_FastJudge( void )
{
	u32 Date_value1		= 0, Time_value1 = 0;
	u32 Date_valueRTC	= 0, Time_valueRTC = 0;

	Date_value1		= Gps_Gprs.Date[0] * 365 + Gps_Gprs.Date[1] * 30 + Gps_Gprs.Date[2];
	Date_valueRTC	= time_now.year * 3600 + time_now.day * 60 + time_now.day;
	Time_value1		= Gps_Gprs.Time[0] * 365 + Gps_Gprs.Time[1] * 30 + Gps_Gprs.Time[2];
	Time_valueRTC	= time_now.hour * 3600 + time_now.min * 60 + time_now.sec;

	if( Date_value1 > Date_valueRTC )
	{
		rt_kprintf( "\r\n  日期超前，Kick  OUT  !\r\n" );
		StatusReg_GPS_V( ); //  不定位
		return false;
	}
	if( ( Time_value1 > Time_valueRTC ) && ( ( Time_value1 - Time_valueRTC ) > 120 ) )
	{
		rt_kprintf( "\r\n  时间超前，Kick  OUT  !\r\n" );
		StatusReg_GPS_V( ); // 不定位
		return false;
	}
	return true;
}

/*
    打印输出 HEX 信息，Descrip : 描述信息 ，instr :打印信息， inlen: 打印长度
 */
void OutPrint_HEX( u8 * Descrip, u8 *instr, u16 inlen )
{
	u32 i = 0;
	rt_kprintf( "\r\n %s:", Descrip );
	for( i = 0; i < inlen; i++ )
	{
		rt_kprintf( "%02X ", instr[i] );
	}
	rt_kprintf( "\r\n" );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Tired_Check( void )
{
	/*if( DispContent == 2 )
	{
		rt_kprintf( "\r\n				速度: %d Km/h    Debugspd=%d KM/h \r\n", GPS_speed / 10, DebugSpd / 10 );
	}*/
	if( GPS_speed > 60 )                                                                                                        // GPS_speed 单位为0.1 km/h  速度大于6 km/h  认为是行驶
	// if(DebugSpd>60)
	{
		//-------------疲劳驾驶相关 -----------------------
		TiredConf_struct.Tired_drive.ACC_ONstate_counter++;                                                                     // ACC 累计工作时间
		if( DispContent == 5 )
		{
			rt_kprintf( "\r\n  ACC ON = %d", TiredConf_struct.Tired_drive.ACC_ONstate_counter );
		}
		if( TiredConf_struct.Tired_drive.ACC_ONstate_counter > 3 )                                                              //ACC 开 485 外设开电，防抖处理
		{
			Power_485CH1_ON;                                                                                                    // 第一路485的电        上电工作
		}

		TiredConf_struct.Tired_drive.ACC_Offstate_counter = 0;                                                                  // clear
		//------ 停车超时相关  -----------
		TiredConf_struct.TiredDoor.Parking_currentcnt	= 0;                                                                    // Clear  停车 计数器清 0
		Warn_Status[1]									&= ~0x08;                                                               // 停车超时清除
		//---------------------------------
		if( TiredConf_struct.Tired_drive.ACC_ONstate_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 - 30 ) )   //提前5分钟蜂鸣器提示注意疲劳驾驶 14100
		{
			TiredConf_struct.Tired_drive.Tgvoice_play = 1;
			TTS_play( "您还有20秒就疲劳驾驶，请注意安全" );
		}

		Status_TiredwhRst = 1;                                                                                                  // 没疲劳时为1
		if( TiredConf_struct.Tired_drive.ACC_ONstate_counter >= ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 ) )        //14400 // 连续驾驶超过4小时算疲劳驾驶
		{
			TiredConf_struct.Tired_drive.Tgvoice_play = 0;

			Status_TiredwhRst								= 2;                                                                // 疲劳了就为2了
			TiredConf_struct.Tired_drive.Tireddrv_status	= 1;                                                                // 记录疲劳驾驶的状态
			if( TiredConf_struct.Tired_drive.ACC_ONstate_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 ) )    //14400
			{
				//Tired_drive.Tgvoice_play=1;  // 开始播放疲劳驾驶语音提示
				// Tired_drive.voicePly_counter++;
				Time2BCD( TiredConf_struct.Tired_drive.start_time );
				rt_kprintf( "\r\n   疲劳驾驶触发了! \r\n" );
				TTS_play( "您已经疲劳驾驶，请注意休息" );
				tts_bro_tired_flag	= 1;
				Warn_Status[3]		|= 0x04; //BIT(2)  疲劳驾驶
				//---- 触发即时上报数据-------
				PositionSD_Enable( );
				Current_UDP_sd = 1;
				//-------------------------------------
				Rstart_time = 1;
			}
			//--------- 蜂鸣器提示相关	关闭蜂鸣器 记录下疲劳驾驶  ------------
			Warn_Status[3]								|= 0x04;                                                                    //BIT(2)  疲劳驾驶
			TiredConf_struct.Tired_drive.Tgvoice_play	= 0;
		}
		TiredConf_struct.Tired_drive.Flag = 1;
	}else
	{
		//------------ 疲劳驾驶相关  ----------------------
		if( TiredConf_struct.Tired_drive.Flag == 1 )                                                                                //只有从ACCon切换过来时才做以下处理，如果长期关着，没有必要了
		{
			//-- ACC 没有休息前还算AccON 的状态  ---------------
			TiredConf_struct.Tired_drive.ACC_ONstate_counter++;                                                                     // ACC 累计工作时间

			if( DispContent == 5 )
			{
				rt_kprintf( "\r\n	ACC ON 2 = %d, GPS_speed = %d", TiredConf_struct.Tired_drive.ACC_ONstate_counter, GPS_speed );
			}
			if( TiredConf_struct.Tired_drive.ACC_ONstate_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 - 30 ) )   //提前5分钟蜂鸣器提示注意疲劳驾驶 14100
			{
				TiredConf_struct.Tired_drive.Tgvoice_play = 1;
				TTS_play( "您还有20秒就疲劳驾驶，请注意安全" );
			}

			if( TiredConf_struct.Tired_drive.ACC_ONstate_counter >= ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 ) )        //14400 // 连续驾驶超过4小时算疲劳驾驶
			{
				Status_TiredwhRst								= 2;                                                                // 疲劳了就为2了
				TiredConf_struct.Tired_drive.Tireddrv_status	= 1;                                                                // 记录疲劳驾驶的状态
				if( TiredConf_struct.Tired_drive.ACC_ONstate_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec + 40 ) )    //14400
				{
					Time2BCD( TiredConf_struct.Tired_drive.start_time );
					rt_kprintf( "\r\n	 速度小，但未满足休息门限时间 疲劳驾驶触发了! \r\n");
					TTS_play( "您已经疲劳驾驶，请注意休息" );
					tts_bro_tired_flag	= 1;
					Warn_Status[3]		|= 0x04;                                                                                    //BIT(2)  疲劳驾驶
					//---- 触发即时上报数据-------
					PositionSD_Enable( );
					Current_UDP_sd = 1;
					//-------------------------------------
					Rstart_time = 1;
				}
				Warn_Status[3] |= 0x04;                                                                                             //BIT(2)  疲劳驾驶
			}
			//------------------------------------------------------------------------------
			TiredConf_struct.Tired_drive.ACC_Offstate_counter++;
			if( DispContent == 5 )
			{
				rt_kprintf( "\r\n  ACC OFF = %d, GPS_speed = %d", TiredConf_struct.Tired_drive.ACC_Offstate_counter, GPS_speed );
			}
			if( TiredConf_struct.Tired_drive.ACC_Offstate_counter >= TiredConf_struct.TiredDoor.Door_MinSleepSec )  //1200	// ACC 关20分钟视为休息
			{
				if( TiredConf_struct.Tired_drive.ACC_Offstate_counter == TiredConf_struct.TiredDoor.Door_MinSleepSec )
				{
					TiredConf_struct.Tired_drive.ACC_Offstate_counter	= 0;
					TiredConf_struct.Tired_drive.ACC_ONstate_counter	= 0;
					Warn_Status[3]										&= ~0x04;                                   //BIT(2)	疲劳驾驶
					if( tts_bro_tired_flag == 1 )
					{
						TTS_play( "您的疲劳驾驶报警已经解除" );
						tts_bro_tired_flag = 0;
					}
					// Status_TiredwhRst=0;
					// DF_WriteFlashSector(DF_TiredStartTime_Page,0,(u8*)&Status_TiredwhRst,1); //清除O后要写入
					//---- 触发即时上报数据-------
					PositionSD_Enable( );
					Current_UDP_sd = 1;
					//-------------------------------------	                          rt_kprintf("\r\n   ACC  休息了 \r\n");
					if( Rstart_time == 1 )
					{
						TiredConf_struct.Tired_drive.Flag = 0; ///////
						//Tired_drive.ACC_ONstate_counter=0;

						Rstart_time										= 0;
						TiredConf_struct.Tired_drive.Tireddrv_status	= 2;
						//rt_kprintf("\r\n疲劳结束时间写入");
					}
				}
			}
		}

		//----------  超长停车相关 ------------------------------------------------------
		if( GPS_speed < 3 )                                                                                         //(GPS_speed==0)  // 停车超时从速度为0时开始
		//if(DebugSpd<3)
		{
			TiredConf_struct.TiredDoor.Parking_currentcnt++;
			//if(DispContent==2)
			//  printf("\r\n @#@\r\n");
			if( TiredConf_struct.TiredDoor.Parking_currentcnt >= TiredConf_struct.TiredDoor.Door_MaxParkingSec )    //TiredDoor.Door_MaxParkingSec)
			{                                                                                                       //  停车大于门限值，就算停车了
				Warn_Status[1] |= 0x08;                                                                             // 停车超时触发
				if( TiredConf_struct.TiredDoor.Parking_currentcnt == TiredConf_struct.TiredDoor.Door_MaxParkingSec )
				{
					//---- 触发即时上报数据-------
					PositionSD_Enable( );
					Current_UDP_sd = 1;
					//-------------------------------------
					TTS_play( " 超时停车触发" );
					rt_kprintf( "\r\n 超时停车触发! \r\n" );
				}
			}
		}else
		{
			TiredConf_struct.TiredDoor.Parking_currentcnt = 0;
		}
		//--------------------------------------------------------------------------------
	}
}

//E:\work_xj\F4_BD\北斗行车记录仪过检(新协议)\2-21 RT-Thread_NewBoard-LCD2-Jiance\bsp\stm32f407vgt6_RecDrv\app_712\lcd\Menu_0_3_Sim.c
void CAN_struct_init( void )
{
	//-------  protocol variables
	CAN_trans.can1_sample_dur	= 0;        // can1 采集间隔  ms
	CAN_trans.can1_trans_dur	= 0;        // can1  上报间隔 s
	CAN_trans.can1_enable_get	= 0;        // 500ms

	CAN_trans.can2_sample_dur	= 0;        // can2 采集间隔  ms
	CAN_trans.can2_trans_dur	= 0;        // can2  上报间隔 s

	//   u8      canid_1[8];// 原始设置
	CAN_trans.canid_1_Filter_ID = 0;        // 接收判断用
	//u8      canid_1_Rxbuf[400]; // 接收buffer
	CAN_trans.canid_1_RxWr			= 0;    // 写buffer下标
	CAN_trans.canid_1_SdWr			= 0;
	CAN_trans.canid_1_ext_state		= 0;    // 扩展帧状态
	CAN_trans.canid_1_sample_dur	= 10;   // 改ID 的采集间隔
	CAN_trans.canid_ID_enableGet	= 0;

	//------- system variables
	CAN_trans.canid_timer		= 0;        //定时器
	CAN_trans.canid_0705_sdFlag = 0;        // 发送标志位

	CAN_trans.nonew_CANdataflag;            //没有新数据来标志位
	CAN_trans.nonew_CANdata_timer;          //  计时器
	CAN_trans.BAK_WR = 0;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  CAN_send_timer( void )
{
	u16 i = 0, datanum = 0;;
	if( CAN_trans.can1_trans_dur > 0 )
	{
		CAN_trans.canid_timer++;
		// if( CAN_trans.canid_timer>=CAN_trans.can1_trans_dur)
		if( CAN_trans.canid_timer >= 4 )
		{
			CAN_trans.canid_timer = 0;
			//------  判断有没有数据项
			if( CAN_trans.canid_1_RxWr )
			{
				datanum = ( CAN_trans.canid_1_RxWr >> 3 );
				memcpy( CAN_trans.canid_1_Sdbuf, CAN_trans.canid_1_Rxbuf, CAN_trans.canid_1_RxWr );
				CAN_trans.canid_1_SdWr = CAN_trans.canid_1_RxWr;
				for( i = 0; i < datanum; i++ )
				{
					CAN_trans.canid_1_ID_SdBUF[i] = CAN_trans.canid_1_ID_RxBUF[i];
				}

				CAN_trans.canid_1_RxWr		= 0; // clear
				CAN_trans.canid_0705_sdFlag = 1;
			}else
			{
				rt_kprintf( "\r\n rx=%d  dur=%ds   CAN send trigger ,no data \r\n", CAN_trans.canid_1_RxWr, CAN_trans.can1_trans_dur );
			}
		}
	}else
	{
		CAN_trans.canid_0705_sdFlag = 0;
		CAN_trans.canid_timer		= 0;
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  CAN_Send_judge( void )
{
	u16 i = 0, datanum = 0;;

	datanum = ( CAN_trans.canid_1_RxWr >> 3 );

	if( datanum >= 35 )
	{
		datanum = ( CAN_trans.canid_1_RxWr >> 3 );
		memcpy( CAN_trans.canid_1_Sdbuf, CAN_trans.canid_1_Rxbuf, CAN_trans.canid_1_RxWr );
		CAN_trans.canid_1_SdWr = CAN_trans.canid_1_RxWr;
		for( i = 0; i < datanum; i++ )
		{
			CAN_trans.canid_1_ID_SdBUF[i] = CAN_trans.canid_1_ID_RxBUF[i];
		}

		CAN_trans.canid_1_RxWr		= 0; // clear
		CAN_trans.canid_0705_sdFlag = 1;
	}
}

//--------------  SMS Msg  Send Related  ------------------------
void SMS_send_init( void )
{
	SMS_send.Msg_sdState	= 0;
	SMS_send.Msg_step		= 0;
	SMS_send.Msg_timer		= 0;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void SMS_send_process( void )
{
	if( SMS_send.Msg_sdState == 1 )
	{
		switch( SMS_send.Msg_step )
		{
			case 1:                         // 发送命令AT+CMGS="13051953513"    //13602069191
				rt_hw_gsm_output( "AT+CMGS=\"13051953513\"\r\n" );
				SMS_send.Msg_step	= 2;
				SMS_send.Msg_timer	= 0;    // clear  timer   and  start counter
				rt_kprintf( "AT+CMGS=\"13051953513\"\r\n" );
				break;
			case  2:    break;
			case  4:                        // 发送内容 0x1A 结束
				rt_hw_gsm_output( "ProductNum:70420  Type: TW705  DeviceNum:03 " );
				SMS_send.Msg_step = 5;
				rt_kprintf( "ProductNum:70420  Type: TW705  DeviceNum:03 \x1a" );
				break;
			case  7:
				rt_hw_gsm_output( "\x1a" );
				SMS_send.Msg_step = 8;
				break;
			default:
				break;
		}
	}
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  SMS_send_Step( u8 step )
{
	SMS_send.Msg_step = step;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
u8  SMS_send_stateQuery( void )
{
	return SMS_send.Msg_sdState;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  SMS_send_enable( void )
{
	//  要发送短信
	SMS_send.Msg_sdState	= 1;
	SMS_send.Msg_step		= 1;
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  SMS_send_end( void )
{
	SMS_send_init( );
}

/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void SMS_Send_timer( void )
{
	if( SMS_send.Msg_sdState == 1 )
	{
		SMS_send.Msg_timer++;
		if( SMS_send.Msg_timer >= 10 ) // 2s
		{
			SMS_send.Msg_timer = 0;
			SMS_send_end( );
			rt_kprintf( " \r\n   SMS send timeout !\r\n" );
		}
		//-----------------------------
		if( SMS_send.Msg_step == 6 )
		{
			SMS_send.Msg_step = 7;
		}

		if( SMS_send.Msg_step == 5 )
		{
			SMS_send.Msg_step = 6;
		}

		if( SMS_send.Msg_step == 3 )
		{
			SMS_send.Msg_step = 4;
		}

		if( SMS_send.Msg_step == 2 )
		{
			rt_hw_gsm_output( "\r\n" );
			SMS_send.Msg_step = 3;
		}
		//-----------------------------
	}
}

//--------------------------------------------------------------------------
int use_gpsSpd( void )
{
	JT808Conf_struct.Speed_GetType		= 0;
	JT808Conf_struct.DF_K_adjustState	= 0; // clear  Flag
	ModuleStatus						&= ~Status_Pcheck;

	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
	rt_kprintf( " \r\n  ------------------------设置从GPS获取速度  Speed_GetType= %d", JT808Conf_struct.Speed_GetType );
	return 1;
}

FINSH_FUNCTION_EXPORT( use_gpsSpd, usegpsspd );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
int use_sensorspd( void )
{
	JT808Conf_struct.Speed_GetType		= 1;
	JT808Conf_struct.DF_K_adjustState	= 1; // clear  Flag
	ModuleStatus						|= Status_Pcheck;
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
	rt_kprintf( " \r\n  ------------------------设置从Sensor获取速度	Speed_GetType= %d", JT808Conf_struct.Speed_GetType );
	return 1;
}

FINSH_FUNCTION_EXPORT( use_sensorspd, sensorSpd );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  Init_original( void )
{
	if( SysConf_struct.Version_ID == SYSID )    //  check  wether need  update  or not
	{
		SysConf_struct.Version_ID = SYSID + 1;
		Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
		Systerm_Reset_counter	= Max_SystemCounter;
		ISP_resetFlag			= 2;            //   借助远程下载重启机制复位系统
	}
	rt_kprintf( "\r\n 手动恢复出厂设置 \r\n" );
}

FINSH_FUNCTION_EXPORT( Init_original, Init_original );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  apn_set( u8 * Content )
{
	u8 infolen = 0;

	infolen = strlen( Content );
	memset( APN_String, 0, sizeof( APN_String ) );
	memcpy( APN_String, (char*)Content, infolen );
	memset( (u8*)SysConf_struct.APN_str, 0, sizeof( APN_String ) );
	memcpy( (u8*)SysConf_struct.APN_str, (char*)Content, infolen );
	Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
	DataLink_APN_Set( APN_String, 1 );
	rt_kprintf( "\r\n 手动设置APN \r\n" );
}

FINSH_FUNCTION_EXPORT( apn_set, Set APN_String );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  dur( u8 *content )
{
	sscanf( content, "%d", (u32*)&Current_SD_Duration );
	rt_kprintf( "\r\n 手动设置上报时间间隔 %d s\r\n", Current_SD_Duration );

	JT808Conf_struct.DURATION.Default_Dur = Current_SD_Duration;
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
}

FINSH_FUNCTION_EXPORT( dur, Set APN_String );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void idset( u8 *ID )
{
	u8 len = 0;

	if( strlen( ID ) != 12 )
	{
		rt_kprintf( "\r\nID  长度不够\r\n" ); return;
	}

	memcpy( (char*)JT808Conf_struct.Vechicle_Info.Vech_sim, ID, 12 );
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );

	memcpy( (char*)IMSI_CODE + 3, (char*)JT808Conf_struct.Vechicle_Info.Vech_sim, 12 );
	IMSI_Convert_SIMCODE( ); //  translate
	rt_kprintf( "\r\n ID=%s\r\n", ID );
}

FINSH_FUNCTION_EXPORT( idset, idset );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void recoverset( void )
{
	FirstRun_Config_Write( ); // 里边更新了 SYSID
}

FINSH_FUNCTION_EXPORT( recoverset, recoverset );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void chepai( u8 *instr )
{
	memset( JT808Conf_struct.Vechicle_Info.Vech_Num, 0, sizeof( JT808Conf_struct.Vechicle_Info.Vech_Num ) );
	memcpy( JT808Conf_struct.Vechicle_Info.Vech_Num, instr, 8 );
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
}

FINSH_FUNCTION_EXPORT( chepai, chepai );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  localtrig( u8 *instr )
{
	local_trig = instr[0] - 0x30;
	if( local_trig )
	{
		GPS_getfirst = 1; local_timer = 0;
	}else
	{
		GPS_getfirst = 0; local_timer = 0;
	}
	rt_kprintf( "\r\n Enable local  time=%d  \r\n", local_trig );
}

FINSH_FUNCTION_EXPORT( localtrig, localtrig );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  current( void )
{
	PositionSD_Enable( );
	Current_UDP_sd = 1;
	memcpy( BakTime, CurrentTime, 3 ); // update

	//  rt_kprintf("\r\n---> tim1  %d  \r\n",TIM1_Timer_Counter);
	//TIM1_Timer_Counter=0;
}

FINSH_FUNCTION_EXPORT( current, current );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void txt_play( u8 *instr )
{
	u16 len = strlen( instr );

	//--------------  播报----
	Dev_Voice.CMD_Type = '2';
	memset( Dev_Voice.Play_info, 0, sizeof( Dev_Voice.Play_info ) );
	memcpy( Dev_Voice.Play_info, instr, len );
	Dev_Voice.info_sdFlag = 1;

	//#ifdef LCD_5inch
	//DwinLCD.Type=LCD_SDTXT;
	// memset(DwinLCD.TXT_content,0,sizeof(DwinLCD.TXT_content));
	//DwinLCD.TXT_contentLen=AsciiToGb(DwinLCD.TXT_content,infolen-1,UDP_HEX_Rx+14);
	//#endif
	//  TTS
	TTS_Get_Data( instr, len );

#ifdef LCD_5inch
	//======  信息都在屏幕上显示
	DwinLCD.Type = LCD_SDTXT;
	memset( DwinLCD.TXT_content, 0, sizeof( DwinLCD.TXT_content ) );
	DwinLCD.TXT_contentLen = AsciiToGb( DwinLCD.TXT_content, len, instr );
#endif
}

FINSH_FUNCTION_EXPORT( txt_play, txt_play );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void ICack( void )
{
	u8	i			= 0;
	u8	ackreg[24]	= { 0x00, 0x01, 0xf1, 0xd8, 0x37, 0x25, 0x61, 0xe8, 0x30, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x39 };

	memcpy( IC_MOD.IC_Rx, ackreg, 24 );
	rt_kprintf( "\r\n IC 卡透传len=%dBytes  RX:", 24 );
	for( i = 0; i < 24; i++ )
	{
		rt_kprintf( "%2X ", ackreg[i] );
	}
	rt_kprintf( "\r\n" );
	//------ 直接发送给IC 卡模块-----
	DeviceData_Encode_Send( 0x0B, 0x40, ackreg, 24 ); 
}

FINSH_FUNCTION_EXPORT( ICack, ICack ); 


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  msg_send( void )
{
	SMS_send_enable( );
	rt_kprintf( "\r\n   收到使能发送短信\r\n" );
}

FINSH_FUNCTION_EXPORT( msg_send, msg_send );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void close_com( void )
{
	JT808Conf_struct.Close_CommunicateFlag = 1;
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
	Close_DataLink( );
	rt_kprintf( "\r\nclose com\r\n" );
}

FINSH_FUNCTION_EXPORT( close_com, close_com );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void open_com( u8 *intstr )
{
	rt_kprintf( "\r\n open com\r\n" );
	redial( );
	JT808Conf_struct.Close_CommunicateFlag = 0;
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );

	if( intstr[0] == '0' )
	{
		Mangqu_Init( );
		MangQU.Enable_SD_state = 0;
	}else
	if( intstr[0] == '1' )
	{
		MangQU.Enable_SD_state = 1;
		// MangQU.PacketNum=0;
		MangQU.PacketNum = 0;
		BlindZoneData_send_start( );
	}
}

FINSH_FUNCTION_EXPORT( open_com, open_com[1 | 0] );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void app_queenable( u8* instr )
{
	if( instr[0] == '0' )
	{
		app_que_enable	= 0;
		stop_current	= 0;
		//----- init-----------------
		AppQue.write_num		= 0;
		AppQue.read_num			= 0;
		AppQue.sd_enable_flag	= 0;
		AppQue.abnormal_counter = 0;
		AppQue.re_send_flag		= 0;
		AppQue.send_timer		= 0;
		//-------------------------
		rt_kprintf( "\r\n app que disable\r\n" );
	}
	if( instr[0] == '1' )
	{
		app_que_enable = 1;
		//----- init-----------------
		AppQue.write_num		= 0;
		AppQue.read_num			= 0;
		AppQue.sd_enable_flag	= 0;
		AppQue.abnormal_counter = 0;
		AppQue.re_send_flag		= 0;
		AppQue.send_timer		= 0;
		//-------------------------
		rt_kprintf( "\r\n app que enable\r\n" );
	}
}

FINSH_FUNCTION_EXPORT( app_queenable, app_queenable );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void   stop_normal( u8 *instr )
{
	if( instr[0] == '0' )
	{
		stopNormal				= 0;
		AppQue.sd_enable_flag	= 0;
		rt_kprintf( "\r\n  stop_normal disable\r\n" );
	}
	if( instr[0] == '1' )
	{
		stopNormal				= 1;
		AppQue.sd_enable_flag	= 0;
		rt_kprintf( "\r\n stop_normal que enable -only  pos\r\n" );
	}
	if( instr[0] == '2' )
	{
		stopNormal = 2;
		rt_kprintf( "\r\n stop_normal que enable-forbid all\r\n" );
	}
}

FINSH_FUNCTION_EXPORT( stop_normal, stop_normal[1 | 0] );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void  link_ic( char *str )
{
	u8			i = 0;
	u8			reg_str[80];

	uint8_t		ip[4];
	uint16_t	port;

	if( strlen( (const char*)str ) )
	{
		i = str2ipport( (char*)str, ip, &port );
		if( i == 5 )
		{
			memcpy( (char*)SysConf_struct.BD_IC_main_IP, ip, 4 );
			SysConf_struct.BD_IC_TCP_port = port;
			Api_Config_write( config, ID_CONF_SYS, (u8*)&SysConf_struct, sizeof( SysConf_struct ) );
			DataLink_IC_Socket_set( ip, port, 0 );
		}
	}

	Hand_Login		= 0;
	Enable_IClink	= 1;
	TCP2_ready_dial = 1;
	rt_kprintf( "   \r\n    Handle  link  IC  \r\n" );
}

FINSH_FUNCTION_EXPORT( link_ic, link_ic );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void cansd_last( void )
{
	u16 i = 0, datanum = 0;;

	datanum = ( CAN_trans.canid_1_RxWr >> 3 );
	memcpy( CAN_trans.canid_1_Sdbuf, CAN_trans.canid_1_Rxbuf, CAN_trans.canid_1_RxWr );
	CAN_trans.canid_1_SdWr = CAN_trans.canid_1_RxWr;
	for( i = 0; i < datanum; i++ )
	{
		CAN_trans.canid_1_ID_SdBUF[i] = CAN_trans.canid_1_ID_RxBUF[i];
	}

	CAN_trans.canid_1_RxWr		= 0; // clear
	CAN_trans.canid_0705_sdFlag = 1;

	rt_kprintf( "   \r\n    send link last \r\n" );
}

FINSH_FUNCTION_EXPORT( cansd_last, cansd_last );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void stopcurrent( u8 *instr )
{
	if( instr[0] == '0' )
	{
		stop_current = 0; rt_kprintf( "\r\n  enable current \r\n" );
	}

	if( instr[0] == '1' )
	{
		stop_current = 1; rt_kprintf( "\r\n stop current \r\n" );
	}
}

FINSH_FUNCTION_EXPORT( stopcurrent, stopcurrent[1 | 0] );


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void sim_0700( uint8_t cmd )
{
	Recode_Obj.SD_Data_Flag = 1;
	Recode_Obj.CountStep	= 1;
	Recode_Obj.CMD			= cmd;
	if( ( cmd == 0x13 ) || ( cmd == 0x14 ) )
	{
		Recode_Obj.Devide_Flag = 0;
		Stuff_RecoderACK_0700H( Packet_Normal );
	}else
	{
		Recode_Obj.Devide_Flag = 1;
		Stuff_RecoderACK_0700H( Packet_Divide );
	}
	Recode_Obj.SD_Data_Flag = 0;
}


FINSH_FUNCTION_EXPORT( sim_0700, sim_0700 );


void password(u8 value)
{
  rt_kprintf("\r\n password(%d)",value);
  JT808Conf_struct.password_flag=value;     // clear  first flag		
  Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));    

}
FINSH_FUNCTION_EXPORT( password, password ); 

void buzzer_onoff(u8 in) 
{
   
   GPIO_InitTypeDef GPIO_InitStructure;
   
     if(0==in)
    {  
     GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//指定复用引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		//模式为输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//频率为快速
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		//下拉以便节省电能
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
	  
    }
	  
   if(1==in)
    {
	  //-----------------  hardware  0x101    5   Beep -----------------
	/*仅设置结构体中的部分成员：这种情况下，用户应当首先调用函数PPP_SturcInit(..)
	来初始化变量PPP_InitStructure，然后再修改其中需要修改的成员。这样可以保证其他
	成员的值（多为缺省值）被正确填入。
	 */
	
	GPIO_StructInit(&GPIO_InitStructure);
	
	/*配置GPIOA_Pin_5，作为TIM2_Channel1 PWM输出*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//指定复用引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//模式必须为复用！
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//频率为快速
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//上拉与否对PWM产生无影响
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //复用GPIOA_Pin1为TIM2_Ch2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //复用GPIOA_Pin5为TIM2_Ch1, 
	} 


}
FINSH_FUNCTION_EXPORT(buzzer_onoff, buzzer_onoff[1|0]);    

void end_link(void)
{
   DataLink_EndFlag=1;
   rt_kprintf("\r\n 手动挂断\r\n");
   
}
FINSH_FUNCTION_EXPORT(end_link, end_link);  

void mq_erase(void)
{
  u8  i=0;
				for(i=0;i<10;i++)
				{
	                 WatchDog_Feed(); 
					 SST25V_BlockErase_64KByte(((CycleStart_offset+128*i)<<9));   
			         DF_delay_ms(600);   
				     WatchDog_Feed(); 
				}
				
	cycle_write=0;//6959
	cycle_read=0;	//0
	DF_Write_RecordAdd(cycle_write,cycle_read,TYPE_CycleAdd);   	
	rt_kprintf("\r\n earse_MQ   WR=%d  RD=%d \r\n",cycle_write,cycle_read);    		

}
FINSH_FUNCTION_EXPORT(mq_erase,mq_erase);
	
void mq_cmd(void)
{
   Get_GSM_HexData("7E860200180136013000046AEF01010000006640000262548106BC3E400238A9580717CBC09C7E",78,0);
   OutPrint_HEX("盲区围栏", GSM_HEX, GSM_HEX_len); 
}
FINSH_FUNCTION_EXPORT(mq_cmd, mq_cmd);

u32 Get_MQ_true_CuurentTotal_packets(void)
{
   u32  delta_reg=0;
        //获取总包数
	        if(cycle_read<cycle_write)
	        {
	           /* delta_reg=cycle_write-cycle_read;
				// 计算总包数
				if(delta_reg%MQ_PKNUM)
				    CurrentTotal=delta_reg/MQ_PKNUM+1;
				else
					 CurrentTotal=delta_reg/MQ_PKNUM;
			    */
			    if(cycle_write%MQ_PKNUM)
				    CurrentTotal=cycle_write/MQ_PKNUM+1;
				else
					 CurrentTotal=cycle_write/MQ_PKNUM; 

	        }
			else   // write 小于 read
			{	    
				// 计算总包数
			    delta_reg=Max_CycleNum+cycle_write-cycle_read; 
				if(delta_reg%MQ_PKNUM)
				    CurrentTotal=delta_reg/MQ_PKNUM+2;  // 跨界要分2包
				else
					 CurrentTotal=delta_reg/MQ_PKNUM+1; 

			}
   return CurrentTotal;
}

void  sequence_sd(u8 value)
{

  BD_send_Init();
  DF_Write_RecordAdd(BDSD.write,BDSD.read,TYPE_BDsdAdd); 
  
  BDSD.Enable_Working=value;   
  		  
  rt_kprintf("\r\n  消息队列发送模式: %d\r\n",BDSD.Enable_Working);
}
FINSH_FUNCTION_EXPORT(sequence_sd, sequence_sd0:disable1:enable);
	

void mq_true_enable(u8 value)
{
 u32  delta_reg=0;
  switch(value)
  { 
     case 2:
          {
		    mq_erase();
			MQsend_counter=0;
		    MQ_TrueUse.Enable_SD_state=2;
			rt_kprintf("\r\n  MQ_true mode  :enable save !\r\n");
		  }
		 break;
	case 1:  	  
		  {
		    MQ_TrueUse.Enable_SD_state=1; 
			MQ_TrueUse.PacketNum=1;
		     //获取总包数
		    Mq_total_pkg=Get_MQ_true_CuurentTotal_packets();
			rt_kprintf("\r\n  MQ_true mode  :enable send ! write=%d  read=%d  Totalpackets: %d  CurrentTotal=%d\r\n",cycle_write,cycle_read,Mq_total_pkg,CurrentTotal);
			
		  }
	      break;
    case  0:
	default:
		  MQ_TrueUse.Enable_SD_state=0; 
			rt_kprintf("\r\n  MQ_true mode  :enable disable !\r\n"); 
	      break;
  	}		  

}
FINSH_FUNCTION_EXPORT(mq_true_enable, mq_true_enable 0:idle+othervalue 1:send 2:save);
//+++++++++++++++++++++++++++++++++++++++wxg++++++++++鉴权+++++++++++
void jianquan(u8 *str)
{
	memset( JT808Conf_struct.ConfirmCode, 0, sizeof( JT808Conf_struct.ConfirmCode ));
	memcpy( JT808Conf_struct.ConfirmCode, str, strlen(str) );	
	JT808Conf_struct.Regsiter_Status	= 1;
	Api_Config_Recwrite_Large( jt808, 0, (u8*)&JT808Conf_struct, sizeof( JT808Conf_struct ) );
	rt_kprintf( "鉴权码: %s\r\n		   鉴权码长度: %d\r\n", str, strlen( (const char*)str ) );
	//-------- 开始鉴权 ------
	DEV_Login.Operate_enable = 1;
}
FINSH_FUNCTION_EXPORT(jianquan,1);

 
// C.  Module
