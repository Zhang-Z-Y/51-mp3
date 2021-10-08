#ifndef __usart_h__
#define __usart_h__

void UsartInit();
void UARTSendString(char *s);
void UARTSendNum(unsigned long dat);

#endif