#ifndef POTENCIOMETER_H
#define PIPOTENCIOMETER_HNS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"

#define MIN_POT_BIT         (535)
#define MAX_POT_BIT         (2055)
#define N_SAMPLES_CONVER    (5)
#define MIN_POS_DEGREES     (-90)
#define MAX_POS_DEGREES     (0)
#define CONV_FACTOR         (float)(MAX_POS_DEGREES - MIN_POS_DEGREES) / (MAX_POT_BIT - MIN_POT_BIT)
#define PIN_POT             (ADC1_CHANNEL_4)

float PotenciometerRead(adc1_channel_t adc_channel);

#ifdef __cplusplus
}
#endif

#endif