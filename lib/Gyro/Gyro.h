 #ifndef GYRO_H
 #define GYRO_H

 #ifdef __cplusplus
extern "C" {
#endif

#define ISM330_SENSOR_ADDR      (0x6A)
#define GYRO_CONTROL_REG        (0x11)
#define ODR_104HZ_2000DPS       (0x4C)
#define ODR_416HZ_1000DPS       (0x68)
#define ODR_416HZ_2000DPS       (0x6C)
#define ODR_833HZ_1000DPS       (0x78)
#define ODR_833HZ_2000DPS       (0x7C)
#define ODR_1K66HZ_1000DPS      (0x88)
#define ODR_1K66HZ_2000DPS      (0x8C)
#define ODR_1K66HZ_500DPS       (0x84)
#define ODR_3K33HZ_1000DPS      (0x98)
#define ODR_3K33HZ_2000DPS      (0x9C)
#define ODR_6K66HZ_1000DPS      (0xA8)
#define ODR_6K66HZ_2000DPS      (0xAC)
#define ODR_6K66HZ_4000DPS      (0xA1)
#define GYRO_X_OUT_L            (0x22) 
#define GYRO_X_OUT_H            (0x23) 
#define GYRO_Y_OUT_L            (0x24) 
#define GYRO_Y_OUT_H            (0x25) 
#define GYRO_Z_OUT_L            (0x26)
#define GYRO_Z_OUT_H            (0x27) 
#define GYRO_CONV_DEG_S_2000    (70.0/1000.0F)
#define GYRO_CONV_DEG_S_1000    (35/1000.0F)
#define GYRO_CONV_DEG_S_500     (17.5/1000.0F)
#define GYRO_CONV_DEG_S_250     (8.75/1000.0F)
#define DT                      (3e-3)
#define ANGULAR_VEL_OFFSET      (-0.1928)


void GyroSet();
float GyroGetAngularVel();
float GyroGetAngle();

#ifdef __cplusplus
}
#endif


 #endif