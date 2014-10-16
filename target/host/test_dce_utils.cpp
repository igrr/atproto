/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "catch.hpp"
#include <limits.h>
extern "C"
{
#include "dce_common.h"
#include "dce_utils.h"
}

TEST_CASE("dce_itoa", "[utils]")
{
    const int values[] = {
        -1, 0, 1, 10, 11, 3, 8, 38, 10301, -25, INT_MAX, INT_MIN
    };
    const size_t nvalues = sizeof(values)/sizeof(int);
    
    char buf[12];
    char buf_ref[12];
    size_t outsize;
    
    for (size_t i = 0; i < nvalues; ++i)
    {
        snprintf(buf_ref, sizeof(buf_ref), "%d", values[i]);
        dce_itoa(values[i], buf, sizeof(buf), &outsize);
        buf[outsize] = 0;
        REQUIRE( strcmp(buf, buf_ref) == 0);
    }
    
    dce_itoa(INT_MAX, buf, 5, &outsize);
    REQUIRE(outsize == 0);
}

TEST_CASE("dce_itoa_zeropad", "[utils]")
{
    const int values[] = {
        0, 1, 10, 11, 3, 8, 38, 10301
    };
    const size_t nvalues = sizeof(values)/sizeof(int);
    
    char buf[6];
    char buf_ref[6];
    
    for (size_t i = 0; i < nvalues; ++i)
    {
        snprintf(buf_ref, sizeof(buf_ref), "%05d", values[i]);
        dce_itoa_zeropad(values[i], buf, sizeof(buf)-1);
        buf[5] = 0;
        REQUIRE( strcmp(buf, buf_ref) == 0);
    }
}

TEST_CASE("dce_except_number", "[utils]")
{
    const int defval = -42;
    const char* strs[] = {
        "123984", "0", "01", "01a", "a0", ".05", "1.9", "-250", "2147483647",
    };
    
    const int vals[] = {
        123984, 0, 1, 1, defval, defval, 1, defval, 2147483647,
    };
    const size_t nvals = sizeof(vals) / sizeof(int);
    REQUIRE((sizeof(strs)/sizeof(const char*) - nvals) == 0);
    
    for (size_t i = 0; i < nvals; ++i)
    {
        const char* buf = strs[i];
        size_t size = strlen(buf);
        int val = dce_expect_number(&buf, &size, defval);
        REQUIRE(val == vals[i]);
    }
}

TEST_CASE("dce_strcpy", "[utils]")
{
    char buf[20];
    char* pbuf = buf;
    size_t bufsize = 10;
    size_t outsize;
    memset(buf, 0, sizeof(buf));
    dce_strcpy("test   8", pbuf, bufsize, &outsize);
    REQUIRE( outsize == 8 );
    pbuf += outsize;
    bufsize -= outsize;
    dce_strcpy("a1234", pbuf, bufsize, &outsize);
    REQUIRE( outsize == 2 );
    REQUIRE( memcmp(buf, "test   8a1\0\0\0\0", 14) == 0);
}

TEST_CASE("concatenate stuff", "[utils]")
{
    static const int valid_baudrates[] = {9600, 19200, 38400, 57600, 74880, 115200, 230400, 460800, 921600};
    static const int valid_baudrates_count = sizeof(valid_baudrates)/sizeof(int);
    char buf[128];
    size_t bufsize = sizeof(buf);
    char * pbuf = buf;
    size_t outsize;
    dce_strcpy("+IPR:(0),(", buf, bufsize, &outsize);
    pbuf += outsize;
    bufsize -= outsize;
    for (size_t i = 0; i < valid_baudrates_count; ++i)
    {
        dce_itoa(valid_baudrates[i], pbuf, bufsize, &outsize);
        pbuf += outsize;
        bufsize -= outsize;
        if (i != valid_baudrates_count - 1)
        {
            *pbuf = ',';
            ++pbuf;
            --bufsize;
        }
    }
    *pbuf = ')';
    ++pbuf;
    --bufsize;
    *pbuf = 0;
    const char* ref = "+IPR:(0),(9600,19200,38400,57600,74880,115200,230400,460800,921600)";
    REQUIRE(bufsize == sizeof(buf) - strlen(ref));
    REQUIRE(strcmp(buf, ref) == 0);
}

typedef struct {
    const char* str;
    int rc;
    uint8_t num[4];
} ip_test_case_t;

TEST_CASE("parse IPs", "[utils]")
{
    ip_test_case_t test_cases[] = {
        {"192.168.1.129",   0,  {192, 168, 1, 129}},
        {"1.2.3.4",         0,  {1, 2, 3, 4}},
        {"001.002.003.004", 0,  {1, 2, 3, 4}},
        {"1..3.4",          -1, {0}},
        {"1.2.3.4:80",      -1, {0}},
        {"http://1.2.3.4",  -1, {0}},
        {"ab.ef.03.24",     -1, {0}},
    };
    
    int test_cases_count = sizeof(test_cases) / sizeof(ip_test_case_t);
    uint8_t ip[4];
    ip_test_case_t* test_case = test_cases;
    for (int i = 0; i < test_cases_count; ++i, ++test_case)
    {
        REQUIRE(dce_parse_ip(test_case->str, ip) == test_case->rc);
        if (!test_case->rc)
        {
            REQUIRE(memcmp(ip, test_case->num, 4) == 0);
        }
    }
}

