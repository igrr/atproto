#ifndef __DCE_COMMON_H
#define __DCE_COMMON_H


#if defined(ESP_PLATFORM)

#define malloc os_malloc
#define free os_free
#define SECTION_ATTR __attribute__((section(".irom0.text")))
#else

#define SECTION_ATTR
#endif

#include <stdint.h>
#include <string.h>


#endif//__DCE_COMMON_H
