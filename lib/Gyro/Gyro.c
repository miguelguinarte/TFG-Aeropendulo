#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "Gyro.h"
#include "I2c.h"
#include <math.h>
#include "MedianFilter.h"

static float angle;
static float angularVel;


void GyroSet(){
    u_int8_t write_buffer[2];
    write_buffer[0] = GYRO_CONTROL_REG;
    write_buffer[1] = ODR_6K66HZ_1000DPS; 
    i2c_master_write_to_device(I2C_NUM_0,ISM330_SENSOR_ADDR,write_buffer,2,I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}


float GyroGetAngularVel(){
    u_int8_t y_axis;
    y_axis = GYRO_Y_OUT_L;
    uint8_t buffer[2] = {0,0};
    int16_t raw;
    i2c_master_write_read_device(I2C_NUM_0,ISM330_SENSOR_ADDR,&y_axis,1,&buffer[0],2,I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    raw = (int16_t) ((buffer[1] << 8) | buffer[0]);
    //angularVel = MedianFilter(raw * GYRO_CONV_DEG_S_1000);
    angularVel = (-1)* raw * GYRO_CONV_DEG_S_1000; //axis does not match  in gyro/acc, you need *(-1)
    //printf("Gyro eje x: %f º/s\n",result);
    //printf("%f\n",angularVel);100e-3
    return angularVel;

}

float GyroGetAngle(){
    angle += (angularVel- ANGULAR_VEL_OFFSET) * DT ;
    angle = fmod(angle,360.0);
    return angle;
}
