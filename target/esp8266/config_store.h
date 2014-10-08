#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#define CONFIG_MAGIC   0x42
#define CONFIG_VERSION 1

#include <stdint.h>

typedef struct {
    int32_t magic;
    int32_t version;
    int32_t baud_rate;
    // bump CONFIG_VERSION when adding new fields
} config_t;


config_t* config_get();
void config_save();


#endif//CONFIG_STORE_H
