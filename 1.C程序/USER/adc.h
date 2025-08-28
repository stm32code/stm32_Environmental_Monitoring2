#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

#define GP2Y_High GPIO_SetBits(GPIOB,GPIO_Pin_11);
#define GP2Y_Low  GPIO_ResetBits(GPIOB,GPIO_Pin_11);

float GetGP2Y(void);
void Adc_Init(void);
u16  Get_Adc(u8 ch); 
u16 Get_GP2Y_Average(u8 times);
 
#endif 
