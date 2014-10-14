/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_TEST_COMMANDS_H
#define __DCE_TEST_COMMANDS_H

#include "dce.h"

typedef struct {
    int  param1;
    char param2[32];
    int  param3;
    char param4[32];
} extended_commands_test_t;


void dce_register_test_commands(dce_t* dce, extended_commands_test_t* commands_ctx);


#endif//__DCE_TEST_COMMANDS_H
