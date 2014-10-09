#include "dce_commands.h"
#include "info_commands.h"
#include "user_interface.h"

dce_result_t SECTION_ATTR dce_handle_GMM(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    dce_emit_information_response(dce, "Chip: ESP8266EX\r\nFirmware: https://github.com/igrr/atproto", -1);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_GMR(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    dce_emit_information_response(dce, VERSION_STRING, -1);
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

static const command_desc_t commands[] = {
    {"GMM", &dce_handle_GMM, DCE_EXEC },
    {"GMR", &dce_handle_GMR, DCE_EXEC },
    {"GSN", &dce_handle_GSN, DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_info_commands(dce_t* dce)
{
    dce_register_command_group(dce, "G", commands, ncommands, 0);
}