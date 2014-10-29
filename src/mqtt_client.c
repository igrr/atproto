/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "mqtt_client.h"

struct mqttc_
{
    mqttc_callback_t callback;
    void* callback_arg;
    const char* will_topic;
    const char* will_payload;
    int will_qos;
    int will_retain;
    const char* auth_user;
    const char* auth_password;
};

mqttc_t* mqttc_create(mqttc_callback_t callback, void* callback_arg)
{
    mqttc_t* client = (mqttc_t*) malloc(sizeof(mqttc_t));
    memset(client, 0, sizeof(mqttc_t));
    client->callback = callback;
    client->callback_arg = callback_arg;
    return client;
}

void mqttc_release(mqttc_t* client)
{
    free(client);
}

void mqttc_set_will(mqttc_t* client, const char* topic, const char* payload, int qos, int retain)
{
    client->will_topic = topic;
    client->will_payload = payload;
    client->will_qos = qos;
    client->will_retain = retain;
}

void mqttc_set_auth(mqttc_t* client, const char* user, const char* password)
{
    client->auth_user = user;
    client->auth_password = password;
}

void mqttc_connect(mqttc_t* client, uint32_t ip, int port)
{
    
}

void mqttc_disconnect(mqttc_t* client);
void mqttc_publish(mqttc_t* client, const char* topic, const char* payload, size_t payload_size, int retain);
void mqttc_subscribe(mqttc_t* client, const char* topic, int qos);
void mqttc_unsubscribe(mqttc_t* client, const char* topic);
