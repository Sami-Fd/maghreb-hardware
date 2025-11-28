#ifndef ICM20948_H
#define ICM20948_H

#include "stm32l4xx_hal.h"

// ICM-20948 I2C Address (7-bit)
// AD0 = 0 -> 0x68
// AD0 = 1 -> 0x69
#define ICM20948_I2C_ADDR_0 0x68
#define ICM20948_I2C_ADDR_1 0x69
#define ICM20948_I2C_ADDR (ICM20948_I2C_ADDR_0 << 1)

// Register Map (Partial)
#define REG_WHO_AM_I 0x00
#define REG_USER_CTRL 0x03
#define REG_PWR_MGMT_1 0x06
#define REG_ACCEL_XOUT_H 0x2D
#define REG_GYRO_XOUT_H 0x33
#define REG_BANK_SEL 0x7F

// Structure to hold sensor data
typedef struct {
  int16_t Accel_X_Raw;
  int16_t Accel_Y_Raw;
  int16_t Accel_Z_Raw;
  int16_t Gyro_X_Raw;
  int16_t Gyro_Y_Raw;
  int16_t Gyro_Z_Raw;

  float Accel_X;
  float Accel_Y;
  float Accel_Z;
  float Gyro_X;
  float Gyro_Y;
  float Gyro_Z;
} ICM20948_Data_t;

// Function Prototypes
uint8_t ICM20948_Init(I2C_HandleTypeDef *hi2c);
void ICM20948_ReadAccel(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data);
void ICM20948_ReadGyro(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data);
void ICM20948_ReadAll(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data);

#endif // ICM20948_H
