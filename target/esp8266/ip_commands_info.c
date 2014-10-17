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

