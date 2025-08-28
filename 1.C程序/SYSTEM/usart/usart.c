#include "sys.h"
#include "usart.h"	  
  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}
void USART1_Send_Byte(unsigned char byte)//���ڷ���һ���ֽ�
{
	USART_SendData(USART1, byte);//ͨ���⺯����������
	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
	//�ȴ�������ɡ���� USART_FLAG_TC �Ƿ���1��//���⺯�� P359 ����

}

//�����ַ��� ����Ӧ��ָ�� ����
void UART1_Send_Str(unsigned char *s)//�����ַ��� ����Ӧ��ָ�� ����
{
//	unsigned char i=0;//����һ���ֲ��������� �����ַ��� ++����

//	while(s[i]!='\0')// ÿ���ַ�����β �����Խ�β��
//	{
//	USART_SendData(USART1,s[i]);//ͨ���⺯����������
//	while( USART_GetFlagStatus(USART1,USART_FLAG_TC)!= SET);
//	//�ȴ�������ɡ���� USART_FLAG_TC �Ƿ���1��//���⺯�� P359 ����

//	i++;//i++һ��
	  while(*s!='\0')
		USART1_Send_Byte(*s++);	
			

}
unsigned int ch2o;	 //��ȩ���ޱ���,��ȩŨ�ȱ���
u8 bz,ms,sec,UART_dat[10],zhi[2],i;	 //�ݴ������л�����
u8 Res;
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		//Res=USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
	 
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

			case 8:  //У����յ�����  ��ȷ������յ�����
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
     //���-������������Ҫ�ȶ�SR,�ٶ�DR�Ĵ����������������жϵ�����
     if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    {
         USART_ReceiveData(USART1);
         USART_ClearFlag(USART1,USART_FLAG_ORE);
     }
      USART_ClearFlag(USART1,USART_IT_RXNE); //һ��Ҫ��������ж�
}	



