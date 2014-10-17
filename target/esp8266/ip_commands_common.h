/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef IP_COMMANDS_COMMON_H
#define IP_COMMANDS_COMMON_H

#include "dce_common.h"
#include "espconn.h"
#include "dce_commands.h"

#define MAX_ESP_CONNECTIONS 8

typedef struct ip_ctx_ ip_ctx_t;

typedef struct {
    int connection_index;
    ip_ctx_t* ctx;
} espconn_callback_arg_t;

struct ip_ctx_ {
    dce_t* dce;
    struct espconn connections[MAX_ESP_CONNECTIONS];
    espconn_callback_arg_t callback_args[MAX_ESP_CONNECTIONS];
};


void ip_ctx_init(ip_ctx_t* ip_ctx);
int ip_espconn_get(ip_ctx_t* ctx, enum espconn_type type);
void ip_espconn_release(ip_ctx_t* ctx, int index);
size_t sprintf_ip(char* buf, uint32_t addr);
size_t sprintf_mac(char* buf, uint8_t* mac);


#endif//IP_COMMANDS_COMMON_H
