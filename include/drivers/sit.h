#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "drivers/dw1000/deca_device_api.h"

/* Default antenna delay */
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

#define SPEED_OF_LIGHT 299702547

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
}header_t;

typedef struct {
    header_t header;
    uint16_t crc;
}msg_header_t;

typedef struct
{
    header_t header;
    uint32_t poll_rx_ts;
    uint32_t resp_tx_ts;
    uint16_t crc;
}msg_ss_twr_final_t;

typedef struct
{
    header_t header;
    uint32_t poll_tx_ts;
    uint32_t resp_rx_ts;
    uint32_t final_tx_ts;
    uint16_t crc;
}msg_ds_twr_final_t;

bool sit_init(dwt_config_t config);
void sit_setRxAfterTxDelay(uint32_t delay_us,uint16_t timeout);

void sit_startPoll(uint8_t* msg_data, uint16_t msg_size);
bool sit_sendAt(uint8_t* msg_data, uint16_t size, uint64_t tx_time);

uint32_t sit_poll_receive();

void sit_receiveNow(uint16_t timeout);
bool sit_checkReceivedIdMsg(msg_id_t id, msg_header_t& message);
bool sit_checkReceivedIdFinalMsg(msg_id_t id, msg_ss_twr_final_t& message);
bool sit_checkReceivedMessage(uint8_t* data, uint16_t expected_size);

void recover_tx_errors();
uint32_t sit_getRegStatus();

uint64_t get_tx_timestamp_u64(void);
uint64_t get_rx_timestamp_u64(void);