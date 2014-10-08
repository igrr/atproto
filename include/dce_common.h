#ifndef __DCE_COMMON_H
#define __DCE_COMMON_H


#if defined(ESP_PLATFORM)
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#define SECTION_ATTR __attribute__((section(".irom0.text")))
#define malloc os_malloc
#define free os_free
#else
#include <string.h>
#include <stdlib.h>
#define SECTION_ATTR
#endif

#endif