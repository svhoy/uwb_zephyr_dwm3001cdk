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

bool is_connected(void);
void ble_sit_notify(float pos);
void bas_notify(void);

int  ble_start_advertising(void);
int  ble_stop_advertising(void);
int  ble_disconnect(void);
void  ble_init(void);

#ifdef __cplusplus
}
#endif

#endif  // __BLE_INIT_H__