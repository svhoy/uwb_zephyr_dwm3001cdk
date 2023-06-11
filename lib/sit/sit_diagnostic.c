#include <math.h>

#include "sit/sit_diagnostic.h"
#include "sit/sit_config.h"
#include <deca_device_api.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(SIT_DIAGNOSTIC, LOG_LEVEL_INF);

#define A_PRF_16       113.8    // Constant A for PRF of 16 MHz. See User Manual for more information.
#define A_PRF_64       120.7    // Constant A for PRF of 64 MHz. See User Manual for more information.
#define RX_CODE_THRESHOLD  8    // For 64 MHz PRF the RX code is 9.
#define LOG_CONSTANT_C0    63.2 // 10log10(2^21) = 63.2 See User Manual for more information.


#define ACCUM_DATA_LEN (4)
static uint8_t accum_data[ACCUM_DATA_LEN];

extern dwt_config_t sit_device_config;

static dwt_rxdiag_t rx_diag;

dwt_nlos_alldiag_t diag_data;
dwt_nlos_ipdiag_t fp_pp_index; 

void get_fp_pp_index(void) {
    dwt_nlos_ipdiag(&fp_pp_index);
    dwt_readdiagnostics(&rx_diag);
    uint16_t fp_int = rx_diag.ipatovFpIndex >> 6;
    uint8_t dgc_decision = dwt_get_dgcdecision();
    uint32_t fp_index = fp_pp_index.index_fp_u32 >> 6;
    uint32_t pp_index = fp_pp_index.index_pp_u32 >> 6;
    dwt_readaccdata(accum_data, ACCUM_DATA_LEN, (fp_index - 2));
    int32_t acc_value = ((int32_t)accum_data[3] << 24) | ((int32_t)accum_data[2] << 16) | ((int32_t)accum_data[1] << 8) | accum_data[0];
    LOG_INF("DGC: %u", dgc_decision);
    LOG_INF("FP: %u", fp_index);
    LOG_INF("PP: %u", fp_int);
}

void get_diagnostic(diagnostic_info *diagnostic) {
    uint8_t D;
    float ip_f1, ip_f2, ip_f3, ip_n, ip_cp, ip_rsl, ip_fsl;
    float ip_alpha, log_constant = 0;

    log_constant = LOG_CONSTANT_C0;

    diag_data.diag_type = IPATOV;
    dwt_nlos_alldiag(&diag_data);
    if (sit_device_config.rxCode > RX_CODE_THRESHOLD){
        ip_alpha = (-(A_PRF_64 + 1));
    } else {
        ip_alpha = -(A_PRF_16);
    }
    ip_n = diag_data.accumCount; // The number of preamble symbols accumulated
    ip_f1 = diag_data.F1 / 4;    // The First Path Amplitude (point 1) magnitude value (it has 2 fractional bits),
    ip_f2 = diag_data.F2 / 4;    // The First Path Amplitude (point 2) magnitude value (it has 2 fractional bits),
    ip_f3 = diag_data.F3 / 4;    // The First Path Amplitude (point 3) magnitude value (it has 2 fractional bits),
    ip_cp = diag_data.cir_power;
    D = diag_data.D * 6;

    // Quadrate bilden
    ip_n *= ip_n;
    ip_f1 *= ip_f1;
    ip_f2 *= ip_f2;
    ip_f3 *= ip_f3;

    // Berechnung der First Path Power und Recived Signal Power (RSSI).
    ip_rsl = 10 * log10((float)ip_cp / ip_n) + ip_alpha + log_constant + D;
    ip_fsl = 10 * log10(((ip_f1 + ip_f2 + ip_f3) / ip_n)) + ip_alpha + D;

    LOG_INF("Recived Index: %f", ip_rsl);
    LOG_INF("First Path Index: %f", ip_fsl);

    // If differenc is bigger than 12 db the singal is Non Line of Sight
    if ((ip_rsl - ip_fsl) > 12 ) {
        LOG_INF("non line of sight"); 
        diagnostic->nlos = 100;
    } else {
        LOG_INF("line of sight");
        diagnostic->nlos = 0;
    }

    diagnostic->fpi = ip_fsl;
    diagnostic->rssi = ip_rsl;
}