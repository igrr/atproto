/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef __DCE_COMMON_H
#define __DCE_COMMON_H

#define VERSION_STRING "0.1"

#ifndef REVISION_STRING
#define REVISION_STRING "?"
#endif

#if defined(ESP_PLATFORM)
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#define SECTION_ATTR __attribute__((section(".irom0.text")))
#define malloc os_malloc
#define free os_free
#define eprintf os_printf
#define sprintf os_sprintf
#else
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define SECTION_ATTR
#define eprintf printf
#endif

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ "@" S2(__LINE__)
#define DCE_FAIL(msg) target_dce_assert("Internal error in " LOCATION ": " msg)
#define DCE_DEBUG(msg) eprintf("\r\n" LOCATION ": " msg "\r\n")
#define DCE_DEBUGV(msg, ...) eprintf("\r\n" LOCATION ": " msg "\r\n", __VA_ARGS__)


#endif
