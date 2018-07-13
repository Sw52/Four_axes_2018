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
uint16_t Goal_Distance = 800;//默认定高值800mm
uint16_t Real_Distance = 0;
uint16_t Error_Distance = 0;

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
void LockPixhawk(void)
{
    PwmControl_3(1100);
    PwmControl_4(1100);//上锁
    Delay_ms(1000);
    Delay_ms(1000);
    Delay_ms(1000);
    PwmControl_4(1520);
}
void LandMode(void)
{
    PwmControl_5(1950); //降落模式
    PwmControl_1(1520);
    PwmControl_2(1520);
    PwmControl_3(1100);
    PwmControl_4(1520);
}
void Get_Coordinate(void)
{
    get_x = Real_XCoordinate;
    get_y = Real_YCoordinate;
}
void Get_Distance(void)//舍弃
{
    Real_Distance = GetAverageDistance();
}
void AltitudeHold(void)//舍弃
{
    Real_Distance = GetAverageDistance();
    if(Real_Distance>Goal_Distance)
        Error_Distance = Real_Distance-Goal_Distance;
    else if(Goal_Distance>Real_Distance)
        Error_Distance = Goal_Distance-Real_Distance;
}
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
