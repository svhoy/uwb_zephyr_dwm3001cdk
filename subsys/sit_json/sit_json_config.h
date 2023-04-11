/*
 *  sit_json.h
 */

#ifndef __SIT_JSON_CONFIG_H__
#define __SIT_JSON_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <data/json.h>

#include "sit_json_config.h"

struct json_data { 
    char state[10];
    uint8_t distance;
    uint8_t test_id;

};

struct json_distance_msg {
    char type[12];
    struct json_data data;
}; 

static const struct json_obj_descr data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct json_data, state, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct json_data, distance, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct json_data, test_id, JSON_TOK_NUMBER),
};

static const struct json_obj_descr distance_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct json_distance_msg, type, JSON_TOK_STRING),
    JSON_OBJ_DESCR_OBJECT(struct json_distance_msg, data, data_descr),
};

#ifdef __cplusplus
}
#endif

#endif  // __SIT_JSON_H__


