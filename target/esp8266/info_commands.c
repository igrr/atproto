/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_commands.h"
#include "info_commands.h"
#include "user_interface.h"
#include "uart.h"

dce_result_t SECTION_ATTR dce_handle_GMM(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    dce_emit_information_response(dce, "Chip: ESP8266EX\r\nFirmware: https://github.com/igrr/atproto", -1);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_GMR(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    dce_emit_information_response(dce, VERSION_STRING " rev. " REVISION_STRING, -1);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_GSN(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    uint32_t chip_id = system_get_chip_id();
    char buf[10];
    os_sprintf(buf, "%08x", chip_id);
    dce_emit_information_response(dce, buf, -1);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_GMEM(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    char line[12];
    int length = os_sprintf(line, "%d", system_get_free_heap_size());
    dce_emit_information_response(dce, line, length);

    int debug_enabled = uart_get_debug();
    uart_set_debug(1);
    system_print_meminfo();
    uart_set_debug(debug_enabled);
    
    dce_emit_basic_result_code(dce, DCE_RC_OK);
}

static const command_desc_t commands[] = {
    {"GMM", &dce_handle_GMM, DCE_EXEC },
    {"GMR", &dce_handle_GMR, DCE_EXEC },
    {"GSN", &dce_handle_GSN, DCE_EXEC },
    {"GMEM", &dce_handle_GMEM, DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_info_commands(dce_t* dce)
{
    dce_register_command_group(dce, "G", commands, ncommands, 0);
}
