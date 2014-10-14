/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_PRIVATE_H
#define __DCE_PRIVATE_H

#include "dce_commands.h"
#include "dce_target.h"

typedef enum {
    COMMAND_STATE = 1,
    ONLINE_DATA_STATE = 2,
    ONLINE_COMMAND_STATE = 3
} state_t;

#define IS_COMMAND_STATE(state) (state & 1)
#define BEGINS(buf, c1, c2) (buf[0] == (c1) && buf[1] == (c2))

typedef struct {
    const char* leadin;
    const command_desc_t* commands;
    int commands_count;
    void* context;
} command_group_t;


#define DCE_MAX_COMMAND_GROUPS 8
#define DCE_MAX_ARGS 8

struct dce_
{
    size_t  rx_buffer_size;
    char*   rx_buffer;
    size_t  rx_buffer_pos;
    
    char*   command_line_buf;
    size_t  command_line_buf_size;
    size_t  command_line_length;
    
    command_group_t command_groups[DCE_MAX_COMMAND_GROUPS];
    size_t  command_groups_count;
    
    state_t state;
    
    char    cr;             /// 6.2.1 parameter S3
    char    lf;             /// 6.2.2 parameter S4
    char    bs;             /// 6.2.3 parameter S5
    char    echo;           /// 6.2.4 parameter E
    char    suppress_rc;    /// 6.2.5 parameter Q
    char    response_fmt;   /// 6.2.6 parameter V
    
    char    command_pending;
};

void dce_init_defaults(dce_t* dce);

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ "@" S2(__LINE__)
#define DCE_FAIL(msg) target_dce_assert("Internal error in " LOCATION ": " msg)

#endif//__DCE_PRIVATE_H
