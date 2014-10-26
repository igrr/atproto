/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "user_interface.h"
#include "ip_commands_info.h"


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
        dce_emit_extended_result_code(dce, "+CIPRESOLVE=\"domain.name\"", -1, 1);
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

dce_result_t SECTION_ATTR ip_handle_CIPSTAT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CIPSTAT", -1, 1);
        return DCE_OK;
    }
    if (argc > 0)
    {
        DCE_DEBUG("invalid arguments");
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    char line[128];
    int length;
    ip_ctx_t* ip_ctx = (ip_ctx_t*) group_ctx;
    ip_connection_t* connection = ip_ctx->connections;
    for (int index = 0; index < MAX_ESP_CONNECTIONS; ++index, ++connection)
    {
        if (!connection->conn)
        {
            length = sprintf(line, "%d,\"UNUSED\"", index);
        }
        else
        {
            enum espconn_type protocol = connection->conn->type;
            const char* protocol_name;
            if (protocol == ESPCONN_TCP)
                protocol_name = "TCP";
            else
                protocol_name = "UDP";
            
            const char* state_names[] = {
                "NONE",
                "WAIT",
                "LISTEN",
                "CONNECT",
                "WRITE",
                "READ",
                "CLOSE"
            };
            
            length = sprintf(line, "%d,\"%s\",%d,%d", index, protocol_name, (int) connection->rx_buffer_pos, (int) connection->rx_buffer_size);
            
            esp_tcp* desc = connection->conn->proto.tcp;
            length += sprintf(line + length, ",\"%s\",%d,%d.%d.%d.%d,%d",
                                 state_names[connection->conn->state],
                                 desc->local_port,
                                 desc->remote_ip[0], desc->remote_ip[1], desc->remote_ip[2], desc->remote_ip[3],
                                 desc->remote_port
                                 );
        }
        if (index == 0)
            dce_emit_information_response(dce, line, length);
        else
            dce_continue_information_response(dce, line, length);
    }
    
    if (!ip_ctx->udp_server.conn)
    {
        length = sprintf(line, "%d,\"UNUSED\"", UDP_SERVER_INDEX);
    }
    else
    {
        ip_udp_server_t* server = &ip_ctx->udp_server;
        length = sprintf(line, "%d,\"UDP,LISTEN\",%d,%d,%d", UDP_SERVER_INDEX, server->conn->proto.tcp->local_port, (int) server->rx_buffer_pos, (int) server->rx_buffer_size);
    }
    dce_continue_information_response(dce, line, length);
    
    if (!ip_ctx->tcp_server.conn)
    {
        length = sprintf(line, "%d,\"UNUSED\"", TCP_SERVER_INDEX);
    }
    else
    {
        ip_tcp_server_t* server = &ip_ctx->tcp_server;
        length = sprintf(line, "%d,\"TCP,LISTEN\",%d,%d", TCP_SERVER_INDEX, server->conn->proto.tcp->local_port, server->conn->link_cnt);
    }
    dce_continue_information_response(dce, line, length);

    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}

