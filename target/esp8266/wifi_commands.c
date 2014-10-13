#include "dce_common.h"
#include "dce_commands.h"
#include "wifi_commands.h"
#include "config_store.h"
#include "user_interface.h"

dce_result_t SECTION_ATTR wifi_handle_CWMODE(dce_t* dce, void* group_ctx, int kind, size_t argc, arg_t* argv)
{
    if (kind == DCE_TEST)
    {
        dce_emit_extended_result_code(dce, "+CWMODE:(1-3)", -1);
    }
    else if (kind == DCE_READ)
    {
        int mode = wifi_get_opmode();
        arg_t arg = {.type = ARG_TYPE_NUMBER, .value.number = mode};
        dce_emit_extended_result_code_with_args(dce, "CWMODE", -1, &arg, 1);
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
    dce_t* dce;
    void*  group_ctx;
} wifi_scan_context_t;

static wifi_scan_context_t s_wifi_scan_context;

void SECTION_ATTR wifi_handle_CWLAP_scan_complete(void* result, STATUS status)
{
    dce_t* dce = s_wifi_scan_context.dce;
    
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
    s_wifi_scan_context.dce = dce;
    s_wifi_scan_context.group_ctx = group_ctx;
    
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
        dce_emit_extended_result_code(dce, "+CWJAP:\"ssid\",\"password\"", -1);
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
        dce_emit_extended_result_code_with_args(dce, "CWJAP", -1, args, 2);
    }
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
        dce_emit_extended_result_code(dce, "+CWSAP:\"ssid\",\"password\",(1-13),(1-4)", -1);
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
        dce_emit_extended_result_code_with_args(dce, "CWSAP", -1, args, 4);
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
    dce_emit_extended_result_code_with_args(dce, "CWSTAT", -1, &arg, 1);
    return DCE_OK;
}


static const command_desc_t commands[] = {
    {"CWMODE", &wifi_handle_CWMODE, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWLAP", &wifi_handle_CWLAP, DCE_ACTION | DCE_EXEC },
    {"CWJAP", &wifi_handle_CWJAP, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWSAP", &wifi_handle_CWSAP, DCE_PARAM | DCE_READ | DCE_WRITE | DCE_TEST },
    {"CWSTAT", &wifi_handle_CWSTAT, DCE_PARAM | DCE_READ },
};

static const int ncommands = sizeof(commands) / sizeof(command_desc_t);

void SECTION_ATTR dce_register_wifi_commands(dce_t* dce)
{
    dce_register_command_group(dce, "CW", commands, ncommands, 0);
}
