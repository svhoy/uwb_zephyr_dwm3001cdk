#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#include <zephyr/kernel.h>
#include <deca_device_api.h>

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

/***************************************************************************
* Initilization for DW3001 -> SPI Connection, DW3000, Antenna Delay  
*
* @param config dwt_config_t for Device configuration
* @param TX_ANT_DLY TX Antenna Delay Configuration
* @param RX_ANT_DLY RX Antenna Delay Configuration
*
* @return error code 
* [
*	-1 -> dwt_initialise failed,
*	-2 -> dwt_configure failed
* ]
****************************************************************************/
uint8_t sit_init(dwt_config_t *config, int TX_ANT_DLY, int RX_ANT_DLY);
void sit_setRxAfterTxDelay(uint32_t delay_us,uint16_t timeout);

void sit_startPoll(uint8_t* msg_data, uint16_t msg_size);
bool sit_sendAt(uint8_t* msg_data, uint16_t size, uint64_t tx_time);

uint32_t sit_poll_receive();

void sit_receiveNow(uint16_t timeout);
bool sit_checkReceivedIdMsg(msg_id_t id, msg_header_t* message);
bool sit_checkReceivedIdFinalMsg(msg_id_t id, msg_ss_twr_final_t* message);
bool sit_checkReceivedMessage(uint8_t* data, uint16_t expected_size);

void recover_tx_errors();
uint32_t sit_getRegStatus();

uint64_t get_tx_timestamp_u64(void);
uint64_t get_rx_timestamp_u64(void);

/*! ------------------------------------------------------------------------------------------------------------------
* @brief This function will continuously read the system status register until it matches the bits set in the mask
*        input parameter. It will then exit the function.
*        This is useful to use when waiting on particular events to occurs. For example, the user could wait for a
*        good UWB frame to be received and/or no receive errors have occurred.
*        The lower 32-bits of the system status register will be read in a while loop. Each iteration of the loop will check if a matching
*        mask value for the higher 32-bits of the system status register is set. If the mask value is set in the higher 32-bits of the system
*        status register, the function will return that value along with the last recorded value of the lower 32-bits of the system status
*        register. Thus, the user should be aware that this function will not wait for high and low mask values to be set in both the low and high
*        system status registers. Alternatively, the user can call this function to *only* check the higher or lower system status registers.
*
* input parameters
* @param lo_result - A pointer to a uint32_t that will contain the final value of the system status register (lower 32 bits).
*                    Pass in a NULL pointer to ignore returning this value.
* @param hi_result - A pointer to a uint32_t that will contain the final value of the system status register (higher 32 bits).
*                    Pass in a NULL pointer to ignore returning this value.
* @param lo_mask - a uint32 mask value that is used to check for certain bits to be set in the system status register (lower 32 bits).
*               Example values to use are as follows:
*               DWT_INT_TXFRS_BIT_MASK - Wait for a TX frame to be sent.
*               SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR - Wait for frame to be received and no reception errors.
*               SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR - Wait for frame to be received and no receive timeout errors
*                                                                                          and no reception errors.
*               SYS_STATUS_RXFR_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_ND_RX_ERR - Wait for packet to be received and no receive timeout errors
*                                                                                            and no reception errors.
*                                                                                            These flags are useful when polling for STS Mode 4 (no data)
*                                                                                            packets.
*               0 - The function will not wait for any bits in the system status register (lower 32 bits).
* @param hi_mask - a uint32 mask value that is used to check for certain bits to be set in the system status register (higher 32 bits).
*               Example values to use are as follows:
*               SYS_STATUS_HI_CCA_FAIL_BIT_MASK - Check for CCA fail status.
// *               0 - The function will not wait for any bits in the system status register (lower 32 bits).
*
* return None
*/
void waitforsysstatus(uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask);
