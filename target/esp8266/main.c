/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "uart.h"
#include "dce.h"
#include "dce_commands.h"
#include "interface_commands.h"
#include "ip_commands.h"
#include "config_store.h"
#include "info_commands.h"
#include "wifi_commands.h"

#define COMMAND_TASK_PRIORITY 0
#define COMMAND_QUEUE_SIZE    1
#define COMMAND_LINE_LENGTH   256

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

void ICACHE_FLASH_ATTR command_task(os_event_t *events)
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

void ICACHE_FLASH_ATTR target_dce_init_factory_defaults()
{
    config_init_default();
}

void ICACHE_FLASH_ATTR target_dce_assert(const char* message)
{
    uart0_transmit(uart0, "\r\n########\r\n", 12);
    uart0_transmit(uart0, message, strlen(message));
    uart0_transmit(uart0, "\r\n########\r\n", 12);
    uart0_wait_for_transmit(uart0);
    system_restart();
}

void ICACHE_FLASH_ATTR init_done()
{
    arg_t args[] = {
        { ARG_TYPE_STRING, .value.string = "atproto" },
        { ARG_TYPE_STRING, .value.string = VERSION_STRING },
        { ARG_TYPE_STRING, .value.string = REVISION_STRING }
    };
    dce_emit_extended_result_code_with_args(dce, "IREADY", -1, args, 3, 0);
}

void ICACHE_FLASH_ATTR user_init(void)
{
    config = config_init();
    dce = dce_init(COMMAND_LINE_LENGTH);
    uart0 = uart0_init(config->baud_rate, &rx_dce_cb);
    uart_set_debug(0);
    dce_register_ip_commands(dce);
    dce_register_wifi_commands(dce);
    dce_register_interface_commands(dce, uart0);
    dce_register_info_commands(dce);
    system_os_task( command_task,
                    COMMAND_TASK_PRIORITY,
                    command_queue,
                    COMMAND_QUEUE_SIZE);
    system_init_done_cb(&init_done);
}

