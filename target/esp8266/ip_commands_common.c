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
    memset(ip_ctx, 0, sizeof(ip_ctx_t));
    ip_connection_t* arg = ip_ctx->connections;
    for (int i = 0; i < MAX_ESP_CONNECTIONS; ++i, ++arg)
    {
        arg->ctx = ip_ctx;
        arg->index = i;
        arg->conn = 0;
    }
    ip_ctx->tcp_server.ctx = ip_ctx;
    ip_ctx->udp_server.ctx = ip_ctx;
}

int SECTION_ATTR ip_espconn_get(ip_ctx_t* ctx,
                                struct espconn* existing_conn, enum espconn_type protocol,
                                size_t rx_buffer_size)
{
    ip_connection_t* connection = ctx->connections;
    int i;
    for (i = 0; i < MAX_ESP_CONNECTIONS && connection->conn; ++i, ++connection) {}
    if (i == MAX_ESP_CONNECTIONS)
        return -1;
    
    if (!existing_conn)
    {
        connection->conn = (struct espconn*) malloc(sizeof(struct espconn));
        memset(connection->conn, 0, sizeof(struct espconn));
        if (protocol == ESPCONN_TCP)
        {
            connection->conn->proto.tcp = (esp_tcp*) malloc(sizeof(esp_tcp));
            memset(connection->conn->proto.tcp, 0, sizeof(esp_tcp));
        }
        else
        {
            connection->conn->proto.udp = (esp_udp*) malloc(sizeof(esp_udp));
            memset(connection->conn->proto.udp, 0, sizeof(esp_udp));
        }
        connection->conn->type = protocol;
    }
    else
    {
        connection->conn = existing_conn;
    }
    connection->conn->reverse = connection;
    
    connection->rx_buffer_size = rx_buffer_size;
    connection->rx_buffer_pos = 0;
    connection->rx_buffer = (char*) malloc(connection->rx_buffer_size);
    return i;
}

void  SECTION_ATTR ip_espconn_release(ip_ctx_t* ctx, int index)
{
    ip_connection_t* connection = ctx->connections + index;
    free(connection->conn->proto.tcp);
    free(connection->conn);
    connection->conn = 0;
    free(connection->rx_buffer);
    connection->rx_buffer = 0;
}

ip_tcp_server_t* SECTION_ATTR ip_tcp_server_create(ip_ctx_t* ctx)
{
    if (ctx->tcp_server.conn)
        return 0;
    
    ip_tcp_server_t* server = &ctx->tcp_server;
    server->clients_count = 0;
    server->conn = (struct espconn*) malloc(sizeof(struct espconn));
    memset(server->conn, 0, sizeof(struct espconn));
    server->conn->proto.tcp = (esp_tcp*) malloc(sizeof(esp_tcp));
    memset(server->conn->proto.tcp, 0, sizeof(esp_tcp));
    server->conn->type = ESPCONN_TCP;
    server->conn->reverse = server;
    server->index = TCP_SERVER_INDEX;
    return server;
}

void SECTION_ATTR ip_tcp_server_release(ip_ctx_t* ctx)
{
    if (!ctx->tcp_server.conn)
        return;
    
    ip_tcp_server_t* server = &ctx->tcp_server;
    free(server->conn->proto.tcp);
    free(server->conn);
    server->conn = 0;
}

ip_udp_server_t* SECTION_ATTR ip_udp_server_create(ip_ctx_t* ctx, size_t rx_buffer_size)
{
    ip_udp_server_t* server = &ctx->udp_server;
    if (server->conn)
        return 0;
    
    server->conn = (struct espconn*) malloc(sizeof(struct espconn));
    memset(server->conn, 0, sizeof(struct espconn));
    server->conn->proto.udp = (esp_udp*) malloc(sizeof(esp_udp));
    memset(server->conn->proto.tcp, 0, sizeof(esp_tcp));
    server->conn->type = ESPCONN_UDP;
    server->conn->reverse = server;
    server->rx_buffer_size = rx_buffer_size;
    server->rx_buffer_pos = 0;
    server->rx_buffer = (char*) malloc(rx_buffer_size);
    server->index = UDP_SERVER_INDEX;
    return server;

}

void SECTION_ATTR ip_udp_server_release(ip_ctx_t* ctx)
{
    ip_udp_server_t* server = &ctx->udp_server;
    if (!server->conn)
        return;
    
    free(server->conn->proto.udp);
    free(server->conn);
    server->conn = 0;
    free(server->rx_buffer);
    server->rx_buffer = 0;
}

size_t SECTION_ATTR sprintf_ip(char* buf, uint32_t addr)
{
    uint8_t* pip = (uint8_t*) &addr;
    return sprintf(buf, "%d.%d.%d.%d",
                      (uint32_t) pip[0], (uint32_t) pip[1],
                      (uint32_t) pip[2], (uint32_t) pip[3]);
}

size_t SECTION_ATTR sprintf_mac(char* buf, uint8_t* mac)
{
    return sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
                      (uint32_t) mac[0], (uint32_t) mac[1],
                      (uint32_t) mac[2], (uint32_t) mac[3],
                      (uint32_t) mac[4], (uint32_t) mac[5]);
}

