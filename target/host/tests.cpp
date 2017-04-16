/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <sstream>
std::string g_tx_data;
bool g_reset = false;
int g_factory_value = 0;

extern "C" {
#include "dce.h"
#include "dce_test_commands.h"
#include "dce_target.h"
#include "dce_private.h"
    
void target_dce_transmit(const char* data, size_t size)
{
    g_tx_data += std::string(data, size);
}

void target_dce_reset()
{
    g_reset = true;
}
    
void target_dce_assert(const char* msg)
{
    throw std::runtime_error(msg);
}
    
void target_dce_request_process_command_line(dce_t* dce)
{
    dce_process_command_line(dce);
}
    
void target_dce_init_factory_defaults()
{
    g_factory_value = 0;
}

}

#define DCE_HANDLE_INPUT_STR(dce, str) dce_handle_input(dce, str, sizeof(str) - 1)

TEST_CASE("set echo s-parameter", "[dce]")
{
    dce_t* dce = dce_init(1024);
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK );
    REQUIRE( g_tx_data == "ATE0\r\r\nOK\r\n" );
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATE1\r") == DCE_OK );
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT\r") == DCE_OK );
    REQUIRE( g_tx_data == "\r\nOK\r\nAT\r\r\nOK\r\n");
    dce_uninit(dce);
}

TEST_CASE("set format s-parameter", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATV0\r") == DCE_OK );
    REQUIRE( g_tx_data == "ATV0\r0\r");
    g_tx_data.clear();
    args.param3 = 123;
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM3?\r") == DCE_OK );
    REQUIRE( g_tx_data == "AT+TESTPARAM3?\r+TESTPARAM3:123\r" );
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM3=?\r") == DCE_OK );
    REQUIRE( g_tx_data == "AT+TESTPARAM3=?\r+TESTPARAM3:(0-127)\r" );
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATV1\r") == DCE_OK );
    REQUIRE( g_tx_data == "ATV1\r\r\nOK\r\n");
    dce_uninit(dce);
}

TEST_CASE("reset parameters", "[dce]")
{
    dce_t* dce = dce_init(1024);
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATV0\r") == DCE_OK );
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT&F0\r") == DCE_OK );
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT\r") == DCE_OK );
    REQUIRE( g_tx_data == "AT\r\r\nOK\r\n");    // back to V1
    g_factory_value = 42;
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT&F1\r") == DCE_OK );
    REQUIRE( g_factory_value == 0 );
    dce_uninit(dce);
}

TEST_CASE("parse extended format arguments", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK);
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTARGS=1,\"abcdef\",42,\"a b c\"\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\nOK\r\n");
    REQUIRE(args.param1 == 1);
    REQUIRE(std::string(args.param2) == "abcdef");
    REQUIRE(args.param3 == 42);
    REQUIRE(std::string(args.param4) == "a b c");
    dce_uninit(dce);
}

TEST_CASE("argument count overflow handling", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK);
    g_tx_data.clear();
    std::stringstream ss;
    ss << "AT+TESTARGS=";
    for (int i = 0; i < DCE_MAX_ARGS + 1; ++i)
    {
        ss << i + 1;
        if (i < DCE_MAX_ARGS)
            ss << ',';
    }
    ss << '\r';
    std::string str = ss.str();
    g_tx_data.clear();
    REQUIRE(dce_handle_input(dce, str.c_str(), str.size()) == DCE_OK);
    REQUIRE(g_tx_data == "\r\nERROR\r\n");
    dce_uninit(dce);
}


TEST_CASE("rx overflow handling", "[dce]")
{
    dce_t* dce = dce_init(10);
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE3456789abcdef") == DCE_OK);
    REQUIRE(g_tx_data == "ATE3456789\r\nERROR\r\nabcdef");
    dce_uninit(dce);
}

TEST_CASE("command line editing", "[dce]")
{
    dce_t* dce = dce_init(1024);
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE1\b\b\b\b\b" "ATS5=\b?\r") == DCE_OK);
    REQUIRE(g_tx_data == "ATE1\b\b\b\b\bATS5=\b?\r\r\n008\r\n\r\nOK\r\n");
    dce_uninit(dce);
}

TEST_CASE("read-only parameter", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK);
    g_tx_data.clear();
    
    args.param1 = 10;
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM1?\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\n+TESTPARAM1:10\r\n");
    g_tx_data.clear();
    
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM1=?\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\n+TESTPARAM1:(1-100)\r\n");
    g_tx_data.clear();
    
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM1=20\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\nERROR\r\n");
    g_tx_data.clear();

    dce_uninit(dce);
}

TEST_CASE("read-write parameter", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK);
    g_tx_data.clear();
    
    args.param3 = 123;
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM3?\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\n+TESTPARAM3:123\r\n");
    g_tx_data.clear();
    
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM3=?\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\n+TESTPARAM3:(0-127)\r\n");
    g_tx_data.clear();
    
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTPARAM3=42\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\nOK\r\n");
    REQUIRE(args.param3 == 42);
    g_tx_data.clear();
    
    dce_uninit(dce);
}

TEST_CASE("consume linefeed after carriage return", "[dce]")
{
    dce_t* dce = dce_init(1024);
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATS3?\r\n") == DCE_OK);
    REQUIRE(g_tx_data == "ATS3?\r\r\n013\r\n\r\nOK\r\n");
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATS4?\r\n") == DCE_OK);
    REQUIRE(g_tx_data == "ATS4?\r\r\n010\r\n\r\nOK\r\n");
    dce_uninit(dce);
}

TEST_CASE("can reset dce even if command is pending", "[dce]")
{
    dce_t* dce = dce_init(1024);
    extended_commands_test_t args;
    dce_register_test_commands(dce, &args);
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATE0\r") == DCE_OK);
    g_tx_data.clear();
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "AT+TESTNORETURN\r") == DCE_OK);
    REQUIRE(g_tx_data == "\r\nBUSY\r\n");
    g_tx_data.clear();
    g_reset = false;
    REQUIRE(DCE_HANDLE_INPUT_STR(dce, "ATZ\r") == DCE_OK);
    REQUIRE(g_reset);
    dce_uninit(dce);
}

typedef std::function<void(int)> func_t;

typedef struct {
    const char* name;
    func_t func;
} stage_t;

void tcp_connect(int32_t ip, int16_t port) { }


void on_connect(void* ctx)
{
    
}

TEST_CASE("continuations for mqtt")
{
    stage_t stages[] = {
        {"connect", [&](int) {
            uint8_t ip[] = {192, 168, 0, 1};
            tcp_connect( *((int32_t*) ip), 54002 );
            
        }}
    };
}

