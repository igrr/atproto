/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce.h"
#include "dce_commands.h"
#include "dce_test_commands.h"
#include "dce_utils.h"
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

dce_result_t dce_handle_TESTPARAM1(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    extended_commands_test_t* ctx = (extended_commands_test_t*) group_ctx;
    if (kind & DCE_READ)
    {
        arg_t result = {ARG_TYPE_NUMBER, .value.number=ctx->param1};
        dce_emit_extended_result_code_with_args(dce, "TESTPARAM1", -1, &result, 1, 1);
    }
    else if (kind & DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+TESTPARAM1:(1-100)", -1, 1);
    }
    else
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    }
    return DCE_OK;
}

dce_result_t dce_handle_TESTPARAM3(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    extended_commands_test_t* ctx = (extended_commands_test_t*) group_ctx;
    if (kind & DCE_READ)
    {
        arg_t result = {ARG_TYPE_NUMBER, .value.number=ctx->param3};
        dce_emit_extended_result_code_with_args(dce, "TESTPARAM3", -1, &result, 1, 1);
    }
    else if (kind & DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+TESTPARAM3:(0-127)", -1, 1);
    }
    else if (kind & DCE_WRITE)
    {
        if (argc != 1 || argv[0].type != ARG_TYPE_NUMBER)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        }
        else
        {
            ctx->param3 = argv[0].value.number;
            dce_emit_basic_result_code(dce, DCE_RC_OK);
        }
    }
    return DCE_OK;
}


static const command_desc_t commands[] = {
        {"TESTARGS",    &dce_handle_TESTARGS,   DCE_ACTION | DCE_EXEC},
        {"TESTPARAM1",  &dce_handle_TESTPARAM1, DCE_PARAM | DCE_TEST | DCE_READ},
        {"TESTPARAM3",  &dce_handle_TESTPARAM3, DCE_PARAM | DCE_TEST | DCE_READ | DCE_WRITE},
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void dce_register_test_commands(dce_t* dce, extended_commands_test_t* commands_ctx)
{
    dce_register_command_group(dce, "T", commands, ncommands, commands_ctx);
}
