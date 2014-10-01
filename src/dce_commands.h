#ifndef __DCE_COMMANDS
#define __DCE_COMMANDS

#include "dce.h"

#define DCE_PARAM    1
#define DCE_ACTION   2
#define DCE_TEST     4
#define DCE_EXEC     8
#define DCE_WRITE   16
#define DCE_READ    32

typedef enum {
    ARG_TYPE_NUMBER,
    ARG_TYPE_STRING,
    ARG_NOT_SPECIFIED
} arg_type_t;

typedef struct {
    arg_type_t type;
    union {
        int number;
        const char * string;
    } value;
} arg_t;

typedef dce_result_t (*cmdhandler_t)(dce_t* dce, void* group_context, int kind, size_t argc, arg_t* argv);

typedef struct
{
    const char* name;
    cmdhandler_t fn;
    int flags;
} command_desc_t;

void dce_register_command_group(dce_t* dce, const char* leadin, const command_desc_t* desc, int ndesc, void* ctx);


#endif//__DCE_COMMANDS
