/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */


#ifndef IP_COMMANDS_SOCKET_H
#define IP_COMMANDS_SOCKET_H

#include "ip_commands_common.h"

dce_result_t ip_handle_CIPCREATE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);
dce_result_t ip_handle_CIPCLOSE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);
dce_result_t ip_handle_CIPCONNECT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);
dce_result_t ip_handle_CIPDISCONNECT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);
dce_result_t ip_handle_CIPSENDI(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);
dce_result_t ip_handle_CIPRD(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv);

#endif//IP_COMMANDS_SOCKET_H
