#include "sit.h"

#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include "deca_probe_interface.h"
#include <config_options.h>
#include <deca_device_api.h>
#include <shared_defines.h>
#include <port.h>


LOG_MODULE_REGISTER(SIT_Module, LOG_LEVEL_INF);

static uint32_t status_reg = 0;

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power of the spectrum at the current
 * temperature. These values can be calibrated prior to taking reference measurements. See NOTE 5 below. */
extern dwt_txconfig_t txconfig_options;


uint8_t sit_init(dwt_config_t *config, int TX_ANT_DLY, int RX_ANT_DLY) {
	
	device_init();
	/* Configure SPI rate, for initialize it should not faster than 7 MHz */
	port_set_dw_ic_spi_slowrate();
	
	/* Reset and initialize DW chip. */
	reset_DWIC(); /* Target specific drive of RSTn line into DW3000 low for a period. */

	k_msleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)
	
	/* Configure SPI rate, after init up to 36 MHz */
	port_set_dw_ic_spi_fastrate();

	/* Probe set device speific functions (eg. write and read spi). */
	int test = dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

	/* Need to make sure DW IC is in IDLE_RC before proceeding */
	while (!dwt_checkidlerc()){
	};
	/* Configure SPI rate, DW3000 supports up to 36 MHz */
	port_set_dw_ic_spi_fastrate();
	if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
		LOG_WRN("dwt_initialise failed");
		return -1;
	}
	/* Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards. */
	dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

	/* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
	if (dwt_configure(config)){
		LOG_WRN("CONFIG FAILED     ");
		return -2;
	}
	/* Configure the TX spectrum parameters (power, PG delay and PG count) */
	dwt_configuretxrf(&txconfig_options);
	
	/* Apply default antenna delay value. */
	dwt_setrxantennadelay(RX_ANT_DLY);
	dwt_settxantennadelay(TX_ANT_DLY);

	/* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
	 * Note, in real low power applications the LEDs should not be used. */
	dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

	return 1;
}

void sit_setRxAfterTxDelay(uint32_t delay_us, uint16_t timeout) {
	dwt_setrxaftertxdelay(delay_us);
	dwt_setrxtimeout(timeout);
}

void sit_startPoll(uint8_t* msg_data, uint16_t msg_size){
	dwt_writesysstatuslo(DWT_INT_TXFRS_BIT_MASK);
	dwt_writetxdata(msg_size, msg_data, 0); // 0 offset
	dwt_writetxfctrl(msg_size, 0, 1);//ranging bit unused by DW1000
	dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//switch to rx after `setrxaftertxdelay`
}

bool sit_sendAt(uint8_t* msg_data, uint16_t size, uint64_t tx_time){
	
	uint32_t reg1 = dwt_readsysstatuslo();
	dwt_setdelayedtrxtime(tx_time);
	dwt_writetxdata(size, msg_data, 0); 
	dwt_writetxfctrl(size, 0, 1); 
	bool late = dwt_starttx(DWT_START_TX_DELAYED);
	if(late == DWT_SUCCESS) {
		waitforsysstatus(&status_reg, NULL, DWT_INT_TXFRS_BIT_MASK, 0);
		dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK); // write to clear send status bit 
		LOG_WRN("Send Success");
		return true;
	} else {
		uint32_t reg2 = dwt_readsysstatuslo();
		recover_tx_errors();
		status_reg = dwt_readsysstatuslo();
		LOG_WRN("sit_sendAt() - dwt_starttx() late");
		return false;
	}
}

uint32_t sit_poll_receive() {
	uint32_t l_status_reg;
	waitforsysstatus(&l_status_reg, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR), 0);
	return l_status_reg;
}

