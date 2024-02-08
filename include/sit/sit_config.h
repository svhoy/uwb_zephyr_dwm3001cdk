/**********************************************************************************
 * 
 *  Copyright (C) 2023  Sven Hoyer
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
***********************************************************************************/

/** 
 * @file sit_config.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Defniton config options for SIT system.
 *
 * This header defines different configurations for the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#ifndef __SIT_CONFIG_H__
#define __SIT_CONFIG_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <deca_device_api.h>

#include <sit_json/sit_json_config.h>

/** 
 * Enum to define if a device is an initator or an responder
*/
typedef enum {
    initiator,  ///< initiator of SSTWR or DSTWR
    responder,  ///< responder of SSTWR or DSTWR
    none        ///< init State for every Device
} device_type_t;

extern device_type_t device_type;


/** 
 * Enum for different mesaurements states 
*/
typedef enum {
    measurement,  ///< measurement state will run the distance measurement
    sleep         ///< sleep state, wait for new informations
} device_state_t;

typedef enum {
    ss_twr,
    ds_3_twr,
    ds_4_twr, ///< not Implemented yet 
    ds_all_twr, ///< not Implemnted yet
} measurement_type_t;

typedef struct {
    uint8_t deviceID;
    uint8_t devices;
    uint8_t initiator; 
    uint8_t responder;
    uint16_t tx_ant_dly;
    uint16_t rx_ant_dly;
    device_type_t device_type;
    device_state_t state;
    measurement_type_t measurement_type;
    bool diagnostic;
    uint8_t min_measurement;
    uint8_t max_measurement;
} device_settings_t;

extern device_settings_t device_settings;

typedef enum {
    twr_1_poll,
    ss_twr_2_resp,
    ds_twr_2_resp, 
    ds_twr_3_final,
} msg_id_t;

typedef struct {
    msg_id_t id;
    uint8_t sequence;
    uint8_t source;
    uint8_t dest;
} header_t;

typedef struct {
    header_t header;
    uint16_t crc;
} msg_simple_t;

typedef struct {
    uint8_t nlos; // NLOS percentage
    float rssi; // Recived Signal Strangth Index (Recived Path Index)
    float fpi; // First Path Index
} diagnostic_info;

typedef struct {
    header_t header;
    uint32_t poll_rx_ts;
    uint32_t resp_tx_ts;
    uint16_t crc;
} msg_ss_twr_final_t;

typedef struct {
    header_t header;
    uint32_t poll_tx_ts;
    uint32_t resp_rx_ts;
    uint32_t final_tx_ts;
    uint16_t crc;
} msg_ds_twr_final_t;

typedef struct {
    char state[15];
    uint8_t responder;
    uint32_t sequence;
    uint32_t measurements;
    float distance;
    uint8_t nlos_percent;
    float rssi_index_resp;
    float fp_index_resp;
} json_data_t;

typedef struct  {
    char type[15];
    json_data_t data;
} json_distance_msg_t; 

extern dwt_config_t sit_device_config;

/* Delay between frames, in UWB microseconds. */
// #define POLL_TX_TO_RESP_RX_DLY_UUS 250 // 240 * 1,026us ->
#define POLL_TX_TO_RESP_RX_DLY_UUS 500 // 240 * 1,026us ->
/* Delay between frames, in UWB microseconds. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 1250 // 650 * 1,026us ->
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500 // 650 * 1,026us ->
/* Receive response timeout. */
#define RESP_RX_TIMEOUT_UUS 1500 // 400 * 1,026us -> 


#define CPU_PROCESSING_TIME 400
#define DS_POLL_TX_TO_RESP_RX_DLY_UUS (750 + CPU_PROCESSING_TIME)
#define DS_RESP_RX_TO_FINAL_TX_DLY_UUS (1000 + CPU_PROCESSING_TIME)
#define DS_RESP_RX_TIMEOUT_UUS 1200
#define DS_PRE_TIMEOUT 5

#define DS_POLL_RX_TO_RESP_TX_DLY_UUS 1350
#define DS_RESP_TX_TO_FINAL_RX_DLY_UUS 1200
#define DS_FINAL_RX_TIMEOUT 1800


/**
 *  UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. 
**/
#define UUS_TO_DWT_TIME 65536

void set_device_state(char *comand);
void set_device_id(uint8_t device_id);
void set_responder(uint8_t responder);
void set_min_measurement(uint8_t measurement);
void set_max_measurement(uint8_t measurement);
void set_rx_ant_dly(uint16_t dly);
void set_tx_ant_dly(uint16_t dly);

#endif // __SIT_CONFIG_H__
