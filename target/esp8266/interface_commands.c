/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce.h"
#include "dce_commands.h"
#include "dce_utils.h"
#include "dce_private.h"
#include "interface_commands.h"
#include "config_store.h"


dce_result_t SECTION_ATTR dce_handle_IDBG(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind & DCE_READ)
    {
        arg_t result = {.type = ARG_TYPE_NUMBER, .value.number = uart_get_debug()};
        dce_emit_extended_result_code_with_args(dce, "IDBG", -1, &result, 1, 1);
    }
    else if (kind & DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+IDBG:(0,1)", -1, 1);
    }
    else
    {
        if (argc != 1 || argv[0].type != ARG_TYPE_NUMBER || argv[0].value.number > 1)
        {
            DCE_DEBUG("invalid argument");
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        uart_set_debug(argv[0].value.number);
        dce_emit_basic_result_code(dce, DCE_RC_OK);
    }
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_IPR(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    static const int valid_baudrates[] = {9600, 19200, 38400, 57600, 74880, 115200, 230400, 460800, 921600};
    static const int valid_baudrates_count = sizeof(valid_baudrates)/sizeof(int);
    uart_t* uart = (uart_t*) group_ctx;

    if (kind & DCE_READ)
    {
        arg_t result = {ARG_TYPE_NUMBER, .value.number=uart0_get_baudrate(uart)};
        dce_emit_extended_result_code_with_args(dce, "IPR", -1, &result, 1, 1);
    }
    else if (kind & DCE_TEST)
    {
        char buf[128];
        size_t bufsize = sizeof(buf);
        char * pbuf = buf;
        size_t outsize;
        dce_strcpy("+IPR:(0),(", buf, bufsize, &outsize);
        pbuf += outsize;
        bufsize -= outsize;
        for (size_t i = 0; i < valid_baudrates_count; ++i)
        {
            dce_itoa(valid_baudrates[i], pbuf, bufsize, &outsize);
            pbuf += outsize;
            bufsize -= outsize;
            if (i != valid_baudrates_count - 1)
            {
                *pbuf = ',';
                ++pbuf;
                --bufsize;
            }
        }
        *pbuf = ')';
        ++pbuf;
        --bufsize;
        
        dce_emit_extended_result_code(dce, buf, sizeof(buf) - bufsize, 1);
    }
    else
    {
        if (argc != 1 || argv->type != ARG_TYPE_NUMBER)
        {
            DCE_DEBUG("invalid arguments");
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        int requested_rate = argv->value.number;
        for (size_t i = 0; i < valid_baudrates_count; ++i)
        {
            if (requested_rate == valid_baudrates[i])
            {
                config_t* config = config_get();
                config->baud_rate = requested_rate;
                config_save();
                
                dce_emit_basic_result_code(dce, DCE_RC_OK);
                uart0_wait_for_transmit(uart);
                
                uart0_set_baudrate(uart, requested_rate);
                return DCE_OK;
            }
        }
        DCE_DEBUG("unsupported baud rate");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    }
    return DCE_OK;
}

static const command_desc_t commands[] = {
    {"IPR", &dce_handle_IPR, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"IDBG", &dce_handle_IDBG, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void ICACHE_FLASH_ATTR dce_register_interface_commands(dce_t* dce, uart_t* uart)
{
    dce_register_command_group(dce, "I", commands, ncommands, uart);
}
