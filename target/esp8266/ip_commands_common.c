/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "user_interface.h"
#include "ip_commands_common.h"
#include "dce_target.h"

void SECTION_ATTR ip_ctx_init(ip_ctx_t* ip_ctx)
{
    memset(ip_ctx->connections, 0, sizeof(struct espconn) * MAX_ESP_CONNECTIONS);
    espconn_callback_arg_t* arg = ip_ctx->callback_args;
    for (int i = 0; i < MAX_ESP_CONNECTIONS; ++i, ++arg)
    {
        arg->connection_index = i;
        arg->ctx = ip_ctx;
        ip_ctx->connections[i].reverse = arg;
    }
}

int SECTION_ATTR ip_espconn_get(ip_ctx_t* ctx, enum espconn_type type, size_t rx_buffer_size)
{
    struct espconn* pconn = ctx->connections;
    int i;
    for (i = 0; i < MAX_ESP_CONNECTIONS && pconn->type != ESPCONN_INVALID; ++i, ++pconn) {}
    if (i == MAX_ESP_CONNECTIONS)
        return -1;
    
    pconn->type = type;
    if (type == ESPCONN_TCP)
    {
        pconn->proto.tcp = (esp_tcp*) malloc(sizeof(esp_tcp));
    }
    else
    {
        pconn->proto.udp = (esp_udp*) malloc(sizeof(esp_udp));
    }
    
    espconn_callback_arg_t* callback_context = ctx->callback_args + i;
    if (callback_context->rx_buffer_size != rx_buffer_size)
    {
        free(callback_context->rx_buffer);
        callback_context->rx_buffer = (char*) malloc(rx_buffer_size);
        callback_context->rx_buffer_size = rx_buffer_size;
    }
    if (!callback_context->rx_buffer)
        DCE_FAIL("failed to allocate rx buffer");
    callback_context->rx_buffer_pos = 0;
    return i;
}

void  SECTION_ATTR ip_espconn_release(ip_ctx_t* ctx, int index)
{
    free(ctx->connections[index].proto.tcp);
    ctx->connections[index].type = ESPCONN_INVALID;
}

size_t SECTION_ATTR sprintf_ip(char* buf, uint32_t addr)
{
    uint8_t* pip = (uint8_t*) &addr;
    return os_sprintf(buf, "%d.%d.%d.%d",
                      (uint32_t) pip[0], (uint32_t) pip[1],
                      (uint32_t) pip[2], (uint32_t) pip[3]);
}

size_t SECTION_ATTR sprintf_mac(char* buf, uint8_t* mac)
{
    return os_sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
                      (uint32_t) mac[0], (uint32_t) mac[1],
                      (uint32_t) mac[2], (uint32_t) mac[3],
                      (uint32_t) mac[4], (uint32_t) mac[5]);
}

