/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "dce_commands.h"
#include "config_store.h"
#include "ip_commands.h"
#include "user_interface.h"
#include "espconn.h"

typedef struct {
    
} ip_ctx_t;

void SECTION_ATTR ip_print_interface_ip(dce_t* dce, int interface)
{
    struct ip_info ip;
    wifi_get_ip_info(interface, &ip);
    char buf[32];
    uint8_t* pip = (uint8_t*) &ip.ip.addr;
    int len = os_sprintf(buf, "%d.%d.%d.%d",
                         (uint32_t) pip[0], (uint32_t) pip[1],
                         (uint32_t) pip[2], (uint32_t) pip[3]);
    dce_emit_information_response(dce, buf, len);
}

dce_result_t SECTION_ATTR ip_handle_CIIPSTA(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_ip(dce, STATION_IF);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
}

dce_result_t SECTION_ATTR ip_handle_CIIPAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != SOFTAP_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_ip(dce, SOFTAP_IF);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
}

//DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
static const command_desc_t commands[] = {
    {"CIIPSTA", &ip_handle_CIIPSTA, DCE_ACTION | DCE_EXEC },
    {"CIIPAP", &ip_handle_CIIPAP, DCE_ACTION | DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_ip_commands(dce_t* dce)
{
    static ip_ctx_t ip_ctx;
    dce_register_command_group(dce, "CI", commands, ncommands, &ip_ctx);
}
