#include "catch.hpp"
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


