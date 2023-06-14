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
    responder   ///< responder of SSTWR or DSTWR
} device_type;

/** 
 * Enum for different mesaurements states 
*/
typedef enum {
    measurement,  ///< measurement state will run the distance measurement
    sleep         ///< sleep state, wait for new informations
} device_state;

typedef struct {
    char deviceID[17];
    device_type type;
    device_state state;
    bool diagnostic;
} device_settings_t;

extern device_settings_t device_settings;

typedef enum {
    twr_1_poll,
    ss_twr_2_resp,
    dd_twr_2_resp
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
} msg_header_t;

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
    uint32_t sequence;
    float distance;
    uint8_t nlos_percent;
    float rssi_index_resp;
    float fp_index_resp;
} json_data_t;

typedef struct  {
    char type[15];
    json_data_t data;
} json_distance_msg_t; 


static dwt_config_t sit_device_config = {
    9,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    DWT_SFD_DW_8,     /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF,   /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Delay between frames, in UWB microseconds. */
// #define POLL_TX_TO_RESP_RX_DLY_UUS 250 // 240 * 1,026us ->
#define POLL_TX_TO_RESP_RX_DLY_UUS 240 // 240 * 1,026us ->
/* Delay between frames, in UWB microseconds. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 1200 // 650 * 1,026us ->
/* Receive response timeout. */
#define RESP_RX_TIMEOUT_UUS 1200 // 400 * 1,026us -> 

/**
 *  UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. 
**/
#define UUS_TO_DWT_TIME 65536

void set_device_state(char *comand);

#endif // __SIT_CONFIG_H__
