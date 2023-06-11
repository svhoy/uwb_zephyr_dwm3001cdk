/*
 *  sit_json.h
 */
#ifndef __SIT_JSON_H__
#define __SIT_JSON_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <data/json.h>

#include "sit_json_config.h"

int json_distance_parser(char *json,  size_t len, void *val);

#ifdef __cplusplus
}
#endif


#endif  // __SIT_JSON_H__