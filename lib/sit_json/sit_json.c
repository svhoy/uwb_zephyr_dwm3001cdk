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

// int json_encode_distance(char **json, float *distance){
//     char json_buf[128] = "{\"type\":\"measurement_msg\", \"distance\":\0";
//     char end_char[] = "}\0";
//     char number[10];
//     snprintf(number, 10, "%3.5lf", *distance);


//     strcat(json_buf, number); 
//     strcat(json_buf, end_char);
//     json_buf[strlen(json_buf)+1] = "\0";
//     LOG_INF("Test: %s", json_buf);
//     *json = k_malloc(strlen(json_buf)+1);
//     *json = strdup(json_buf);

//     return 0;
// }

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