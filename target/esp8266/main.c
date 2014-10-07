#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"

#include "uart.h"

uart_t* uart0;

void uart_echo(char c)
{
	uart0_transmit(&c, 1);
}

void user_init(void)
{
    uart0 = uart0_init(9600, &uart_echo);
}

