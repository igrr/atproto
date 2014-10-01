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
