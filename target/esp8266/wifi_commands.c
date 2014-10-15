/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "dce_commands.h"
#include "wifi_commands.h"
#include "config_store.h"
#include "user_interface.h"

#define CONNECTION_MONITORING_INTERVAL_MS 100
#define ONCE 0
#define REPEAT 1

typedef struct {
    dce_t* dce;
    os_timer_t connection_monitor_timer;
    int last_connection_status;
} wifi_ctx_t;

dce_result_t SECTION_ATTR wifi_handle_CWMODE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CWMODE:(1-3)", -1, 1);
    }
    else if (kind == DCE_READ)
    {
        int mode = wifi_get_opmode();
        arg_t arg = {.type = ARG_TYPE_NUMBER, .value.number = mode};
        dce_emit_extended_result_code_with_args(dce, "CWMODE", -1, &arg, 1, 1);
    }
    else
    {
        if (argc != 1 || argv[0].type != ARG_TYPE_NUMBER || argv[0].value.number > 3)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        wifi_set_opmode(argv[0].value.number);
        dce_emit_basic_result_code(dce, DCE_RC_OK);
    }
    return DCE_OK;
}

typedef struct {
    wifi_ctx_t* wifi_ctx;
} wifi_scan_context_t;

static wifi_scan_context_t s_wifi_scan_context;

void SECTION_ATTR wifi_handle_CWLAP_scan_complete(void* result, STATUS status)
{
    dce_t* dce = s_wifi_scan_context.wifi_ctx->dce;
    
    if (status != OK)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return;
    }
    char line[128];
    int i = 0;
    for (struct bss_info* it = (struct bss_info*) result; it; it = STAILQ_NEXT(it, next), ++i)
    {
        int length = os_sprintf(line, "%d,\"%s\",%d", it->authmode, it->ssid, it->rssi);
        if (i == 0)
            dce_emit_information_response(dce, line, length);
        else
            dce_continue_information_response(dce, line, length);
    }
    dce_emit_basic_result_code(dce, DCE_RC_OK);
}

dce_result_t SECTION_ATTR wifi_handle_CWLAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    int status = wifi_station_get_connect_status();
    if (status != STATION_GOT_IP)
    {
        struct station_config conf;
        *conf.ssid = 0;
        *conf.password = 0;
        wifi_station_set_config(&conf);
        wifi_station_disconnect();
    }
    s_wifi_scan_context.wifi_ctx = (wifi_ctx_t*) group_ctx;
    wifi_station_scan(&wifi_handle_CWLAP_scan_complete);
    return DCE_OK;
}

dce_result_t SECTION_ATTR wifi_handle_CWJAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CWJAP:\"ssid\",\"password\"", -1, 1);
    }
    else if (kind == DCE_WRITE)
    {
        struct station_config conf;
        if (argc != 2 ||
            argv[0].type != ARG_TYPE_STRING ||
            argv[1].type != ARG_TYPE_STRING ||
            strlen(argv[0].value.string) >= sizeof(conf.ssid) - 1 ||
            strlen(argv[1].value.string) >= sizeof(conf.password) - 1 )
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_RC_OK;
        }
        
        strcpy(conf.ssid, argv[0].value.string);
        strcpy(conf.password, argv[1].value.string);
        wifi_station_set_config(&conf);
        wifi_station_connect();
        dce_emit_basic_result_code(dce, DCE_RC_OK);
    }
    else
    {
        struct station_config conf;
        wifi_station_get_config(&conf);
        arg_t args[] = {
            {ARG_TYPE_STRING, .value.string = conf.ssid},
            {ARG_TYPE_STRING, .value.string = conf.password}
        };
        dce_emit_extended_result_code_with_args(dce, "CWJAP", -1, args, 2, 1);
    }
    return DCE_OK;
}


dce_result_t SECTION_ATTR wifi_handle_CWQAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    struct station_config conf;
    *conf.ssid = 0;
    *conf.password = 0;
    wifi_station_set_config(&conf);
    wifi_station_disconnect();
    dce_emit_basic_result_code(dce, DCE_RC_OK);
    return DCE_OK;
}


