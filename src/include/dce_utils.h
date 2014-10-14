/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_UTILS
#define __DCE_UTILS


int dce_ishex(char c);

char dce_htoi(char c);

int dce_expect_number(const char** buf, size_t *psize, int def_val);
void dce_itoa(int val, char* buf, size_t bufsize, size_t* outsize);
void dce_itoa_zeropad(int val, char* buf, size_t bufsize);  // works for nonnegative numbers
void dce_strcpy(const char* str, char* buf, size_t bufsize, size_t* outsize);
#endif //__DCE_UTILS
