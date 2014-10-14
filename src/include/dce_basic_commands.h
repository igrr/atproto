/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_BASIC_COMMANDS_H
#define __DCE_BASIC_COMMANDS_H

#include "dce.h"

dce_result_t dce_process_basic_command(dce_t* ctx, const char* buf, size_t size);

dce_result_t dce_process_sparameter_command(dce_t* ctx, const char* buf, size_t size);


#endif//__DCE_BASIC_COMMANDS_H
