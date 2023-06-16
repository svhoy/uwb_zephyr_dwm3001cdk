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
#include <stdlib.h>
#include <string.h>
#include <zephyr/data/json.h>

#include "sit_json_config.h"
typedef struct {
    char type[16];
    char command[6];
} json_command_msg_t;


#ifdef __cplusplus
}
#endif

#endif  // __SIT_JSON_H__


