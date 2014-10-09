#include "dce_common.h"
#include "dce_commands.h"
#include "wifi_commands.h"
#include "config_store.h"
#include "user_interface.h"

dce_result_t SECTION_ATTR wifi_handle_CWMODE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CWMODE:(1-3)", -1);
    }
    else if (kind == DCE_READ)
    {
        int mode = wifi_get_opmode();
        arg_t arg = {.type = ARG_TYPE_NUMBER, .value.number = mode};
        dce_emit_extended_result_code_with_args(dce, "CWMODE", -1, &arg, 1);
    }
    else
    {
        if (argc != 1 || argv[0].type != ARG_TYPE_NUMBER || argv[0].value.number > 3)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        ETS_UART_INTR_DISABLE();
        wifi_set_opmode(argv[0].value.number);
        ETS_UART_INTR_ENABLE();
        dce_emit_basic_result_code(dce, DCE_RC_OK);
    }
    return DCE_OK;
}


static const command_desc_t commands[] = {
    {"CWMODE", &wifi_handle_CWMODE, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_wifi_commands(dce_t* dce)
{
    dce_register_command_group(dce, "CW", commands, ncommands, 0);
}