dce_result_t SECTION_ATTR wifi_handle_CWSAP(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != SOFTAP_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CWSAP:\"ssid\",\"password\",(1-13),(1-4)", -1, 1);
    }
    else if (kind == DCE_WRITE)
    {
        struct softap_config conf;
        if (argc != 4 ||
            argv[0].type != ARG_TYPE_STRING ||
            strlen(argv[0].value.string) > sizeof(conf.ssid) - 1 ||
            argv[1].type != ARG_TYPE_STRING ||
            strlen(argv[1].value.string) > sizeof(conf.password) - 1 ||
            argv[2].type != ARG_TYPE_NUMBER ||
            argv[3].type != ARG_TYPE_NUMBER)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_RC_OK;
        }
        wifi_softap_get_config(&conf);
        strcpy(conf.ssid, argv[0].value.string);
        strcpy(conf.password, argv[1].value.string);
        conf.channel = argv[2].value.number;
        conf.authmode = argv[3].value.number;
        
        if (conf.channel > 13 ||
            conf.authmode > AUTH_WPA_WPA2_PSK)
        {
            dce_emit_basic_result_code(dce, DCE_RC_ERROR);
            return DCE_OK;
        }
        wifi_softap_set_config(&conf);
        dce_emit_basic_result_code(dce, DCE_RC_OK);
    }
    else
    {
        struct softap_config conf;
        wifi_softap_get_config(&conf);
        arg_t args[] = {
            {ARG_TYPE_STRING, .value.string = conf.ssid},
            {ARG_TYPE_STRING, .value.string = conf.password},
            {ARG_TYPE_NUMBER, .value.number = conf.channel},
            {ARG_TYPE_NUMBER, .value.number = conf.authmode},
        };
        dce_emit_extended_result_code_with_args(dce, "CWSAP", -1, args, 4, 1);
    }
    return DCE_OK;
}

dce_result_t SECTION_ATTR wifi_handle_CWSTAT(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
    {
        dce_emit_basic_result_code(dce, DCE_RC_ERROR);
        return DCE_RC_OK;
    }
    arg_t arg = {ARG_TYPE_NUMBER, .value.number=wifi_station_get_connect_status()};
    dce_emit_extended_result_code_with_args(dce, "CWSTAT", -1, &arg, 1, 1);
    return DCE_OK;
}

dce_result_t SECTION_ATTR wifi_handle_CWLIF(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    wifi_ctx_t* ctx = (wifi_ctx_t*) group_ctx;
    struct station_info * station = wifi_softap_get_station_info();
    while (station)
    {
        char buf[128];
        uint8_t* ip = (uint8_t*) &(station->ip.addr);
        uint8_t* mac = station->bssid;
        
        size_t len = os_sprintf(buf, "%d.%d.%d.%d,%02x:%02x:%02x:%02x:%02x:%02x",
                                (uint32_t) ip[0], (uint32_t) ip[1], (uint32_t) ip[2], (uint32_t) ip[3],
                                (uint32_t) mac[0], (uint32_t) mac[1],
                                (uint32_t) mac[2], (uint32_t) mac[3],
                                (uint32_t) mac[4], (uint32_t) mac[5]);
        dce_emit_information_response(dce, buf, len);
        station = STAILQ_NEXT(station, next);
    }
    wifi_softap_free_station_info();
    dce_emit_basic_result_code(dce, DCE_RC_OK);
}

void wifi_connection_monitor_cb(void* arg)
{
    wifi_ctx_t* ctx = (wifi_ctx_t*) arg;
    int mode = wifi_get_opmode();
    if (mode != STATION_MODE && mode != STATIONAP_MODE)
        return;
    
    int status = wifi_station_get_connect_status();
    if (status != ctx->last_connection_status)
    {
        ctx->last_connection_status = status;
        arg_t arg = { ARG_TYPE_NUMBER, .value.number=status };
        dce_emit_extended_result_code_with_args(ctx->dce, "CWSTAT", -1, &arg, 1, 0);
    }
}

void wifi_start_connection_monitor(wifi_ctx_t* wifi_ctx)
{
    os_timer_setfn(&wifi_ctx->connection_monitor_timer, (os_timer_func_t*) &wifi_connection_monitor_cb, wifi_ctx);
    os_timer_arm(&wifi_ctx->connection_monitor_timer, CONNECTION_MONITORING_INTERVAL_MS, REPEAT);
}

void wifi_stop_connection_monitor(wifi_ctx_t* wifi_ctx)
{
    os_timer_disarm(&wifi_ctx->connection_monitor_timer);
}

static const command_desc_t commands[] = {
    {"CWMODE", &wifi_handle_CWMODE, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWLAP", &wifi_handle_CWLAP, DCE_ACTION | DCE_EXEC },
    {"CWJAP", &wifi_handle_CWJAP, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWQAP", &wifi_handle_CWQAP, DCE_ACTION | DCE_EXEC },
    {"CWSAP", &wifi_handle_CWSAP, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWSTAT", &wifi_handle_CWSTAT, DCE_PARAM | DCE_READ },
    {"CWLIF", &wifi_handle_CWLIF, DCE_ACTION | DCE_EXEC },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_wifi_commands(dce_t* dce)
{
    static wifi_ctx_t wifi_ctx;
    wifi_ctx.last_connection_status = STATION_IDLE;
    wifi_ctx.dce = dce;
    dce_register_command_group(dce, "CW", commands, ncommands, &wifi_ctx);
    wifi_start_connection_monitor(&wifi_ctx);
}
