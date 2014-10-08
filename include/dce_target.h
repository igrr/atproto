#ifndef DCE_TARGET_H
#define DCE_TARGET_H

#include "dce.h"

extern void target_dce_transmit(const char* data, size_t size);
extern void target_dce_reset();
extern void target_dce_assert(const char* message);
extern void target_dce_request_process_command_line(dce_t* dce);
dce_result_t dce_process_command_line(dce_t* dce);

#endif//DCE_TARGET_H
