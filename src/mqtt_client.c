/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 * This is a port of PubSubClient by Nicholas O'Leary in plain C99.
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "dce_common.h"
#include "mqtt_client.h"

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE 128

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15

enum { MQTT_PROTOCOL_VERSION=3 };

enum mqtt_message_type_t
{
    MQTT_CONNECT = 1,
    MQTT_CONNACK,
    MQTT_PUBLISH,
    MQTT_PUBACK,
    MQTT_PUBREC,
    MQTT_PUBREL,
    MQTT_PUBCOMP,
    MQTT_SUBSCRIBE,
    MQTT_SUBACK,
    MQTT_UNSUBSCRIBE,
    MQTT_UNSUBACK,
    MQTT_PINGREQ,
    MQTT_PINGRESP,
    MQTT_DISCONNECT,
    MQTT_Reserved
};

enum { MQTT_TYPE_SHIFT = 4 };

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
    uint16_t next_message_id;
    uint8_t buffer[MQTT_MAX_PACKET_SIZE];
    const char* id;
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

void mqttc_connect(mqttc_t* client, const char* id, uint32_t ip, int port)
{
    client->id = id;
    // tcp_connect(ip, port, &mqttc_on_connect, client);
}

void mqttc_append_str(uint8_t** pdst, const uint8_t* dstend, const char* src)
{
    size_t length = strlen(src);
    uint8_t* dst = *pdst;
    if (dst + 2 + length >= dstend)
        DCE_FAIL("buffer size exceeded");
    memcpy(dst + 2, src, length);
    dst[0] = length >> 8;
    dst[1] = length & 0xff;
    *pdst = dst + 2 + length;
}

void mqttc_encode_remaining_length(uint8_t* dst, size_t value, size_t *out_length)
{
    uint8_t pos = 0;
    uint8_t* p = dst;

    for (; value; ++p)
    {
        uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value)
            byte |= 0x80;
        *p = byte;
    }
    
    *out_length = p - dst;
}

void mqttc_write(mqttc_t* client, uint8_t header, uint8_t* buf, const uint8_t* buf_end)
{
    uint8_t remaining_length_buffer[4];
    size_t remaining_length_size;
    size_t remaining_length = buf_end - buf;
    mqttc_encode_remaining_length(remaining_length_buffer, remaining_length, &remaining_length_size);
    uint8_t* msg_start = buf - 1 - (int)remaining_length_size;
    size_t msg_size = 1 + remaining_length_size + remaining_length;
    msg_start[0] = header;
    memcpy(msg_start + 1, remaining_length_buffer, remaining_length_size);
    // tcp_write(client->tcp_ctx, msg_start, msg_size, &mqttc_write_complete, client);
}

void mqttc_on_connect(mqttc_t* client)
{
    client->next_message_id = 1;
    const uint8_t d[] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_PROTOCOL_VERSION};
    const size_t offset = 5;
    uint8_t* pbuf = client->buffer + offset;
    memcpy(pbuf, d, sizeof(d));
    pbuf += sizeof(d);
    uint8_t* buf_end = client->buffer + sizeof(client->buffer);

    uint8_t v;
    if (client->will_topic)
    {
        v = 0x06 | (client->will_qos << 3) | (client->will_retain << 5);
    }
    else
    {
        v = 0x02;
    }
    
    if (client->auth_user)
    {
        v = v|0x80;
        if(client->auth_password != NULL)
        {
            v = v | (0x80>>1);
        }
    }
    
    *pbuf++ = v;
    *pbuf++ = ((MQTT_KEEPALIVE) >> 8);
    *pbuf++ = ((MQTT_KEEPALIVE) & 0xFF);

    mqttc_append_str(&pbuf, buf_end, client->id);
    
    if (client->will_topic) {
        mqttc_append_str(&pbuf, buf_end, client->will_topic);
        mqttc_append_str(&pbuf, buf_end, client->will_payload);
    }
    
    if (client->auth_user != NULL)
    {
        mqttc_append_str(&pbuf, buf_end, client->auth_user);
        if (client->auth_password != NULL)
        {
            mqttc_append_str(&pbuf, buf_end, client->auth_password);
        }
    }
    mqttc_write(client, MQTT_CONNECT << MQTT_TYPE_SHIFT, pbuf, buf_end);
}



void mqttc_disconnect(mqttc_t* client);
void mqttc_publish(mqttc_t* client, const char* topic, const char* payload, size_t payload_size, int retain);
void mqttc_subscribe(mqttc_t* client, const char* topic, int qos);
void mqttc_unsubscribe(mqttc_t* client, const char* topic);
