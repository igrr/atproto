#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "uart.h"
#include "dce.h"

#define COMMAND_TASK_PRIORITY 0
#define COMMAND_QUEUE_SIZE    1

uart_t* uart0;
dce_t* dce;

os_event_t command_queue[COMMAND_QUEUE_SIZE];

void ICACHE_FLASH_ATTR uart_echo(char c)
{
	uart0_transmit_char(c);
}

void ICACHE_FLASH_ATTR rx_dce_cb(char c)
{
	dce_handle_input(dce, &c, 1);
}

void ICACHE_FLASH_ATTR target_dce_request_process_command_line(dce_t* dce)
{
    system_os_post(COMMAND_TASK_PRIORITY, 0, 0);
}

void command_task(os_event_t *events)
{
	dce_process_command_line(dce);
}

void ICACHE_FLASH_ATTR target_dce_transmit(const char* data, size_t size)
{
    uart0_transmit(data, size);
}

void ICACHE_FLASH_ATTR target_dce_reset()
{
    system_restart();
}

void ICACHE_FLASH_ATTR target_dce_assert(const char* message)
{
    uart0_transmit(message, os_strlen(message));
    while(true){}	// wait to be reset by the watchdog
}

void ICACHE_FLASH_ATTR user_init(void)
{
	dce = dce_init(256);
    uart0 = uart0_init(115200, &rx_dce_cb);
    uart_disable_debug();
    system_os_task( command_task,
    				COMMAND_TASK_PRIORITY,
    				command_queue,
    				COMMAND_QUEUE_SIZE);
}

