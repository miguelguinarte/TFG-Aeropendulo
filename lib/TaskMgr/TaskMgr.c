#include <stdio.h>
#include "TaskMgr.h"
#include "I2c.h"
#include "Accelerometer.h"
#include "Gyro.h"
#include "Timer.h"

void TaskMgrInit(){
    //I2cInialitation();
	GyroSet();
    AccelerometerSet();
    //TimerInit();
}

void TaskMgr(){

}