/* 
 *  DWM1001 ble_init.c 
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#define LOG_LEVEL 3
#include <logging/log.h>
LOG_MODULE_REGISTER(BLE_INIT, LOG_LEVEL_WRN);

#include "drivers/sit_ble/ble_init.h"
#include "drivers/sit_ble/cts.h"

#define POS_MAX_LEN 20


bool connection_status = false;

static uint8_t sit_position = 100U;

static struct bt_uuid_128 sit_uuid = BT_UUID_INIT_128(
	BT_UUID_DWM1001_SERVICE_VAL);

static struct bt_uuid_128 sit_notify_uuid = BT_UUID_INIT_128(
	BT_UUID_DWM1001_NOTIFY_VAL);

static struct bt_uuid_128 sit_command_uuid = BT_UUID_INIT_128(
	BT_UUID_DWM1001_COMMAND_VAL);

static ssize_t read_pos(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	uint8_t pos = sit_position;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &pos,
				 sizeof(pos));
}

static ssize_t write_pos(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > POS_MAX_LEN) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	value[offset + len] = 0;

	return len;
}

static void sit_pos_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				       uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("SIT Notifications %s", notif_enabled ? "enabled" : "disabled");
}

/* Vendor Primary Service Declaration */
BT_GATT_SERVICE_DEFINE(sit_service,
	BT_GATT_PRIMARY_SERVICE(&sit_uuid),
	BT_GATT_CHARACTERISTIC(&sit_notify_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ , 
				   read_pos, write_pos, &sit_position),
	BT_GATT_CCC(sit_pos_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(&sit_command_uuid.uuid,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ |
			       BT_GATT_PERM_WRITE,
			       read_pos, write_pos, &sit_position),
			
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, DWM1001_UUID_BASE, DWM1001_UUID_SERVICE)
};



static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
		connection_status = false;
	} else {
		printk("Connected\n");
		connection_status = true;
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	connection_status = false;
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

	printk("Bluetooth initialized\n");

	ble_set_device_name("DWM1001");
	ble_device_name();

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}
	
	ble_start_advertising();
}

int ble_start_advertising(){
	int err;

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return err;
	}

	printk("Advertising successfully started\n");

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

void ble_sit_notify(float pos) {	
	bt_gatt_notify(NULL, &sit_service.attrs[1], &pos, sizeof(pos));
}

void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

void ble_init(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_ready();

	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);
}