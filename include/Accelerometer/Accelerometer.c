#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "I2c.h"
#include "Accelerometer.h"
#include <math.h>


static float acc_x = 0;
static float acc_z = 0;

void AccelerometerSet(){
    u_int8_t write_buffer[2];
    write_buffer[0] = ACC_CONTROL_REG;
    write_buffer[1] = ODR_3K33HZ_2G; 
    i2c_master_write_to_device(I2C_NUM_0,ISM330_SENSOR_ADDR,write_buffer,2,I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

}

void AccelerometerRead(){

    u_int8_t x_axis = ACC_X_OUT_L;
    u_int8_t buffer[6] = {0,0,0,0,0,0};
    float result;  
    int16_t raw_x;
    int16_t raw_z;
    i2c_master_write_read_device(I2C_NUM_0,ISM330_SENSOR_ADDR,&x_axis,1,&buffer[0],6,I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

    raw_x = (int16_t) ((buffer[1] << 8) | buffer[0]);
    acc_x = raw_x * ACC_CONV_M_S2;
    raw_z = (int16_t) ((buffer[5] << 8) | buffer[4]);
    acc_z = raw_z *  ACC_CONV_M_S2;

    //printf("%f\t %f\n",acc_x,acc_z);

}

float AccelerometerGetAngle(){
    float ang_x;
    AccelerometerRead();
    ang_x = (float) atan(acc_x/acc_z) * 180.0/PI;
    ang_x -= ANGLE_OFFSET;
    return ang_x;
}