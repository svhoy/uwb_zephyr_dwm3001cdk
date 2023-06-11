/* 
 *  sit_mesh.c 
 */

#include <zephyr/types.h>
#include <sys/printk.h>
#include <logging/log.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>
#include <drivers/hwinfo.h>

#include <sit_led.h>

#include "board.h"
#include "sit_mesh.h"

LOG_MODULE_REGISTER(sit_mesh, LOG_LEVEL_INF);

static void attention_on(struct bt_mesh_model *mod)
{
	sit_set_led(1, 1);
}

static void attention_off(struct bt_mesh_model *mod)
{
	sit_set_led(1, 0);
}


static const struct bt_mesh_health_srv_cb health_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_model models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = BT_COMP_ID_LF,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static void mesh_blink(uint32_t num) {
	printk("Test: %d", num);

}

static void output_number(bt_mesh_output_action_t act, uint32_t num) {
	if(act == BT_MESH_DISPLAY_NUMBER) {
		printk("Test 1");
		printk("Output OOB Number: %u \n", num);
	} else if (act == BT_MESH_BLINK){
		LOG_INF("Output OOB Number: %u \n", num);
		for(int i = 1; i <= num; i++) {
			k_msleep(200);
			sit_set_led(1, 1);
			k_msleep(200);
			sit_set_led(1, 0);
		}
	}
	printk("Test3");
}

static void prov_complete(uint16_t net_idx, uint16_t addr) {
	printk("Prov Complete \n");
}

static void prov_reset(void)
{
	bt_mesh_prov_enable( BT_MESH_PROV_GATT | BT_MESH_PROV_ADV );
}

static void new_noded_added(uint16_t net_idx, uint8_t uuid[16], uint16_t addr, uint8_t num_elem) {
	printk("New Element: %d, %02x, %d, %d \n", net_idx, uuid, addr, num_elem);
}

static uint8_t dev_uuid[16];


static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 1,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = output_number,
	.complete = prov_complete,
	.node_added = new_noded_added,
	.reset = prov_reset,
};


int sit_set_device_name(const char * name)
{
    int err;

    bt_set_name(name);
    if(err < 0) {
        LOG_ERR("Name not set: %d", err);
		return err;
    }

    return 0;
}


static void bt_ready(int err) {
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");
	sit_set_device_name("DWM3001");

	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	bt_mesh_prov_enable( BT_MESH_PROV_GATT | BT_MESH_PROV_ADV );
	printk("Mesh initialized\n");
}

bool sit_mesh_is_provisioned(void) {
	return bt_mesh_is_provisioned();
}


void sit_mesh_prov_init(void) {

}

void sit_mesh_init(void) {
	int err = -1; 

	printk("Init Sit MESH \n");

	if (IS_ENABLED(CONFIG_HWINFO)) {
		err = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	}

	if (err < 0) {
		dev_uuid[0] = 0xc7;
		dev_uuid[1] = 0x13;
		dev_uuid[2] = 0x03;
		dev_uuid[3] = 0x0e;
		dev_uuid[4] = 0xb8;
		dev_uuid[5] = 0x7d;
		dev_uuid[6] = 0x4b;
		dev_uuid[7] = 0x90;
		dev_uuid[8] = 0xb5;
		dev_uuid[9] = 0xe7;
		dev_uuid[10] = 0x0a;
		dev_uuid[11] = 0x51;
		dev_uuid[12] = 0xe8;
		dev_uuid[13] = 0xaa;
		dev_uuid[14] = 0x8e;
		dev_uuid[15] = 0x66;
	}
	printk("UUID: %16x \n", dev_uuid);
	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}
