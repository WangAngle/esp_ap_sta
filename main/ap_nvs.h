#ifndef __AP_NVS_H__
#define __AP_NVS_H__

#include <stdint.h>

typedef struct 
{
    uint8_t mac[6];
    uint8_t ssid[32];
    uint8_t pass[64];
}ap_info_t;

typedef struct 
{
    uint8_t ssid[32];
    uint8_t pass[64];
}sta_info_t;

int get_ap_info(ap_info_t *ap_info);
int set_ap_info(ap_info_t *ap_info);

int get_sta_info(sta_info_t *sta_info);
int set_sta_info(sta_info_t *sta_info);

#endif