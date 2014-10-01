#include "dce.h"
#include "dce_commands.h"
#include "dce_test_commands.h"
#include <string.h>

dce_result_t dce_handle_TESTARGS(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    extended_commands_test_t* ctx = (extended_commands_test_t*) group_ctx;
    if (kind & DCE_EXEC)
    {
        if (argc != 4 ||
            argv[0].type != ARG_TYPE_NUMBER ||
            argv[1].type != ARG_TYPE_STRING ||
            argv[2].type != ARG_TYPE_NUMBER ||
            argv[3].type != ARG_TYPE_STRING)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        ctx->param1 = argv[0].value.number;
        strncpy(ctx->param2, argv[1].value.string, sizeof(ctx->param2));
        ctx->param3 = argv[2].value.number;
        strncpy(ctx->param4, argv[3].value.string, sizeof(ctx->param4));
        dce_emit_basic_result_code(dce, DCE_RC_OK);
        return DCE_OK;
    }
    else
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }

}

static const command_desc_t commands[] = {
        {"TESTARGS", &dce_handle_TESTARGS, DCE_ACTION | DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void dce_register_test_commands(dce_t* dce, extended_commands_test_t* commands_ctx)
{
    dce_register_command_group(dce, "T", commands, ncommands, commands_ctx);
}
