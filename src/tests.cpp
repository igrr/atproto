
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

std::string g_tx_data;
bool g_reset = false;


extern "C" {
#include "dce.h"
#include "dce_info_commands.h"
#include "dce_test_commands.h"

void user_dce_transmit(const char* data, size_t size)
{
    g_tx_data += std::string(data, size);
}

void user_dce_reset()
{
    g_reset = true;
}
    
void user_dce_assert(const char* msg)
{
    throw std::runtime_error(msg);
}

}

#define DCE_HANDLE_INPUT_STR(dce, str) dce_handle_input(dce, str, sizeof(str) - 1)

TEST_CASE("call extended format commands", "[dce]")
{
    dce_t* dce = dce_init(1024);
    dce_register_info_commands(dce);
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "AT+GMI\r") == DCE_OK );
    dce_uninit(dce);
    REQUIRE( g_tx_data == "AT+GMI\r\r\nERROR\r\n" );
}

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
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATV0\r") == DCE_OK );
    REQUIRE( g_tx_data == "ATV0\r0\r");
    g_tx_data.clear();
    REQUIRE( DCE_HANDLE_INPUT_STR(dce, "ATV1\r") == DCE_OK );
    REQUIRE( g_tx_data == "ATV1\r\r\nOK\r\n");
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

