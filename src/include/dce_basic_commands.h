#ifndef __DCE_BASIC_COMMANDS_H
#define __DCE_BASIC_COMMANDS_H

#include "dce.h"

dce_result_t dce_process_basic_command(dce_t* ctx, const char* buf, size_t size);

dce_result_t dce_process_sparameter_command(dce_t* ctx, const char* buf, size_t size);


#endif//__DCE_BASIC_COMMANDS_H
