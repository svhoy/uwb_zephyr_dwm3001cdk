/*
 *  ble_device.h
 */
#ifndef __BLE_DEVICE_H__
#define __BLE_DEVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include "ble_init.h"

int ble_set_device_name(const char *name);
void ble_device_name(void);
void ble_device_address(void);
void ble_device_init(void);

#ifdef __cplusplus
}
#endif


#endif //__BLE_DEVICE_H__