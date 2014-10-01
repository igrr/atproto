#include "dce_commands.h"
#include "dce_info_commands.h"

dce_result_t dce_handle_GMI(dce_t* dce, int kind, int argc, arg_t* argv)
{
    dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    return DCE_OK;
}

dce_result_t dce_handle_GMM(dce_t* dce, int kind, int argc, arg_t* argv)
{
    dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    return DCE_OK;
}

dce_result_t dce_handle_GMR(dce_t* dce, int kind, int argc, arg_t* argv)
{
    dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    return DCE_OK;
}

dce_result_t dce_handle_GSN(dce_t* dce, int kind, int argc, arg_t* argv)
{
    dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    return DCE_OK;
}

static const command_desc_t commands[] = {
    {"GMI", &dce_handle_GMI, DCE_EXEC },
    {"GMM", &dce_handle_GMM, DCE_EXEC },
    {"GMR", &dce_handle_GMR, DCE_EXEC },
    {"GSN", &dce_handle_GSN, DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void dce_register_info_comands(dce_t* dce)
{
    dce_register_command_group(dce, "G", commands, ncommands);
}
