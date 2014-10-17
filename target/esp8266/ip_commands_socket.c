/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "user_interface.h"
#include "ip_commands_socket.h"

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
