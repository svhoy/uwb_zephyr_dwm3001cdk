/*
 *  ble_init.h
 */
#ifndef __BLE_INIT_H__
#define __BLE_INIT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ble_device.h"
#include "ble_uuids.h" 

/***************************************************************************
* Initilization for BLE  
*
* @return error code 
* [
*	0 -> init ok,
*	err -> init failed
* ]
****************************************************************************/
uint8_t sit_ble_init(void);


bool is_connected(void);
void ble_sit_notify(float pos);
int ble_get_command();
void bas_notify(void);

int  ble_start_advertising(void);
int  ble_stop_advertising(void);
int  ble_disconnect(void);


#ifdef __cplusplus
}
#endif

#endif  // __BLE_INIT_H__