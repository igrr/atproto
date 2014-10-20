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
    memset(ip_ctx->connections, 0, sizeof(ip_ctx->connections));
    ip_connection_t* arg = ip_ctx->connections;
    for (int i = 0; i < MAX_ESP_CONNECTIONS; ++i, ++arg)
    {
        arg->ctx = ip_ctx;
        arg->type = UNUSED;
        arg->server_index = -1;
        arg->index = i;
    }
}

int SECTION_ATTR ip_espconn_get(ip_ctx_t* ctx,
                                struct espconn* con, enum espconn_type protocol,
                                size_t rx_buffer_size)
{
    ip_connection_t* pconn = ctx->connections;
    int i;
    for (i = 0; i < MAX_ESP_CONNECTIONS && pconn->type != UNUSED; ++i, ++pconn) {}
    if (i == MAX_ESP_CONNECTIONS)
        return -1;
    
    if (!con)
    {
        pconn->connection = (struct espconn*) malloc(sizeof(struct espconn));
        memset(pconn->connection, 0, sizeof(struct espconn));
    }
    else
    {
        pconn->connection = con;
    }
    pconn->connection->reverse = pconn;
    
    if (protocol == ESPCONN_TCP)
    {
        pconn->connection->proto.tcp = (esp_tcp*) malloc(sizeof(esp_tcp));
    }
    else
    {
        pconn->connection->proto.udp = (esp_udp*) malloc(sizeof(esp_udp));
    }
    pconn->connection->type = protocol;
    pconn->type = CREATED;
    pconn->rx_buffer_size = rx_buffer_size;
    pconn->rx_buffer_pos = 0;
    pconn->rx_buffer = (char*) malloc(pconn->rx_buffer_size);
    
    return i;
}

void  SECTION_ATTR ip_espconn_release(ip_ctx_t* ctx, int index)
{
    ip_connection_t* conn = ctx->connections + index;
    free(conn->connection->proto.tcp);
    free(conn->connection);
    conn->connection = 0;
    free(conn->rx_buffer);
    conn->rx_buffer = 0;
    conn->type = UNUSED;
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

