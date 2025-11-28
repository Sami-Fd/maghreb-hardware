#include "icm20948.h"

// Sensitivity scales (default settings)
#define ACCEL_SENSITIVITY_2G 16384.0f
#define GYRO_SENSITIVITY_250DPS 131.0f

void ICM20948_SelectBank(I2C_HandleTypeDef *hi2c, uint8_t bank) {
  uint8_t data = (bank << 4);
  HAL_I2C_Mem_Write(hi2c, ICM20948_I2C_ADDR, REG_BANK_SEL, 1, &data, 1, 100);
}

uint8_t ICM20948_Init(I2C_HandleTypeDef *hi2c) {
  uint8_t check;
  uint8_t data;

  // Select Bank 0
  ICM20948_SelectBank(hi2c, 0);
  HAL_Delay(10);

  // Check WHO_AM_I register
  HAL_I2C_Mem_Read(hi2c, ICM20948_I2C_ADDR, REG_WHO_AM_I, 1, &check, 1, 100);

  if (check == 0xEA) { // 0xEA is the default WHO_AM_I value for ICM-20948
    // Reset the device
    data = 0x80;
    HAL_I2C_Mem_Write(hi2c, ICM20948_I2C_ADDR, REG_PWR_MGMT_1, 1, &data, 1,
                      100);
    HAL_Delay(50);

    // Wake up the sensor (clear SLEEP bit) and select Auto Clock (0x01)
    data = 0x01;
    HAL_I2C_Mem_Write(hi2c, ICM20948_I2C_ADDR, REG_PWR_MGMT_1, 1, &data, 1,
                      100);
    HAL_Delay(50);

    return 0; // OK
  }
  return 1; // Error
}

void ICM20948_ReadAccel(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data) {
  uint8_t Rec_Data[6];

  // Select Bank 0
  ICM20948_SelectBank(hi2c, 0);

  // Read 6 bytes of data starting from ACCEL_XOUT_H
  if (HAL_I2C_Mem_Read(hi2c, ICM20948_I2C_ADDR, REG_ACCEL_XOUT_H, 1, Rec_Data,
                       6, 100) == HAL_OK) {
    data->Accel_X_Raw = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
    data->Accel_Y_Raw = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
    data->Accel_Z_Raw = (int16_t)(Rec_Data[4] << 8 | Rec_Data[5]);

    // Convert to physical units (g)
    data->Accel_X = data->Accel_X_Raw / ACCEL_SENSITIVITY_2G;
    data->Accel_Y = data->Accel_Y_Raw / ACCEL_SENSITIVITY_2G;
    data->Accel_Z = data->Accel_Z_Raw / ACCEL_SENSITIVITY_2G;
  }
}

void ICM20948_ReadGyro(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data) {
  uint8_t Rec_Data[6];

  // Select Bank 0
  ICM20948_SelectBank(hi2c, 0);

  // Read 6 bytes of data starting from GYRO_XOUT_H
  if (HAL_I2C_Mem_Read(hi2c, ICM20948_I2C_ADDR, REG_GYRO_XOUT_H, 1, Rec_Data, 6,
                       100) == HAL_OK) {
    data->Gyro_X_Raw = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
    data->Gyro_Y_Raw = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
    data->Gyro_Z_Raw = (int16_t)(Rec_Data[4] << 8 | Rec_Data[5]);

    // Convert to physical units (dps)
    data->Gyro_X = data->Gyro_X_Raw / GYRO_SENSITIVITY_250DPS;
    data->Gyro_Y = data->Gyro_Y_Raw / GYRO_SENSITIVITY_250DPS;
    data->Gyro_Z = data->Gyro_Z_Raw / GYRO_SENSITIVITY_250DPS;
  }
}

void ICM20948_ReadAll(I2C_HandleTypeDef *hi2c, ICM20948_Data_t *data) {
  ICM20948_ReadAccel(hi2c, data);
  ICM20948_ReadGyro(hi2c, data);
}
