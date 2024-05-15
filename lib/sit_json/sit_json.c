#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sit_json/sit_json.h"
#include <cJSON/cJSON.h>

#include <zephyr/kernel.h>
#include <zephyr/data/json.h>


#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_JSON, LOG_LEVEL_INF);



int json_decode_distance(char *json, size_t len, void *val) {
    int ret = 5;
    return ret;
}

int json_encode_distance(char **json, double *distance){
    cJSON *json_object = cJSON_CreateObject();
    if (cJSON_AddStringToObject(json_object, "type", "distance_msg") == NULL) {
        return -1;
    }
    if (cJSON_AddNumberToObject(json_object, "distance", *distance) == NULL) {
        return -2;
    }
    char* jsonString = cJSON_Print(json_object);
    size_t jsonStringLength = strlen(jsonString);
    *json = (char*)malloc((jsonStringLength + 1) * sizeof(char));
    if (*json == NULL) {
        k_free(jsonString);
        return -3;
    }

    strcpy(*json, jsonString);

    cJSON_Delete(json_object);
    k_free(jsonString);
    return 0;
}

int json_decode_state_msg(char *json, json_command_msg_t *command_struct) {

    const cJSON *type = NULL;
    const cJSON *command = NULL;
    cJSON *json_msg = cJSON_Parse(json);
    if (json_msg == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_ERR("Error before: %s\n", error_ptr);
        }
        return -1;
    }

    type = cJSON_GetObjectItemCaseSensitive(json_msg, "type");
    if (cJSON_IsString(type)) {
        LOG_INF("Checking msg type \"%s\"\n", type->valuestring);
        strcpy(command_struct->type, type->valuestring);
    } else {
        LOG_ERR("Something wrong");
    }

    command = cJSON_GetObjectItemCaseSensitive(json_msg, "command");
    if (cJSON_IsString(command) && (command->valuestring != NULL)) {
        LOG_INF("Checking command \"%s\"\n", command->valuestring);
        strcpy(command_struct->command, command->valuestring);
    } else {
        LOG_ERR("Something wrong");
    }

    LOG_INF("Type: %s", type->valuestring);
    cJSON_Delete(json_msg);

    return 0;
}

int json_decode_setup_msg(char *json, json_setup_msg_t *setup_struct) {

    const cJSON *type = NULL;
    const cJSON *device_type = NULL;
    const cJSON *initiator_device = NULL;
    const cJSON *initiator = NULL;
    const cJSON *responder_list = NULL;
    const cJSON *responder_device = NULL;
    const cJSON *responder = NULL;
    const cJSON *min_measurement = NULL;
    const cJSON *max_measurement = NULL;
    const cJSON *measurement_type = NULL;
    const cJSON *rx_ant_dly = NULL;
    const cJSON *tx_ant_dly = NULL;
    cJSON *json_msg = cJSON_Parse(json);
    if (json_msg == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_ERR("Error before: %s\n", error_ptr);
        }
        return -1;
    }

    type = cJSON_GetObjectItemCaseSensitive(json_msg, "type");
    if (cJSON_IsString(type)) {
        LOG_INF("Checking msg type \"%s\"\n", type->valuestring);
        strcpy(setup_struct->type, type->valuestring);
    } else {
        LOG_ERR("Something with msg type wrong");
    }

    device_type = cJSON_GetObjectItemCaseSensitive(json_msg, "device_type");
    if (cJSON_IsString(device_type)) {
        LOG_INF("Checking msg device_type \"%s\"\n", device_type->valuestring);
        strcpy(setup_struct->device_type, device_type->valuestring);
    } else {
        LOG_ERR("Something with device type wrong");
    }

    initiator_device = cJSON_GetObjectItemCaseSensitive(json_msg, "initiator_device");
    if (cJSON_IsString(initiator_device) && (initiator_device->valuestring != NULL)) {
        LOG_INF("Checking initiator_device \"%s\"\n", initiator_device->valuestring);
        strcpy(setup_struct->initiator_device, initiator_device->valuestring);
    } else {
        LOG_ERR("Something init device wrong");
    }

    initiator = cJSON_GetObjectItemCaseSensitive(json_msg, "initiator");
    setup_struct->initiator = initiator->valueint;

    responder_list = cJSON_GetObjectItemCaseSensitive(json_msg, "responder_device");
    uint8_t index = 0;
    cJSON_ArrayForEach(responder_device, responder_list) {
        if (cJSON_IsString(responder_device) && (responder_device->valuestring != NULL)) {
            LOG_INF("Checking responder_device \"%s\"\n", responder_device->valuestring);
            strcpy(setup_struct->responder_device[index], responder_device->valuestring);
        } else {
            LOG_ERR("Something resp devicewrong");
        }
        index++;
    }

    responder = cJSON_GetObjectItemCaseSensitive(json_msg, "responder");
    setup_struct->responder = responder->valueint;
    
    min_measurement = cJSON_GetObjectItemCaseSensitive(json_msg, "min_measurement");
    setup_struct->min_measurement = min_measurement->valueint;

    max_measurement = cJSON_GetObjectItemCaseSensitive(json_msg, "max_measurement");
    setup_struct->max_measurement = max_measurement->valueint;

    measurement_type = cJSON_GetObjectItemCaseSensitive(json_msg, "measurement_type");
    if (cJSON_IsString(measurement_type)) {
        LOG_INF("Checking Measurement Type \"%s\"\n", measurement_type->valuestring);
        strcpy(setup_struct->measurement_type, measurement_type->valuestring);
    } else {
        LOG_ERR("Something measurement type wrong");
    }

    rx_ant_dly = cJSON_GetObjectItemCaseSensitive(json_msg, "rx_ant_dly");
    setup_struct->rx_ant_dly = rx_ant_dly->valueint;
    LOG_INF("Checking RX DLY Type \"%d\"\n", rx_ant_dly->valueint);
    tx_ant_dly = cJSON_GetObjectItemCaseSensitive(json_msg, "tx_ant_dly");
    LOG_INF("Checking RX DLY Type \"%d\"\n", tx_ant_dly->valueint);
    setup_struct->tx_ant_dly = tx_ant_dly->valueint;

    LOG_INF("Type: %s", type->valuestring);
    cJSON_Delete(json_msg);

    return 0;
}