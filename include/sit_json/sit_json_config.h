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
#include <zephyr/data/json.h>

#include "sit_json_config.h"

typedef struct {
    char state[15];
    int32_t distance;
} json_data_t;

typedef struct  {
    char type[15];
    json_data_t data;
} json_distance_msg_t; 


typedef struct {
    char type[16];
    char command[6];
} json_command_msg_t;


#ifdef __cplusplus
}
#endif

#endif  // __SIT_JSON_H__


