#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#define ISM330_SENSOR_ADDR  (0x6A)
#define ACC_CONTROL_REG     (0x10)
#define ODR_416HZ_2G        (0x60)
#define ODR_833HZ_2G        (0x70)
#define ODR_1K66HZ_2G       (0x80)
#define ODR_3K33HZ_2G       (0x90)
#define ODR_6K6HZ_2G        (0xA0)
#define ODR_1K66HZ_4G       (0x88)
#define ACC_X_OUT_L         (0x28)
#define ACC_X_OUT_H         (0x29)
#define ACC_Y_OUT_L         (0x2A)
#define ACC_Y_OUT_H         (0x2B)
#define ACC_Z_OUT_L         (0x2C)
#define ACC_Z_OUT_H         (0x2D)
#define ACC_GRAVITY_EARTH   (9.80665F)
#define ACC_CONV_M_S2       (0.061*ACC_GRAVITY_EARTH/1000.0)
#define PI                  (3.141592) 
#define ANGLE_OFFSET       (-1.4987)


void AccelerometerSet();
void AccelerometerRead();
float AccelerometerGetAngle();

#ifdef __cplusplus
}
#endif


#endif