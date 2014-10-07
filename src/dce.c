/*

 
 */


#if defined (ESP_PLATFORM)
    #include "ets_sys.h"
    #include "os_type.h"
    #include "osapi.h"
    #include "mem.h"
#else
    #include <stdint.h>
    #include <stdlib.h>
#endif

#include "dce.h"
#include "dce_private.h"
#include "dce_utils.h"
#include "dce_basic_commands.h"
#include "dce_info_commands.h"

#define AT_ESCAPE '\\'
#define DCE_CONTINUE -1


static const char* dce_result_code_v0[] = {
    "0",
    "1",
    "2",
    "3",
    "4",
};

static const char* dce_result_code_v1[] = {
    "OK",
    "CONNECT",
    "NO CARRIER",
    "",
    "ERROR"
};


dce_t* SECTION_ATTR dce_init(size_t rx_buffer_size)
{
    dce_t* ctx = (dce_t*) malloc(sizeof(dce_t));
    
    ctx->rx_buffer_size  = rx_buffer_size;
    ctx->rx_buffer       = (char*) malloc(rx_buffer_size);
    ctx->rx_buffer_pos   = 0;
    
    ctx->command_groups_count = 0;
    ctx->state = COMMAND_STATE;
    
    dce_init_defaults(ctx);
    
    return ctx;
}

void SECTION_ATTR dce_init_defaults(dce_t* ctx)
{
    ctx->cr              = 13;
    ctx->lf              = 10;
    ctx->bs              = 8;
    ctx->echo            = 1;
    ctx->suppress_rc     = 0;
    ctx->response_fmt    = 1;
}


void SECTION_ATTR dce_register_command_group(dce_t* ctx, const char* leadin, const command_desc_t* desc, int ndesc, void* group_ctx)
{
    if (ctx->command_groups_count == DCE_MAX_COMMAND_GROUPS)
    {
        DCE_FAIL("command groups limit exceeded");
        return;
    }
    command_group_t* next = ctx->command_groups + ctx->command_groups_count;
    next->commands       = desc;
    next->commands_count = ndesc;
    next->leadin         = leadin;
    next->context        = group_ctx;

    ctx->command_groups_count++;
}
#if 0
// substitute escape sequence character pointed to by pc with unescaped character
// returns DCE_OK is character was consumed
// returns DCE_CONTINUE if the character should be processed further
dce_result_t dce_process_escape(dce_t* ctx, char* pc)
{
    if (ctx->escape == 0)
    {
        if (*pc != AT_ESCAPE)
            return DCE_CONTINUE;
        
        ctx->escape = 1;
        ctx->escape_char = 0;
        return DCE_OK;
    }

    char h = dce_htoi(*pc);
    if (h < 0)
        return DCE_INVALID_INPUT;
    
    if (ctx->escape == 1)
    {
        // first hex character
        ctx->escape_char |= (h << 8);
        ctx->escape++;
        return DCE_OK;
    }
    
    // second hex character
    ctx->escape_char |= h;
    ctx->escape = 0;
    *pc = ctx->escape_char;
    return DCE_CONTINUE;
}
#endif

void SECTION_ATTR dce_emit_basic_result_code(dce_t* dce, dce_result_code_t result)
{
    if (dce->suppress_rc)   // 6.2.5 Result code suppression
        return;

    if (dce->response_fmt == 0) // 6.2.6 DCE response format
    {
        const char* text = dce_result_code_v0[result];
        size_t length = strlen(text);
        user_dce_transmit(text, length);
        user_dce_transmit(&dce->cr, 1);
    }
    else
    {
        const char crlf[] = {dce->cr, dce->lf};
        user_dce_transmit(crlf, 2);
        const char* text = dce_result_code_v1[result];
        size_t length = strlen(text);
        user_dce_transmit(text, length);
        user_dce_transmit(crlf, 2);
    }
}

