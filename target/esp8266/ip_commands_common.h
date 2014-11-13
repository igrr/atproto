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

#define MAX_ESP_CONNECTIONS 5
#define DEFAULT_RX_BUFFER_SIZE 2048
#define UDP_SERVER_INDEX (MAX_ESP_CONNECTIONS + 0)
#define TCP_SERVER_INDEX (MAX_ESP_CONNECTIONS + 1)
typedef struct ip_ctx_ ip_ctx_t;

typedef struct {
    int index;
    ip_ctx_t* ctx;
    struct espconn* conn;
    char* rx_buffer;
    size_t rx_buffer_size;
    size_t rx_buffer_pos;
} ip_connection_t;

typedef ip_connection_t ip_udp_server_t;

typedef struct {
    int index;
    ip_ctx_t* ctx;
    struct espconn* conn;
    int clients_count;
    size_t rx_buffer_size;
    int clients[MAX_ESP_CONNECTIONS];
} ip_tcp_server_t;

struct ip_ctx_ {
    dce_t* dce;
    ip_connection_t connections[MAX_ESP_CONNECTIONS];
    ip_udp_server_t udp_server;
    ip_tcp_server_t tcp_server;

};


void ip_ctx_init(ip_ctx_t* ip_ctx);
int ip_espconn_get(ip_ctx_t* ctx,
        struct espconn* con, enum espconn_type protocol, size_t rx_buffer_size);
void ip_espconn_release(ip_ctx_t* ctx, int index);
ip_tcp_server_t* ip_tcp_server_create(ip_ctx_t* ctx);
void ip_tcp_server_release(ip_ctx_t* ctx);
ip_udp_server_t* ip_udp_server_create(ip_ctx_t* ctx, size_t rx_buffer_size);
void ip_udp_server_release(ip_ctx_t* ctx);

size_t sprintf_ip(char* buf, uint32_t addr);
size_t sprintf_mac(char* buf, uint8_t* mac);


#endif//IP_COMMANDS_COMMON_H
