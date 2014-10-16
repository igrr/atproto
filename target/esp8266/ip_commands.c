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

void ip_ctx_init(ip_ctx_t* ip_ctx)
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

int ip_espconn_get(ip_ctx_t* ctx, enum espconn_type type)
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
    return i;
}

void ip_espconn_release(ip_ctx_t* ctx, int index)
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

void SECTION_ATTR ip_print_interface_ip(dce_t* dce, int interface, const char* cmdname)
{
    struct ip_info ip;
    wifi_get_ip_info(interface, &ip);
    char buf[32];
    sprintf_ip(buf, ip.ip.addr);
    arg_t arg = { ARG_TYPE_STRING, .value.string=buf };
    dce_emit_extended_result_code_with_args(dce, cmdname, -1, &arg, 1, 1);
}

void SECTION_ATTR ip_print_interface_mac(dce_t* dce, int interface, const char* cmdname)
{
    uint8_t mac[6];
    wifi_get_macaddr(interface, mac);
    char buf[32];
    sprintf_mac(buf, mac);
    arg_t arg = { ARG_TYPE_STRING, .value.string=buf };
    dce_emit_extended_result_code_with_args(dce, cmdname, -1, &arg, 1, 1);
}

dce_result_t SECTION_ATTR ip_handle_CIPSTA(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        DCE_DEBUG("not in sta or sta+ap mode");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_ip(dce, STATION_IF, "CIPSTA");
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != SOFTAP_MODE && mode != STATIONAP_MODE)
    {
        DCE_DEBUG("not in ap or sta+ap mode");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_ip(dce, SOFTAP_IF, "CIPAP");
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPSTAMAC(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        DCE_DEBUG("not in sta or sta+ap mode");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_mac(dce, STATION_IF, "CIPSTAMAC");
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPAPMAC(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != SOFTAP_MODE && mode != STATIONAP_MODE)
    {
        DCE_DEBUG("not in ap or sta+ap mode");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_print_interface_mac(dce, SOFTAP_IF, "CIPAPMAC");
    return DCE_OK;
}

void SECTION_ATTR ip_resolve_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg)
{
    dce_t* dce = ((ip_ctx_t*)callback_arg)->dce;
    if (!ipaddr)
    {
        DCE_DEBUG("name resolution failed");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return;
    }
    
    char buf[32];
    sprintf_ip(buf, ipaddr->addr);
    arg_t arg = { ARG_TYPE_STRING, .value.string=buf };
    dce_emit_extended_result_code_with_args(dce, "CIPRESOLVE", -1, &arg, 1, 1);
}

dce_result_t SECTION_ATTR ip_handle_CIPRESOLVE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_information_response(dce, "+CIPRESOLVE=\"domain.name\"", -1);
        return DCE_OK;
    }
    if (argc != 1 ||
        argv[0].type != ARG_TYPE_STRING)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    
    const char* hostname = argv[0].value.string;
    ip_addr_t addr;
    err_t result = espconn_gethostbyname(group_ctx, hostname, &addr, &ip_resolve_callback);
    if (result == ESPCONN_ARG)
    {
        DCE_DEBUG("invalid host name");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
    }
    else if (result == ESPCONN_OK)
    {
        ip_resolve_callback(hostname, &addr, group_ctx);
    }
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPCREATE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPCREATE=\"TCP|UDP\"[,port]", -1, 1);
        return DCE_OK;
    }
    
    if (!(argc > 0 && argv[0].type == ARG_TYPE_STRING &&
          (argc == 1 ||
           (argc == 2 && (argv[1].type == ARG_TYPE_NUMBER || argv[1].type == ARG_NOT_SPECIFIED))
       )))
    {
        DCE_DEBUG("invalid args");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    enum espconn_type connection_type = ESPCONN_INVALID;
    if (strcmp("TCP", argv[0].value.string) == 0)
        connection_type = ESPCONN_TCP;
    else if (strcmp("UDP", argv[0].value.string) == 0)
        connection_type = ESPCONN_UDP;
    else
    {
        DCE_DEBUG("invalid protocol");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    ip_ctx_t* ip_ctx = (ip_ctx_t*) group_ctx;
    int index = ip_espconn_get(ip_ctx, connection_type);
    if (index < 0) // all connections are in use
    {
        DCE_DEBUG("all connections in use");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    struct espconn* connection = ip_ctx->connections + index;
    
    int port;
    if (argc == 2 && argv[1].type == ARG_TYPE_NUMBER)
        port = argv[1].value.number;
    else
        port = espconn_port();
    if (connection_type == ESPCONN_TCP)
        connection->proto.tcp->local_port = port;
    else
        connection->proto.udp->local_port = port;
    
    arg_t args[] = {{ARG_TYPE_NUMBER, .value.number=index},
                    {ARG_TYPE_NUMBER, .value.number=port}};
    dce_emit_extended_result_code_with_args(dce, "CIPCREATE", -1, args, 2, 1);
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPCLOSE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPCLOSE=<index>", -1, 1);
        return DCE_OK;
    }
    if (argc != 1 || argv[0].type != ARG_TYPE_NUMBER)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_ctx_t* ip_ctx = (ip_ctx_t*) group_ctx;
    int index = argv[0].value.number;
    struct espconn* connection = ip_ctx->connections + index;
    if (connection->type == ESPCONN_INVALID)
    {
        DCE_DEBUG("connection not in use");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    ip_espconn_release(ip_ctx, index);
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_RC_OK;
}

void ip_recv_callback(struct espconn* connection, char *pdata, unsigned short len)
{
    espconn_callback_arg_t* arg = (espconn_callback_arg_t*) connection->reverse;
    arg_t args[] = {
        {ARG_TYPE_NUMBER, .value.number = arg->connection_index},
        {ARG_TYPE_NUMBER, .value.number = len}
    };
    
    dce_emit_extended_result_code_with_args(arg->ctx->dce, "CIPDR", -1, args, 2, 0);
}

void ip_sent_callback(struct espconn* connection)
{
    espconn_callback_arg_t* arg = (espconn_callback_arg_t*) connection->reverse;
    arg_t res = {ARG_TYPE_NUMBER, .value.number = arg->connection_index};
    dce_emit_extended_result_code_with_args(arg->ctx->dce, "CIPSENDI", -1, &res, 1, 0);
}

void ip_tcp_connect_callback(struct espconn* connection)
{
    espconn_callback_arg_t* arg = (espconn_callback_arg_t*) connection->reverse;
    DCE_DEBUG("connect callback");
    arg_t res = {ARG_TYPE_NUMBER, .value.number = arg->connection_index};
    dce_emit_extended_result_code_with_args(arg->ctx->dce, "CIPCONNECT", -1, &res, 1, 1);
}

void ip_tcp_disconnect_callback(struct espconn* connection)
{
    espconn_callback_arg_t* arg = (espconn_callback_arg_t*) connection->reverse;
    arg_t res = {ARG_TYPE_NUMBER, .value.number = arg->connection_index};
    dce_emit_extended_result_code_with_args(arg->ctx->dce, "CIPDISCONNECT", -1, &res, 1, 1);
}

void ip_tcp_reconnect_callback(struct espconn* connection, sint8 err)
{
    espconn_callback_arg_t* arg = (espconn_callback_arg_t*) connection->reverse;
    arg_t res[] = {
        {ARG_TYPE_NUMBER, .value.number = arg->connection_index},
        {ARG_TYPE_NUMBER, .value.number = err}
    };
    dce_emit_extended_result_code_with_args(arg->ctx->dce, "CIPRECONNECT", -1, res, 2, 0);
}

dce_result_t SECTION_ATTR ip_handle_CIPCONNECT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPCONNECT=<index>,\"ip_addr\",<remote_port>", -1, 1);
        return DCE_OK;
    }
    if (argc != 3 ||
        argv[0].type != ARG_TYPE_NUMBER ||
        argv[1].type != ARG_TYPE_STRING ||
        argv[2].type != ARG_TYPE_NUMBER)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    ip_ctx_t* ctx = (ip_ctx_t*) group_ctx;
    int index = argv[0].value.number;
    if (index >= MAX_ESP_CONNECTIONS ||
        ctx->connections[index].type == ESPCONN_INVALID)
    {
        DCE_DEBUG("invalid connection index");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    struct espconn* connection = ctx->connections + index;
    
    uint8_t* remote_ip;
    if (connection->type == ESPCONN_TCP)
        remote_ip = connection->proto.tcp->remote_ip;
    else
        remote_ip = connection->proto.udp->remote_ip;
    if (dce_parse_ip(argv[1].value.string, remote_ip) != 0)
    {
        DCE_DEBUG("invalid remote IP address");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }

    if (connection->type == ESPCONN_TCP)
        connection->proto.tcp->remote_port = argv[2].value.number;
    else
        connection->proto.udp->remote_port = argv[2].value.number;
    
    espconn_regist_recvcb(connection, (espconn_recv_callback) &ip_recv_callback);
    espconn_regist_sentcb(connection, (espconn_sent_callback) &ip_sent_callback);
    if (connection->type == ESPCONN_TCP)
    {
        espconn_regist_connectcb(connection, (espconn_connect_callback) &ip_tcp_connect_callback);
        espconn_regist_reconcb(connection, (espconn_reconnect_callback) &ip_tcp_reconnect_callback);
        espconn_regist_disconcb(connection,  (espconn_connect_callback) &ip_tcp_disconnect_callback);
    }
    espconn_connect(connection);
    
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPDISCONNECT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPDISCONNECT=<index>", -1, 1);
        return DCE_OK;
    }
    if (argc != 1 ||
        argv[0].type != ARG_TYPE_NUMBER)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    ip_ctx_t* ctx = (ip_ctx_t*) group_ctx;
    int index = argv[0].value.number;
    if (index >= MAX_ESP_CONNECTIONS ||
        ctx->connections[index].type == ESPCONN_INVALID)
    {
        DCE_DEBUG("invalid connection index");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    struct espconn* connection = ctx->connections + index;
    espconn_disconnect(connection);
    return DCE_OK;
}