void sit_receiveNow(uint16_t timeout) {
	dwt_setrxtimeout(timeout); // 0 : disable timeout
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

bool sit_checkReceivedMessage(uint8_t* data, uint16_t expected_size) {
	bool result = false;
	status_reg = sit_poll_receive();
	LOG_INF("Test: %08x & %08x", status_reg, DWT_INT_RXFCG_BIT_MASK);
	if(status_reg & DWT_INT_RXFCG_BIT_MASK){
		/* Clear good RX frame event in the DW IC status register. */
		dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
		uint16_t size = dwt_getframelength();
		if (size == expected_size) {
			dwt_readrxdata(data, size, 0);
			result = true;
		}else{
			LOG_ERR("rx size %u != expected_size %u",size,expected_size);
		}
	}else{

		dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		LOG_WRN("sit_checkReceivedMessage() no 'RX Frame Checksum Good'");
		uint32_t reg2 = dwt_readsysstatuslo();
		uint32_t reg_removed = (status_reg ^ reg2);
		LOG_WRN("sit_checkReceivedMessage() reg1 = 0x%08x ; reg2 = 0x%08x ; removed =  0x%08x",status_reg,reg2,reg_removed);
		status_reg = reg2;
	}

	return result;
}
bool sit_checkReceivedIdMsg(msg_id_t id, msg_header_t& message) {
	
	bool result = false;
	if(sit_checkReceivedMessage((uint8_t*)&message, sizeof(msg_header_t))){
		if(message.header.id == id){//TODO check that dest is self when id is moved as global here
			result = true;
		}else{
			LOG_ERR("sit_checkReceivedId() mismatch id(%u/%u)",(uint8_t)id,(uint8_t)message.header.id);
		}
	}else{
		LOG_ERR("sit_checkReceivedId(%u,header) fail",(uint8_t)id);
	}
	return result;
}


bool sit_checkReceivedIdFinalMsg(msg_id_t id, msg_ss_twr_final_t& message) {
	bool result = false;
	if(sit_checkReceivedMessage((uint8_t*)&message, sizeof(msg_ss_twr_final_t))){
		if(message.header.id == id){//TODO check that dest is self when id is moved as global here
			result = true;
		}else{
			LOG_ERR("sit_checkReceivedIdFinalMsg() mismatch id(%u/%u)",(uint8_t)id,(uint8_t)message.header.id);
		}
	}else{
		LOG_ERR("sit_checkReceivedIdFinalMsg(%u,header) fail",(uint8_t)id);
	}
	return result;
}

void recover_tx_errors() {
	uint32_t status = dwt_readsysstatuslo();
	if(status & DWT_INT_RXFCE_BIT_MASK)
	{
		LOG_INF("recovering TX errors 0x%08x", (status & DWT_INT_RXFCE_BIT_MASK));
		dwt_writesysstatuslo(SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
	}
}

uint32_t sit_getRegStatus(){ // TODO rename to get Divice Status 
	return dwt_readsysstatuslo();
}


/*! --------------------------------------------------------------------------
 * @fn get_tx_timestamp_u64()
 *
 * @brief Get the TX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, 
 *            for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
uint64_t get_tx_timestamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;

	dwt_readtxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

/*! --------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits,
 *        for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
uint64_t get_rx_timestamp_u64(void)
{
	uint8_t ts_tab[5];
	uint64_t ts = 0;
	int i;

	dwt_readrxtimestamp(ts_tab);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

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
 *               0 - The function will not wait for any bits in the system status register (lower 32 bits).
 *
 * return None
 */
void waitforsysstatus(uint32_t *lo_result, uint32_t *hi_result, uint32_t lo_mask, uint32_t hi_mask)
{
	uint32_t lo_result_tmp = 0;
	uint32_t hi_result_tmp = 0;

	// If a mask has been passed into the function for the system status register (lower 32-bits)
	if (lo_mask)
	{
		while (!((lo_result_tmp = dwt_readsysstatuslo()) & (lo_mask)))
		{
			// If a mask value is set for the system status register (higher 32-bits)
			if (hi_mask)
			{
				// If mask value for the system status register (higher 32-bits) is found
				if ((hi_result_tmp = dwt_readsysstatushi()) & hi_mask)
				{
					break;
				}
			}
		}
	}
	// if only a mask value for the system status register (higher 32-bits) is set
	else if (hi_mask)
	{
		while (!((hi_result_tmp = dwt_readsysstatushi()) & (hi_mask))) { };
	}

	if (lo_result != NULL)
	{
		*lo_result = lo_result_tmp;
	}

	if (hi_result != NULL)
	{
		*hi_result = hi_result_tmp;
	}
}
