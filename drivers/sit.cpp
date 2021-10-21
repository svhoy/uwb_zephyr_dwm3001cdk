#include "drivers/sit.h"

#include <string.h>

#include <logging/log.h>
#include <sys/printk.h>

#include "drivers/dw1000/deca_device_api.h"
#include "drivers/dw1000/deca_regs.h"
#include "drivers/dw1000/deca_spi.h"
#include "drivers/dw1000/port.h"

LOG_MODULE_REGISTER(SIT_Module, LOG_LEVEL_WRN);

static uint32_t status_reg = 0;

bool sit_init(dwt_config_t config) {
    /* Configure DW1000 SPI */
    openspi();

    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        LOG_WRN("dwt_initialise failed");
        return false;
    }
	port_set_dw1000_fastrate();
    dwt_configure(&config);
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

	return true;
}

void sit_setRxAfterTxDelay(uint32_t delay_us, uint16_t timeout) {
	dwt_setrxaftertxdelay(delay_us);
    dwt_setrxtimeout(timeout);
}

void sit_startPoll(uint8_t* msg_data, uint16_t msg_size){
    dwt_writetxdata(msg_size, msg_data, 0); // 0 offset
	dwt_writetxfctrl(msg_size, 0, 1);//ranging bit unused by DW1000
	dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);//switch to rx after `setrxaftertxdelay`
}

bool sit_sendAt(uint8_t* msg_data, uint16_t size, uint64_t tx_time){

	uint32_t reg1 = dwt_read32bitreg(SYS_STATUS_ID);
	dwt_setdelayedtrxtime(tx_time);
	dwt_writetxdata(size, msg_data, 0); 
	dwt_writetxfctrl(size, 0, 1); 
	bool late = dwt_starttx(DWT_START_TX_DELAYED);
	if(late == DWT_SUCCESS) {
		while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
		{
		};
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
		LOG_WRN("Send Success");
		return true;
	} else {
		uint32_t reg2 = dwt_read32bitreg(SYS_STATUS_ID);
		recover_tx_errors();
		status_reg = dwt_read32bitreg(SYS_STATUS_ID);
		LOG_WRN("sit_sendAt() - dwt_starttx() late");
		LOG_WRN("reg1 = 0x%08x ; reg2 = 0x%08x; regnow = 0x%08x",reg1,reg2,status_reg);
		return false;
	}
}

uint32_t sit_poll_receive() {
	uint32_t l_status_reg;
	while (!((l_status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))	{
	};
	return l_status_reg;
}

void sit_receiveNow(uint16_t timeout) {
    dwt_setrxtimeout(timeout); // 0 : disable timeout
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

bool sit_checkReceivedMessage(uint8_t* data, uint16_t expected_size) {
	bool result = false;
	status_reg = sit_poll_receive();
	if(status_reg & SYS_STATUS_RXFCG){
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);//clearing any rx and tx
		uint16_t size = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
		if (size == expected_size) {
			dwt_readrxdata(data, size, 0);
			result = true;
		}else{
			LOG_ERR("rx size %u != expected_size %u",size,expected_size);
		}
	}else{
		dwt_write32bitreg(SYS_STATUS_ID,SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);//clear errors
		dwt_rxreset();
		LOG_WRN("sit_checkReceivedMessage() no 'RX Frame Checksum Good'");
		uint32_t reg2 = sit_getRegStatus();
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
	uint32_t status = dwt_read32bitreg(SYS_STATUS_ID);
	if(status & SYS_STATUS_TXERR)
	{
		LOG_INF("recovering TX errors 0x%08x", (status & SYS_STATUS_TXERR));
		dwt_write8bitoffsetreg(SYS_CTRL_ID, SYS_CTRL_OFFSET, (uint8)SYS_CTRL_TRXOFF);
	}
}

uint32_t sit_getRegStatus(){
    return dwt_read32bitreg(SYS_STATUS_ID);
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