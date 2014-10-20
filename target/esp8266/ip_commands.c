/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "dce_commands.h"
#include "dce_utils.h"
#include "config_store.h"
#include "ip_commands.h"
#include "user_interface.h"
#include "espconn.h"
#include "ip_commands_common.h"
#include "ip_commands_info.h"
#include "ip_commands_socket.h"



static const command_desc_t commands[] = {
    { "CIPSTA",         &ip_handle_CIPSTA,          DCE_PARAM | DCE_READ },
    { "CIPAP",          &ip_handle_CIPAP,           DCE_PARAM | DCE_READ },
    { "CIPSTAMAC",      &ip_handle_CIPSTAMAC,       DCE_PARAM | DCE_READ },
    { "CIPAPMAC",       &ip_handle_CIPAPMAC,        DCE_PARAM | DCE_READ },
    { "CIPSTAT",        &ip_handle_CIPSTAT,         DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPRESOLVE",     &ip_handle_CIPRESOLVE,      DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCREATE",      &ip_handle_CIPCREATE,       DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCLOSE",       &ip_handle_CIPCLOSE,        DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCONNECT",     &ip_handle_CIPCONNECT,      DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPDISCONNECT",  &ip_handle_CIPDISCONNECT,   DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPSENDI",       &ip_handle_CIPSENDI,        DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPRD",          &ip_handle_CIPRD,           DCE_ACTION | DCE_EXEC | DCE_TEST },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_ip_commands(dce_t* dce)
{
    static ip_ctx_t ip_ctx;
    ip_ctx_init(&ip_ctx);
    ip_ctx.dce = dce;
    dce_register_command_group(dce, "CI", commands, ncommands, &ip_ctx);
}
