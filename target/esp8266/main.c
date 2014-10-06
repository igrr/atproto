#include "driver/uart.h"

void user_init(void)
{
	uart_init(BIT_RATE_9600, BIT_RATE_9600);
}

void at_recvTask(void)
{
	uint8_t temp[2]= {0, 0};
	temp[0] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
	uart0_sendStr(temp);
}
