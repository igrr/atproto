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



dce_t* dce_init(int rx_buffer_size);

dce_result_t dce_handle_input(dce_t* parser, const char* cmd, size_t size);

void dce_emit_basic_result_code(dce_t* dce, dce_result_code_t result);
void dce_emit_information_response(dce_t* dce, const char* response);

void dce_uninit(dce_t* parser);


extern void user_dce_transmit(const char* data, size_t size);
extern void user_dce_reset();

#endif //__DCE_H
