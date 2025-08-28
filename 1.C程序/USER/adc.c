 #include "adc.h"
 #include "delay.h"
	   
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3	

void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA0 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
  
	//PB11 作为脉冲输出引脚   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;           
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GP2Y_High;

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能



}				  
//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 0, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}
//0.9V表示空气洁净，3.6V表示空气灰尘很多。由于空气中不可能一点灰尘都没有，所以取个正常值0.9V，换成灰尘浓度就是0.053mg/m3
//当达到最大电压值3.6V时，此时的灰尘浓度为0.512mg/m3
//所以我们这个传感器的范围就是0~512ug/m3（这个512只是一个估算大约的值，实际情况中还是有可能超出这值的）
float GetGP2Y(void)
{
    u32 AD_PM;  
	  double pm; 
    GP2Y_Low;//LED=0;
    delay_us(280);
    AD_PM = Get_Adc(ADC_Channel_0);	//PA0
    delay_us(40);
    GP2Y_High;//LED=1;
    delay_us(9680);
    pm = 0.17*AD_PM-0.1; //转换公式
 //   printf("%d\n",pm);
	  return pm;
} 

u16 Get_GP2Y_Average(u8 times)
{
	u32 pm_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		pm_val+=GetGP2Y();
		delay_ms(10);
	}
	return pm_val/times;
} 































