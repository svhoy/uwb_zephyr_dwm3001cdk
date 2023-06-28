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
 * @file sit_ble_init.c
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Init for BLE System in SIT.
 *
 * This file defines functions for distance measurement for the 
 * DWM3001cdk in the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <sit/sit.h>
#include <sit_json/sit_json.h>

#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BLE_INIT, LOG_LEVEL_INF);

#include "sit_ble/ble_init.h"
#include "sit_ble/cts.h"

#define POS_MAX_LEN 20

struct bt_conn *default_conn;
bool connection_status = false;

static int sit_position = 100U;

static struct bt_uuid_128 sit_uuid = BT_UUID_INIT_128(
	BT_UUID_SIT_SERVICE_VAL);

static struct bt_uuid_128 sit_pos_notify_uuid = BT_UUID_INIT_128(
	BT_UUID_SIT_POS_NOTIFY_VAL);

static struct bt_uuid_128 sit_int_command_uuid = BT_UUID_INIT_128(
	BT_UUID_SIT_INT_COMMAND_VAL);

static struct bt_uuid_128 sit_json_command_uuid = BT_UUID_INIT_128(
	BT_UUID_SIT_JSON_COMMAND_VAL);

static ssize_t write_int_comand(
		struct bt_conn *conn,
		const struct bt_gatt_attr *attr,
		const void *buf,
		uint16_t len,
		uint16_t offset,
		uint8_t flags
	) {
	const uint8_t *value = buf;

	if (offset + len > POS_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (*value >= 0 && *value <= 10) {
		if(*value == 5 ){
			device_settings.state = measurement;
		} else if (*value == 0){ 
			device_settings.state = sleep;
		}
	} else {
		return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
	}
	return len;
}

static ssize_t write_json_comand(
		struct bt_conn *conn,
		const struct bt_gatt_attr *attr,
		const void *buf,
		uint16_t len,
		uint16_t offset,
		uint8_t flags
	) {
	char *value = malloc(len + 1);
	json_command_msg_t command_str;

	memcpy(value, buf, len);
	value[len+1] = '\0';
	int ret = json_decode_state_msg(value, &command_str);
	if (ret < 0) {
		LOG_ERR("JSON Parse Error: %d", ret);
	} else {
		if (strcmp(command_str.type, "measurement_msg") == 0 ){
			if(strcmp(command_str.command, "start")) {
				reset_sequence();
			}
			set_device_state(command_str.command);
		} else {
			LOG_ERR("Command: %s", command_str.type);
		}
	}
	free(value);
	return len;
}

static void sit_pos_ccc_cfg_changed(
		const struct bt_gatt_attr *attr,
		uint16_t value
	) {
	ARG_UNUSED(attr);
	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("SIT Notifications %s", notif_enabled ? "enabled" : "disabled");
}

/* SIT Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(sit_service,
	BT_GATT_PRIMARY_SERVICE(&sit_uuid),
	BT_GATT_CHARACTERISTIC(&sit_pos_notify_uuid.uuid,
			       BT_GATT_CHRC_NOTIFY,
				   BT_GATT_PERM_READ,
				   NULL, NULL, &sit_position),
	BT_GATT_CCC(sit_pos_ccc_cfg_changed,
		    	BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(&sit_int_command_uuid.uuid,
			       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE,
			       NULL, write_int_comand, NULL),
	BT_GATT_CHARACTERISTIC(&sit_json_command_uuid.uuid,
			       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE,
			       NULL, write_json_comand, NULL),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, SIT_UUID_BASE, SIT_UUID_SERVICE)
};



static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err 0x%02x)\n", err);
		connection_status = false;
		
		bt_conn_unref(default_conn);
		default_conn = NULL;
	} else {
		LOG_DBG("Connected");
		connection_status = true;
		default_conn = bt_conn_ref(conn);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	connection_status = false;
	if (default_conn){
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}
} 

bool is_connected(void) {
	return connection_status;
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	LOG_INF("Bluetooth initialized\n");
	char *deviceID;
	get_device_id(&deviceID);
	ble_set_device_name(deviceID);
	ble_device_name();
	// ble_device_address();

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}
}

int ble_start_advertising(){
	int err;

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return err;
	}

	LOG_INF("Advertising successfully started\n");

	return 0;
} 

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
};

void ble_sit_notify(json_distance_msg_t *json_data, size_t data_len) {
	bt_gatt_notify(NULL, &sit_service.attrs[1], json_data, data_len);
}

void bas_notify(void) {
	uint8_t battery_level = bt_bas_get_battery_level();
	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}
	bt_bas_set_battery_level(battery_level);
}

uint8_t sit_ble_init(void){
	int err;
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	bt_ready();

	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);

	return 0;
}