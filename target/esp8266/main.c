#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "uart.h"
#include "dce.h"
#include "interface_commands.h"
#include "config_store.h"
#include "info_commands.h"

#define COMMAND_TASK_PRIORITY 0
#define COMMAND_QUEUE_SIZE    1

static uart_t* uart0;
static dce_t* dce;
static config_t* config;

os_event_t command_queue[COMMAND_QUEUE_SIZE];

void ICACHE_FLASH_ATTR uart_echo(char c)
{
    uart0_transmit_char(uart0, c);
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
    uart0_transmit(uart0, data, size);
}

void ICACHE_FLASH_ATTR target_dce_reset()
{
    system_restart();
}

void ICACHE_FLASH_ATTR target_dce_assert(const char* message)
{
    uart0_transmit(uart0, "\r\n########\r\n", 12);
    uart0_transmit(uart0, message, os_strlen(message));
    uart0_transmit(uart0, "\r\n########\r\n", 12);
    uart0_wait_for_transmit(uart0);
    system_restart();
}

void ICACHE_FLASH_ATTR config_init(void)
{
    config = config_get();
    if (config->magic != CONFIG_MAGIC || config->version != CONFIG_VERSION)
    {
        // initialize default configuration
        os_printf("\r\nSetting default configuration.\r\n");
        config->magic = CONFIG_MAGIC;
        config->version = CONFIG_VERSION;
        config->baud_rate = 9600;
        config_save();
    }
}

void ICACHE_FLASH_ATTR user_init(void)
{
    config_init();
    dce = dce_init(256);
    uart0 = uart0_init(config->baud_rate, &rx_dce_cb);
    dce_register_interface_commands(dce, uart0);
    dce_register_info_commands(dce);
    uart_set_debug(0);
    system_os_task( command_task,
                    COMMAND_TASK_PRIORITY,
                    command_queue,
                    COMMAND_QUEUE_SIZE);
}

