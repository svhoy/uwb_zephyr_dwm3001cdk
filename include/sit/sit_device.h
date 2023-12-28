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
 * @file sit_device.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief SIT device header.
 *
 * Device Functions Definiton to read the device status register
 * 
 * @bug No known bugs.
 * @todo everything 
 */
#include <stdint.h>
#include <stddef.h>

/***************************************************************************
* Read the systemstatus low register and return the device status
*
* @return uint_32 status register 
* 
****************************************************************************/
uint32_t sit_get_device_status(void);

/***************************************************************************************************************************************************
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
*               0 - The function will not wait for any bits in the system status register (lower 32 bits).
*
* return None
*/
void waitforsysstatus(
    uint32_t *lo_result,
    uint32_t *hi_result,
    uint32_t lo_mask,
    uint32_t hi_mask
    );

/***************************************************************************
 * Set antenna delay. 
 *
 * @param rx_delay delay when recive the uwb frame
 * @param tx_delay delay when transmit the uwb frame
 *
 * @return None
 * 
*****************************************************************************/
void set_antenna_delay(
    uint16_t rx_delay,
    uint16_t tx_delay
);

/***************************************************************************
 * Get tx antenna delay. 
 *
 * @return uint16_t tx antenna delay
 * 
*****************************************************************************/
uint16_t get_tx_ant_dly();

/***************************************************************************
 * Get rx antenna delay. 
 *
 * @return uint16_t tx antenna delay
 * 
*****************************************************************************/
uint16_t get_rx_ant_dly();

/********************************************************************************
 * @brief Init the Device ID 
 * 
 * This Function saves the Device ID in a char array for further usage
 *
 * @param  none
 *
 * @return  none
 */
void init_device_id(void);

/********************************************************************************
 * @brief Get Device ID for usage
 * 
 * This Function stores the device id in a char array 
 * which comes as pointer as parameter
 *
 * @param  deviceID     -> Pointer to a char array with the size of minimal 17
 * @param  deviceIDSize -> size of the pointer
 *
 * @return  none
 */
void get_device_id(char **deviceID);
