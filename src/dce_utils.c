/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "dce_utils.h"

int SECTION_ATTR dce_ishex(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

char SECTION_ATTR dce_htoi(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

int SECTION_ATTR dce_expect_number(const char** buf, size_t *psize, int def_val)
{
    size_t s = *psize;
    if (s == 0)
        return def_val;
    
    int result = 0;
    int i;
    const char* b = *buf;
    for (i = 0; i < s; ++i, ++b)
    {
        int x = *b - '0';
        if (x < 0 || x > 9)
            break;
        result = result * 10 + x;
    }
    
    if (i == 0)
        return def_val;
    
    *psize -= i;
    *buf += i;
    return result;
}

void SECTION_ATTR dce_itoa(int val, char* buf, size_t bufsize, size_t* outsize)
{
    char negative = 0;
    if (val == 0)
    {
        buf[0] = '0';
        *outsize = 1;
        return;
    }
    else if (val < 0)
    {
        negative = 1;
    }
    int digits[10];
    int i;
    for (i = 0; val != 0; ++i)
    {
        int div = val / 10;
        digits[i] = val - div * 10;
        val = div;
    }
    if (bufsize < i + negative)
    {
        *outsize = 0;
        return;
    }
    
    char* start = buf;
    if (negative)
    {
        *buf = '-';
        ++buf;
    }
    for (; i > 0; --i, ++buf)
    {
        *buf = '0' + ((negative) ? -digits[i - 1] : digits[i - 1]);
    }
    *outsize = buf - start;
}

void SECTION_ATTR dce_itoa_zeropad(int val, char* buf, size_t bufsize)
{
    int digits[10];
    int i;
    for (i = 0; val > 0; ++i)
    {
        int div = val / 10;
        digits[i] = val - div * 10;
        val = div;
    }
    
    for (int j = 0; j < bufsize - i; ++j, ++buf)
        *buf = '0';
    for (; i > 0; --i, ++buf)
        *buf = '0' + digits[i - 1];
}

void SECTION_ATTR dce_strcpy(const char* str, char* buf, size_t bufsize, size_t* outsize)
{
    const char* start = buf;
    for (;bufsize; --bufsize, ++buf, ++str)
    {
        char c = *str;
        if (!c)
            break;
        *buf = c;
    }
    
    *outsize = buf - start;
}

int SECTION_ATTR dce_parse_ip(const char* buf, uint8_t* ip)
{
    size_t len = strlen(buf);
    const char* pb = buf;
    for (int i = 0; i < 4 && len > 0; ++i)
    {
        int val = dce_expect_number(&pb, &len, -1);
        if (val < 0 || val > 255)
            return -1;
        ip[i] = val;
        if (i == 3)
            break;
        if (!len || *pb != '.')
            return -1;
        --len;
        ++pb;
    }
    if (len)
        return -1;
    return 0;
}

int SECTION_ATTR dce_expect_string(char** pbuf, size_t* psize, char** result)
{
    char* src = *pbuf;
    size_t size = *psize;
    if (size < 2 || *src != '"')
        return -1;
    ++src;
    --size;
    int quote = 0;
    *result = src;
    for (char* dst = src; size > 0; ++src, --size)
    {
        char c = *src;
        if (quote == 0 && c == '\\')
        {
            quote = 1;
            continue;
        }
        if (quote == 1)
        {
            switch (c) {
                case '\\':
                case '"':
                    *dst = c;
                    break;
                case 'n':
                    *dst = '\n';
                    break;
                case 'r':
                    *dst = '\r';
                    break;
                case 't':
                    *dst = '\t';
                    break;
                case 'x':
                    quote = 2;
                    continue;
                default:
                    DCE_DEBUGV("invalid escape sequence: \\%c", c);
                    return -1;
            }
            ++dst;
            quote = 0;
            continue;
        }
        if (quote == 2)
        {
            if (size < 2)
            {
                DCE_DEBUG("incomplete escape sequence");
                return -1;
            }
            char c1 = src[0];
            char c2 = src[1];
            ++src;
            --size;
            if (!dce_ishex(c1) || !dce_ishex(c2))
            {
                DCE_DEBUG("escape sequence \\x?? contains non hex characters");
                return -1;
            }
            char result = (dce_htoi(c1) << 4) | dce_htoi(c2);
            if (result == 0)
            {
                DCE_DEBUG("escape sequence \\x00 is not supported");
                return -1;
            }
            *dst = result;
            ++dst;
            quote = 0;
            continue;
        }
        if (c == '"')
        {
            *dst = 0;
            *pbuf = ++src;
            *psize = --size;
            return 0;
        }
        *dst = c;
        ++dst;
    }
    DCE_DEBUG("string is missing a closing quote");
    return -1;
}


