#include "Potenciometer.h"

float PotenciometerRead(adc1_channel_t adc_channel){
    int sum = 0;
    float potenciometerValue = 0;
    for (int i = 0; i < N_SAMPLES_CONVER; i++)
        sum += adc1_get_raw(adc_channel);
    potenciometerValue = (float) sum / N_SAMPLES_CONVER;
    return CONV_FACTOR * (potenciometerValue - MIN_POT_BIT) + MIN_POS_DEGREES;
}