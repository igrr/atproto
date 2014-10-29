/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H


#include <stdint.h>

typedef void(*mqttc_callback_t)(void*, int, const char*, int);

typedef struct mqttc_ mqttc_t;

mqttc_t* mqttc_create(mqttc_callback_t callback, void* callback_arg);
void mqttc_release(mqttc_t* client);
void mqttc_set_will(mqttc_t* client, const char* topic, const char* payload, int qos, int retain);
void mqttc_set_auth(mqttc_t* client, const char* user, const char* password);
void mqttc_connect(mqttc_t* client, uint32_t ip, int port);
void mqttc_disconnect(mqttc_t* client);
void mqttc_publish(mqttc_t* client, const char* topic, const char* payload, size_t payload_size, int retain);
void mqttc_subscribe(mqttc_t* client, const char* topic, int qos);
void mqttc_unsubscribe(mqttc_t* client, const char* topic);

#endif//MQTT_CLIENT_H
