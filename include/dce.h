/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_H
#define __DCE_H

#include "dce_common.h"

#define DCE_OK                 0
#define DCE_BUFFER_OVERFLOW    1
#define DCE_INVALID_INPUT      2

typedef struct dce_ dce_t;
typedef int dce_result_t;

typedef enum {
    DCE_RC_OK = 0,
    DCE_RC_CONNECT = 1,
    DCE_RC_NO_CARRIER = 2,
    DCE_RC_ERROR = 4,
} dce_result_code_t;



dce_t* dce_init(size_t rx_buffer_size);

dce_result_t dce_handle_input(dce_t* dce, const char* cmd, size_t size);

void dce_emit_basic_result_code(dce_t* dce, dce_result_code_t result);

// use size=-1 for zero-terminated strings
void dce_emit_information_response(dce_t* dce, const char* response, size_t size);
void dce_continue_information_response(dce_t* dce, const char* response, size_t size);
void dce_emit_extended_result_code(dce_t* dce, const char* response, size_t size);


void dce_uninit(dce_t* dce);

#endif //__DCE_H
