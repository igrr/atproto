/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H


#include <stdint.h>

typedef struct mqttc_ mqttc_t;

typedef void(*mqttc_callback_t)(void*, int, const char*, int);

mqttc_t* mqttc_create(mqttc_callback_t callback, void* callback_arg);
void mqttc_release(mqttc_t* client);
void mqttc_set_will(const char* topic, const char* payload, int qos, int retain);
void mqttc_set_auth(const char* user, const char* password);
void mqttc_connect(uint32_t ip, int port);
void mqttc_disconnect();
void mqttc_publish(const char* topic, const char* payload, size_t payload_size, int retain);
void mqttc_subscribe(const char* topic, int qos);
void mqttc_unsubscribe(const char* topic);

#endif//MQTT_CLIENT_H
