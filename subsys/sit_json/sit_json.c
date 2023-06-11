#include "sit_json.h"

int json_distance_parser(char *json, size_t len, void *val) {
    int ret = json_obj_parse(json, 
                             len, 
                             distance_msg_descr, 
                             ARRAY_SIZE(distance_msg_descr),
                             val);
    return ret;
}