void SECTION_ATTR dce_emit_extended_result_code_with_args(dce_t* dce, const char* command_name, size_t size, arg_t* args, size_t argc)
{
    if (dce->suppress_rc)   // 6.2.5 Result code suppression
        return;
    const char crlf[] = {dce->cr, dce->lf};
    if (dce->response_fmt == 1)
    {
        user_dce_transmit(crlf, 2);
    }
    if (size == -1)
        size = strlen(command_name);
    user_dce_transmit("+", 1);
    user_dce_transmit(command_name, size);
    user_dce_transmit(":", 1);
    for (size_t iarg = 0; iarg < argc; ++iarg)
    {
        arg_t* arg = args + iarg;
        if (arg->type == ARG_TYPE_STRING)
        {
            const char* str = arg->value.string;
            size_t str_size = strlen(str);
            user_dce_transmit(str, str_size);
        }
        else if (arg->type == ARG_TYPE_NUMBER)
        {
            char buf[12];
            size_t str_size;
            dce_itoa(arg->value.number, buf, sizeof(buf), &str_size);
            user_dce_transmit(buf, str_size);
        }
        if (iarg != argc - 1)
            user_dce_transmit(",", 1);
    }
    user_dce_transmit(crlf, 2);
}

void SECTION_ATTR dce_emit_extended_result_code(dce_t* dce, const char* response, size_t size)
{
    if (dce->suppress_rc)   // 6.2.5 Result code suppression
        return;
    const char crlf[] = {dce->cr, dce->lf};
    if (dce->response_fmt == 1)
    {
        user_dce_transmit(crlf, 2);
    }
    if (size == -1)
        size = strlen(response);
    user_dce_transmit(response, size);
    user_dce_transmit(crlf, 2);
}

void SECTION_ATTR dce_emit_information_response(dce_t* dce, const char* response, size_t size)
{
    const char crlf[] = {dce->cr, dce->lf};
    if (dce->response_fmt == 1)
    {
        user_dce_transmit(crlf, 2);
    }
    if (size == -1)
        size = strlen(response);
    user_dce_transmit(response, size);
    user_dce_transmit(crlf, 2);
}

dce_result_t SECTION_ATTR dce_parse_args(const char* cbuf, size_t size, size_t* pargc, arg_t* args)
{
    // we'll be parsing arguments in place
    char* buf = (char*) cbuf; //it actually is a modifiable rx buffer, so TODO: remove const everywhere up the call chain
    int argc = 0;
    
    while (size > 0)
    {
        char c = *buf;
        arg_t arg;
        arg.type = ARG_NOT_SPECIFIED;
        arg.value.string = 0;
        
        if (c == '"')    // it's a string
        {
            ++buf;
            --size;
            const char* str = buf;
            for (;size > 0 && *buf != '"'; ++buf, --size)
            {
                // TODO: handle escape sequences
            }
            if (*buf != '"')    // line has ended without a closing quote
                return DCE_INVALID_INPUT;
            *buf = 0;
            arg.type = ARG_TYPE_STRING;
            arg.value.string = str;
            ++buf;
            --size;
            if (size > 0 && *buf == ',')
            {
                ++buf;
                --size;
            }
        }
        // TODO: add support for hex and binary numbers (5.4.2.1)
        else if (c >= '0' && c <= '9') // it's a number
        {
            arg.value.number = dce_expect_number((const char**)&buf, &size, 0);
            arg.type = ARG_TYPE_NUMBER;
            if (size > 0 && *buf == ',')
            {
                ++buf;
                --size;
            }
        }
        else if (c != ',')
        {
            return DCE_INVALID_INPUT;
        }
        args[argc++] = arg;
    }
    *pargc = argc;
    return DCE_OK;
}

