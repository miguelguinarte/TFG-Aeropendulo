#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "I2c.h"
#include "Accelerometer.h"
#include "Gyro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void I2cInialitation(){
    i2c_config_t conf = {
	    	.mode = I2C_MODE_MASTER,
		    .sda_io_num = 21,
    		.scl_io_num = 22,
	    	.sda_pullup_en = GPIO_PULLUP_ENABLE,
		    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    		.master.clk_speed = 400000,
	    };

    i2c_param_config(I2C_NUM_0, &conf);
    //ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}
