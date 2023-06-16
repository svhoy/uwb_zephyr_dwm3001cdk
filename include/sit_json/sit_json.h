/*
 *  sit_json.h
 */
#ifndef __SIT_JSON_H__
#define __SIT_JSON_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "sit_json_config.h"

int json_distance_parser(char *json,  size_t len, void *val);
int json_encode_distance(char **json, double *distance);
int json_decode_state_msg(char *json, json_command_msg_t *command_struct);

#ifdef __cplusplus
}
#endif

#endif  // __SIT_JSON_H__