dce_result_code_t SECTION_ATTR dce_process_args_run_command(dce_t* ctx, const command_group_t* group, const command_desc_t* command, const char* buf, size_t size)
{
    int flags = command->flags;
    if (size == 0)    // 5.4.3.1 Action execution, no arguments
    {
        if (!(flags & DCE_EXEC))
        {
            dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
            return DCE_OK;
        }
        return (*command->fn)(ctx, group->context, DCE_EXEC, 0, 0);
    }
    
    char c = buf[0];
    if (c == '?')   // 5.4.4.3 parameter read (AT+FOO?)
    {
        if (!(flags & DCE_PARAM) || !(flags & DCE_READ))
        {
            dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
            return DCE_OK;
        }
        return (*command->fn)(ctx, group->context, DCE_READ, 0, 0);
    }
    else if (c == '=')
    {
        if (size > 1 && buf[1] == '?')  // 5.4.3.2, 5.4.4.4 action or parameter test (AT+FOO=?)
        {
            if (!(flags & DCE_TEST))
            {
                // paramter or action is supported, but does not respond to queries
                dce_emit_basic_result_code(ctx, DCE_RC_OK);
                return DCE_OK;
            }
            return (*command->fn)(ctx, group->context, DCE_TEST, 0, 0);
        }
        else    // 5.4.4.2, 5.4.3.1 paramter set or execute action with subparameters (AT+FOO=params)
        {
            size_t argc = 0;
            arg_t args[DCE_MAX_ARGS];
            int rc = dce_parse_args(buf + 1, size - 1, &argc, args);
            if (rc != DCE_OK)
            {
                dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
                return DCE_OK;
            }
            int kind = (flags & DCE_ACTION) ? DCE_EXEC : DCE_WRITE;
            return (*command->fn)(ctx, group->context, kind, argc, args);
        }
    }
    DCE_FAIL("should be unreachable");
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_process_extended_format_command(dce_t* ctx, const char* buf, size_t size)
{
    for (int igrp = 0; igrp < ctx->command_groups_count; ++igrp)
    {
        const command_group_t* group = ctx->command_groups + igrp;
        int pos;
        for (pos = 0;
             pos < size && group->leadin[pos] != 0 && buf[pos] == group->leadin[pos];
             ++pos);
        if (group->leadin[pos] != 0)
            continue;
        for (int icmd = 0; icmd < group->commands_count; ++icmd)
        {
            const command_desc_t* command = group->commands + icmd;
            for (;pos < size && command->name[pos] != 0 && buf[pos] == command->name[pos]; ++pos);
            
            if (command->name[pos] != 0)
                continue;
            
            if (pos < size && buf[pos] != '=' && buf[pos] != '?')
                continue;
            
            return dce_process_args_run_command(ctx, group, command, buf + pos, size - pos);
        }
    }
    dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
    return DCE_OK;
}


dce_result_t SECTION_ATTR dce_process_command_line(dce_t* ctx)
{
    // 5.2.1 command line format
    size_t size = ctx->rx_buffer_pos;
    char *buf = ctx->rx_buffer;
    
    if (size < 2)
    {
        dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    // command line should start with 'AT' prefix
    // TODO: implement support for command line repetition (5.2.4)
    if (!BEGINS(buf, 'A', 'T') &&
        !BEGINS(buf, 'a', 't'))
    {
        dce_emit_basic_result_code(ctx, DCE_RC_ERROR);
        return DCE_OK;
    }
    
    if (size == 2)   // 'AT' was sent
    {
        dce_emit_basic_result_code(ctx, DCE_RC_OK);
        return DCE_OK;
    }
    
    buf += 2;
    size -= 2;
    // process single command
    // TODO: implement support for multiple commands per line
    
    char c = buf[0];
    if (c == '+')
        return dce_process_extended_format_command(ctx, buf + 1, size - 1);

    if (c == 'S')  // S-parameter (5.3.2)
        return dce_process_sparameter_command(ctx, buf + 1, size - 1);
    
    return dce_process_basic_command(ctx, buf, size);
}


dce_result_t SECTION_ATTR dce_handle_command_state_input(dce_t* ctx, const char* cmd, size_t size)
{
    if (ctx->rx_buffer_pos + size > ctx->rx_buffer_size)
    {
        // FIXME: better handling of rx buffer overflow
        // may actually need less space because of escaped characters
        // TODO: send error response
        return DCE_BUFFER_OVERFLOW;
    }
    
    // TODO: implement command line editing (5.2.2)
    for (size_t i = 0; i < size; ++i)
    {
        char c = cmd[i];
        if (ctx->echo)
        {
            user_dce_transmit(&c, 1);
        }
        if (c == ctx->cr)
        {
            dce_result_t rc = dce_process_command_line(ctx);
            if (rc != DCE_OK)
                return rc;
            
            ctx->rx_buffer_pos = 0;
        }
        else
        {
            ctx->rx_buffer[ctx->rx_buffer_pos++] = c;
        }
    }
    
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_data_state_input(dce_t* ctx, const char* cmd, size_t size)
{
    return DCE_OK;
}

dce_result_t SECTION_ATTR dce_handle_input(dce_t* ctx, const char* cmd, size_t size)
{
    if (IS_COMMAND_STATE(ctx->state))
    {
        return dce_handle_command_state_input(ctx, cmd, size);
    }
    else
    {
        return dce_handle_data_state_input(ctx, cmd, size);
    }
}

void SECTION_ATTR dce_uninit(dce_t* ctx)
{
    free(ctx->rx_buffer);
    free(ctx);
}
