#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "uart.h"
#include "dce.h"

uart_t* uart0;
dce_t* dce;

void ICACHE_FLASH_ATTR uart_echo(char c)
{
	uart0_transmit_char(c);
}

void ICACHE_FLASH_ATTR rx_dce_cb(char c)
{
	dce_handle_input(dce, &c, 1);
}

void ICACHE_FLASH_ATTR user_dce_transmit(const char* data, size_t size)
{
    uart0_transmit(data, size);
}

void ICACHE_FLASH_ATTR user_dce_reset()
{
    software_reset();
}

void ICACHE_FLASH_ATTR user_dce_assert(const char* message)
{
    uart0_transmit(message, os_strlen(message));
    user_dce_reset();
}


void ICACHE_FLASH_ATTR user_init(void)
{
	dce = dce_init(256);
    uart0 = uart0_init(115200, &rx_dce_cb);
    uart_disable_debug();
}

