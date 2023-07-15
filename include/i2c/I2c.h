#ifndef I2C_H
#define I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#define ACK_EN                  (1)
#define I2C_MASTER_TIMEOUT_MS   (1000)

void I2cInialitation();

#ifdef __cplusplus
}
#endif

#endif