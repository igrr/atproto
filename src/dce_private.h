#ifndef __DCE_PRIVATE_H
#define __DCE_PRIVATE_H

#include "dce_commands.h"

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

struct dce_
{
    size_t  rx_buffer_size;
    char*   rx_buffer;
    size_t  rx_buffer_pos;
    
    command_group_t command_groups[DCE_MAX_COMMAND_GROUPS];
    size_t  command_groups_count;
    
    state_t state;
    
    char    escape;         /// escape symbol received flag
    char    escape_char;    /// used to collect the escaped character code
    
    char    cr;             /// 6.2.1 parameter S3
    char    lf;             /// 6.2.2 parameter S4
    char    bs;             /// 6.2.3 parameter S5
    char    echo;           /// 6.2.4 parameter E
    char    suppress_rc;    /// 6.2.5 parameter Q
    char    response_fmt;   /// 6.2.6 parameter V
};

void dce_init_defaults(dce_t* dce);


#endif//__DCE_PRIVATE_H
