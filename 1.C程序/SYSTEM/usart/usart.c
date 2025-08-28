#include "sys.h"
#include "usart.h"	  
  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}
void USART1_Send_Byte(unsigned char byte)//串口发送一个字节
{
	USART_SendData(USART1, byte);//通过库函数发送数据
	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
	//等待发送完成。检测 USART_FLAG_TC 是否置1；//见库函数 P359 介绍

}

//发送字符串 函数应用指针 方法
void UART1_Send_Str(unsigned char *s)//发送字符串 函数应用指针 方法
{
//	unsigned char i=0;//定义一个局部变量用来 发送字符串 ++运算

//	while(s[i]!='\0')// 每个字符串结尾 都是以结尾的
//	{
//	USART_SendData(USART1,s[i]);//通过库函数发送数据
//	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
//	//等待发送完成。检测 USART_FLAG_TC 是否置1；//见库函数 P359 介绍

//	i++;//i++一次
	  while(*s!='\0')
		USART1_Send_Byte(*s++);	
			

}
unsigned int ch2o;	 //甲醛上限变量,甲醛浓度变量
u8 bz,ms,sec,UART_dat[10],zhi[2],i;	 //暂存数据切换变量
u8 Res;
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//Res=USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
	 
	 UART_dat[bz]=USART_ReceiveData(USART1);
   ///////////////////////////////////////////////////////////
		switch(bz)
		{
			case 0:
				if(UART_dat[bz]==0xFF)
				{
					bz=1;
				}else
				{
					bz=0; 
				}
			break;

			case 1:
				if(UART_dat[bz]==0x17)
				{
					bz=2;
				}else
				{
					bz=0; 
				}
			break;

			case 2:
				if(UART_dat[bz]==0x04)
				{
					bz=3;  //LED1=0;
				}else
				{
					bz=0;  
				}
			break;

			case 3:
				if(UART_dat[bz]==0x00)
				{
					bz=4; 
				}else
				{
					bz=0; 
				}
			break;

			case 4:
				bz=5;   
			break;

			case 5:
				bz=6; 
			break;

			case 6:
				if(UART_dat[bz]==0x13)
				{
					bz=7;   
				}else
				{
					bz=0; 
				}
			break;

			case 7:
				if(UART_dat[bz]==0x88)
				{
					bz=8;  
				}else
				{
					bz=0; 
				}
			break;

			case 8:  //校验接收的数据  正确则处理接收的数据
				if((UART_dat[bz]+UART_dat[1]+UART_dat[2]+UART_dat[3]+UART_dat[4]+UART_dat[5]+UART_dat[6]+UART_dat[7])%256==0)
				{
					ch2o=UART_dat[4]*256+UART_dat[5];
					ch2o=ch2o*13.392857;				
					bz=0;
				}else
				{
					bz=0;   
				}
			break;

		}
		  		 
  } 
     //溢出-如果发生溢出需要先读SR,再读DR寄存器则可清除不断入中断的问题
     if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    {
         USART_ReceiveData(USART1);
         USART_ClearFlag(USART1,USART_FLAG_ORE);
     }
      USART_ClearFlag(USART1,USART_IT_RXNE); //一定要清除接收中断
}	