dce_result_t SECTION_ATTR ip_handle_CIPSENDI(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPSENDI=<index>,\"data_to_send\"", -1, 1);
        return DCE_OK;
    }
    if (argc != 2 ||
        argv[0].type != ARG_TYPE_NUMBER ||
        argv[1].type != ARG_TYPE_STRING)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    ip_ctx_t* ctx = (ip_ctx_t*) group_ctx;
    int index = argv[0].value.number;
    if (index >= MAX_ESP_CONNECTIONS ||
        ctx->connections[index].type == ESPCONN_INVALID)
    {
        DCE_DEBUG("invalid connection index");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_OK;
    }
    struct espconn* connection = ctx->connections + index;
    espconn_sent(connection, (uint8_t*) argv[1].value.string, strlen(argv[1].value.string));
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}


static const command_desc_t commands[] = {
    { "CIPSTA",         &ip_handle_CIPSTA,          DCE_PARAM | DCE_READ },
    { "CIPAP",          &ip_handle_CIPAP,           DCE_PARAM | DCE_READ },
    { "CIPSTAMAC",      &ip_handle_CIPSTAMAC,       DCE_PARAM | DCE_READ },
    { "CIPAPMAC",       &ip_handle_CIPAPMAC,        DCE_PARAM | DCE_READ },
    { "CIPRESOLVE",     &ip_handle_CIPRESOLVE,      DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCREATE",      &ip_handle_CIPCREATE,       DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCLOSE",       &ip_handle_CIPCLOSE,        DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPCONNECT",     &ip_handle_CIPCONNECT,      DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPDISCONNECT",  &ip_handle_CIPDISCONNECT,   DCE_ACTION | DCE_EXEC | DCE_TEST },
    { "CIPSENDI",       &ip_handle_CIPSENDI,        DCE_ACTION | DCE_EXEC | DCE_TEST },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_ip_commands(dce_t* dce)
{
    static ip_ctx_t ip_ctx;
    ip_ctx_init(&ip_ctx);
    ip_ctx.dce = dce;
    dce_register_command_group(dce, "CI", commands, ncommands, &ip_ctx);
}
