/**
  ******************************************************************************
  * 文件名程: contral.c
  * 作    者: By Sw Young
  * 版    本: V1.0
  * 功    能:
  * 编写日期: 2018.7.6
  ******************************************************************************
  * 说明：
  * 硬件平台：TM4C123G
  *   *****
  * 软件设计说明：
  *   *****
  * Github：
  ******************************************************************************
**/
#include <0.96'OLED/OLED.h>
#include <0.96'OLED/OLED.h>
#include "head.h"
#include "colorful_LED/colorful_LED.h"
#include "Pwm/pwm.h"
#include "Timer/Timer.h"
#include "delay/delay.h"
#include "uart/uart.h"
#include "Beep/Beep.h"
#include "Control.h"

/*
 * 参数初始化
 */
//高度:单位 MM
uint16_t Goal_Distance = 700;//默认定高值800mm
float volatile Real_Distance = 0,Last_Real_Distance = 0;
extern int int_distance;
extern int Distance_Laser;
uint16_t Error_Distance = 0;

extern int16_t int_pitch ,int_roll ,int_yaw;


int16_t RealAttitude_roll;
int16_t RealAttitude_pitch;
int16_t RealAttitude_yaw;

uint16_t err_roll=0,err_pitch=0;

uint16_t Goal_XCoordinate,Goal_YCoordinate;
uint16_t Real_XCoordinate,Real_YCoordinate;
extern volatile uint8_t get_x, get_y;

uint8_t Control_Open = 0;
bool Control_Serial = true;

/*遥控器校准值
 *CH1 1100-1950
 *CH2 1100-1950
 *CH3 1100-1950
 *CH4 1100-1950
 *CH5 1100(自稳)-1520(定高)-1950(降落)
 */
/**
  * 函 数 名:UnlockPixhawk
  * 函数功能: Pixhawk解锁函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void UnlockPixhawk(void)
{
    PwmControl_1(1520);
    PwmControl_2(1520);
    PwmControl_3(1100);
    PwmControl_4(1520);
    PwmControl_5(1100); //自稳模式
    Delay_ms(1000);//延时等待
    PwmControl_3(1100);
    PwmControl_4(1950);//解锁
    Delay_ms(1000);
    Delay_ms(1000);
    Delay_ms(1000);
    PwmControl_4(1520);

    PwmControl_1(1520);
    PwmControl_2(1520);
    PwmControl_3(1100);
    PwmControl_4(1520);

    PwmControl_5(1520); //定高模式

}
/**
  * 函 数 名:LockPixhawk
  * 函数功能: Pixhawk上锁函数函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void LockPixhawk(void)
{
    PwmControl_3(1100);
    PwmControl_4(1100);//上锁
    Delay_ms(1000);
    Delay_ms(1000);
    Delay_ms(1000);
    PwmControl_4(1520);
}
/**
  * 函 数 名:LandMode
  * 函数功能: 飞行器降落模式函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void LandMode(void)
{
    PwmControl_5(1950); //降落模式
    PwmControl_1(1520);
    PwmControl_2(1520);
    PwmControl_3(1100);
    PwmControl_4(1520);
}
/**
  * 函 数 名:Led_Twinkle
  * 函数功能:   LED闪烁函数
  * 输入参数: 颜色、闪烁次数
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void Get_Coordinate(void)
{
    get_x = Real_XCoordinate;
    get_y = Real_YCoordinate;
}
/**
  * 函 数 名:Get_Distance
  * 函数功能: 获取飞行器高度函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void Get_Distance(void)
{
    //Real_Distance = int_distance*10;//转换成mm
    //Real_Distance = GetAverageDistance();
    Real_Distance = Distance_Laser*10;
    if(fabs(Real_Distance-Last_Real_Distance)>500)
        Real_Distance = Last_Real_Distance;
    Last_Real_Distance = Real_Distance;
}
/**
  * 函 数 名:Get_Attitude
  * 函数功能: 获取姿态
  * 输入参数:
  * 返 回 值: 无
  * 说    明:
  *   By Sw Young
  *   2017.7.6
  */
void Get_Attitude(void)
{
    RealAttitude_pitch = int_pitch;
    RealAttitude_roll = int_roll;
    RealAttitude_yaw = int_yaw;
}
/**
  * 函 数 名:AltitudeHold
  * 函数功能: 自稳模式飞行
  * 输入参数:
  * 返 回 值: 无
  * 说    明:
  *   By Sw Young
  *   2017.7.6
  */
void AltitudeHold(void)
{
    //低于目标高度
    if(Goal_Distance-(int)Real_Distance > 100&&Goal_Distance-(int)Real_Distance <300)
    {
       PwmControl_3(1655);
    }
    else if(Goal_Distance-(int)Real_Distance > 300&&Goal_Distance-(int)Real_Distance <500)
    {
       PwmControl_3(1670);
    }
    else if(Goal_Distance-(int)Real_Distance > 500)
    {
       PwmControl_3(1700);
    }
    //高于目标高度
    else if((int)Real_Distance - Goal_Distance > 100&&(int)Real_Distance - Goal_Distance < 300)
    {
       PwmControl_3(1435);
    }
    else if((int)Real_Distance - Goal_Distance > 300&&(int)Real_Distance - Goal_Distance < 500)
    {
       PwmControl_3(1380);
    }
    else if((int)Real_Distance - Goal_Distance > 500)
    {
       PwmControl_3(1350);
    }
    //死区内
    else if((((int)Real_Distance-Goal_Distance<100)&&((int)Real_Distance-Goal_Distance>0))\
           ||((Goal_Distance-(int)Real_Distance<100)&&Goal_Distance-(int)Real_Distance>0))//调节死区 -100 ~ +100
    {
       PwmControl_3(channel_val_MID);
    }
}
/**
  * 函 数 名:OledDisplayInit
  * 函数功能: OLED显示初始化函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void OledDisplayInit(void)
{
    OLED_Clear();
    OLED_ShowString(16,0,"FourAxes2018",16);
    OLED_ShowString(6,3,"AxesState:",16);
    OLED_ShowString(90,3,"OFF",16);
    OLED_ShowString(6,6,"GoalDis:",16);
    OLED_ShowNum(50,6,Goal_Distance,4,16);
    OLED_ShowString(80,88,"MM",16);
}
/**
  * 函 数 名:Display
  * 函数功能: OLED显示函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void Display(void)
{
    OLED_ShowString(6,3,"AxesState:",16);
    if(Control_Open)
        OLED_ShowString(90,3,"ON ",16);
    else
        OLED_ShowString(90,3,"OFF",16);
    OLED_ShowString(6,6,"GoalDis:",16);
    OLED_ShowNum(56,6,Goal_Distance,4,16);
    OLED_ShowString(88,6,"MM",16);
}
/**
  * 函 数 名:AttitudeProtection
  * 函数功能: 姿态保护函数
  * 输入参数:
  * 返 回 值: 无
  * 说    明: 无
  *   By Sw Young
  *   2017.7.6
  */
void AttitudeProtection(void)
{
    err_roll=fabs(RealAttitude_roll);
    err_pitch=fabs(RealAttitude_pitch);
    if(err_pitch>38||err_roll>38)
    {
        Control_Open = false;
        LandMode();
    }
}
