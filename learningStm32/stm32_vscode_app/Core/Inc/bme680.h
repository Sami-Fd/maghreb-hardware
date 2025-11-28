#ifndef BME680_H
#define BME680_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

// BME680 I2C Address
// SDO = GND -> 0x76
// SDO = VDD -> 0x77
#define BME680_I2C_ADDR_PRIMARY 0x76
#define BME680_I2C_ADDR_SECONDARY 0x77
#define BME680_I2C_ADDR (BME680_I2C_ADDR_SECONDARY << 1)

// Register Addresses
#define BME680_REG_CHIP_ID 0xD0
#define BME680_REG_RESET 0xE0
#define BME680_REG_CTRL_HUM 0x72
#define BME680_REG_CTRL_MEAS 0x74
#define BME680_REG_CONFIG 0x75
#define BME680_REG_CTRL_GAS_1 0x71
#define BME680_REG_CTRL_GAS_0 0x70
#define BME680_REG_GAS_WAIT_0 0x64
#define BME680_REG_RES_HEAT_0 0x5A
#define BME680_REG_IDAC_HEAT_0 0x50
#define BME680_REG_GAS_R_LSB 0x2B
#define BME680_REG_GAS_R_MSB 0x2A
#define BME680_REG_HUM_LSB 0x26
#define BME680_REG_HUM_MSB 0x25
#define BME680_REG_TEMP_XLSB 0x24
#define BME680_REG_TEMP_LSB 0x23
#define BME680_REG_TEMP_MSB 0x22
#define BME680_REG_PRESS_XLSB 0x21
#define BME680_REG_PRESS_LSB 0x20
#define BME680_REG_PRESS_MSB 0x1F

// Chip ID
#define BME680_CHIP_ID 0x61

// Calibration Data Structure
typedef struct {
  uint16_t par_t1;
  int16_t par_t2;
  int8_t par_t3;
  uint16_t par_p1;
  int16_t par_p2;
  int8_t par_p3;
  int16_t par_p4;
  int16_t par_p5;
  int8_t par_p6;
  int8_t par_p7;
  int16_t par_p8;
  int16_t par_p9;
  uint8_t par_p10;
  uint16_t par_h1;
  uint16_t par_h2;
  int8_t par_h3;
  int8_t par_h4;
  int8_t par_h5;
  uint8_t par_h6;
  int8_t par_h7;
  int8_t par_gh1;
  int16_t par_gh2;
  int8_t par_gh3;
} BME680_CalibData_t;

// Data Structure
typedef struct {
  float Temperature;   // deg C
  float Pressure;      // hPa
  float Humidity;      // % RH
  float GasResistance; // Ohms (Not fully implemented in basic driver)
} BME680_Data_t;

// Function Prototypes
uint8_t BME680_Init(I2C_HandleTypeDef *hi2c);
void BME680_ReadAll(I2C_HandleTypeDef *hi2c, BME680_Data_t *data);

#endif // BME680_H
