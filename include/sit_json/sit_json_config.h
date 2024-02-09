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

typedef struct {
    char type[16];
    char initiator_device[17];
    uint8_t initiator;
    char responder_device[4][17];
    uint8_t responder;
    uint8_t min_measurement;
    uint8_t max_measurement;
    char measurement_type[11];
    uint16_t rx_ant_dly;
    uint16_t tx_ant_dly;
} json_setup_msg_t;

#ifdef __cplusplus
}
#endif

#endif  // __SIT_JSON_H__


