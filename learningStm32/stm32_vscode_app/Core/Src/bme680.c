#include "bme680.h"

static BME680_CalibData_t calib;
static int32_t t_fine; // Global t_fine for compensation

// Helper to read calibration data
static void BME680_ReadCalibration(I2C_HandleTypeDef *hi2c) {
  uint8_t coeff_array1[25];
  uint8_t coeff_array2[16];

  // Read Calibration Data Range 1 (0x89 - 0xA1)
  HAL_I2C_Mem_Read(hi2c, BME680_I2C_ADDR, 0x89, 1, coeff_array1, 25, 100);
  // Read Calibration Data Range 2 (0xE1 - 0xF0)
  HAL_I2C_Mem_Read(hi2c, BME680_I2C_ADDR, 0xE1, 1, coeff_array2, 16, 100);

  calib.par_t1 = (uint16_t)(coeff_array2[8] << 8) | coeff_array2[9];
  calib.par_t2 = (int16_t)(coeff_array1[2] << 8) | coeff_array1[1];
  calib.par_t3 = (int8_t)coeff_array1[3];

  calib.par_p1 = (uint16_t)(coeff_array1[6] << 8) | coeff_array1[5];
  calib.par_p2 = (int16_t)(coeff_array1[8] << 8) | coeff_array1[7];
  calib.par_p3 = (int8_t)coeff_array1[9];
  calib.par_p4 = (int16_t)(coeff_array1[12] << 8) | coeff_array1[11];
  calib.par_p5 = (int16_t)(coeff_array1[14] << 8) | coeff_array1[13];
  calib.par_p6 = (int8_t)coeff_array1[16];
  calib.par_p7 = (int8_t)coeff_array1[15];
  calib.par_p8 = (int16_t)(coeff_array1[20] << 8) | coeff_array1[19];
  calib.par_p9 = (int16_t)(coeff_array1[22] << 8) | coeff_array1[21];
  calib.par_p10 = (uint8_t)coeff_array1[23];

  calib.par_h1 =
      (uint16_t)(((uint16_t)coeff_array2[2] << 4) | (coeff_array2[1] & 0x0F));
  calib.par_h2 = (uint16_t)(((uint16_t)coeff_array2[0] << 4) |
                            ((coeff_array2[1] >> 4) & 0x0F));
  calib.par_h3 = (int8_t)coeff_array2[3];
  calib.par_h4 = (int8_t)coeff_array2[4];
  calib.par_h5 = (int8_t)coeff_array2[5];
  calib.par_h6 = (uint8_t)coeff_array2[6];
  calib.par_h7 = (int8_t)coeff_array2[7];
}

uint8_t BME680_Init(I2C_HandleTypeDef *hi2c) {
  uint8_t id = 0;
  HAL_I2C_Mem_Read(hi2c, BME680_I2C_ADDR, BME680_REG_CHIP_ID, 1, &id, 1, 100);

  if (id != BME680_CHIP_ID) {
    return 1; // Error
  }

  // Soft Reset
  uint8_t reset_cmd = 0xB6;
  HAL_I2C_Mem_Write(hi2c, BME680_I2C_ADDR, BME680_REG_RESET, 1, &reset_cmd, 1,
                    100);
  HAL_Delay(100);

  BME680_ReadCalibration(hi2c);

  // Set Humidity Oversampling to x1
  uint8_t ctrl_hum = 0x01;
  HAL_I2C_Mem_Write(hi2c, BME680_I2C_ADDR, BME680_REG_CTRL_HUM, 1, &ctrl_hum, 1,
                    100);

  // Set Temp x1, Press x1, Mode Forced
  // osrs_t = 001 (x1) -> bit 7,6,5 = 001
  // osrs_p = 001 (x1) -> bit 4,3,2 = 001
  // mode = 00 (Sleep) -> bit 1,0 = 00 (Will set to forced in ReadAll)
  uint8_t ctrl_meas = (1 << 5) | (1 << 2) | 0;
  HAL_I2C_Mem_Write(hi2c, BME680_I2C_ADDR, BME680_REG_CTRL_MEAS, 1, &ctrl_meas,
                    1, 100);

  // Config: Filter coeff 3 (010) -> bit 4,3,2
  uint8_t config = (2 << 2);
  HAL_I2C_Mem_Write(hi2c, BME680_I2C_ADDR, BME680_REG_CONFIG, 1, &config, 1,
                    100);

  return 0; // OK
}

static float BME680_Compensate_Temperature(uint32_t temp_adc) {
  double var1, var2, calc_temp;
  var1 = (((double)temp_adc / 16384.0) - ((double)calib.par_t1 / 1024.0)) *
         ((double)calib.par_t2);
  var2 = ((((double)temp_adc / 131072.0) - ((double)calib.par_t1 / 8192.0)) *
          (((double)temp_adc / 131072.0) - ((double)calib.par_t1 / 8192.0))) *
         ((double)calib.par_t3 * 16.0);
  t_fine = (int32_t)(var1 + var2);
  calc_temp = ((var1 + var2) / 5120.0);
  return (float)calc_temp;
}

static float BME680_Compensate_Pressure(uint32_t pres_adc) {
  double var1, var2, var3, calc_pres;
  var1 = ((double)t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * (((double)calib.par_p6) / 131072.0);
  var2 = var2 + (var1 * ((double)calib.par_p5) * 2.0);
  var2 = (var2 / 4.0) + (((double)calib.par_p4) * 65536.0);
  var1 = ((((double)calib.par_p3 * var1 * var1) / 16384.0) +
          ((double)calib.par_p2 * var1)) /
         524288.0;
  var1 = (1.0 + (var1 / 32768.0)) * ((double)calib.par_p1);

  if (var1 == 0.0)
    return 0; // Avoid division by zero

  calc_pres = 1048576.0 - (double)pres_adc;
  calc_pres = (((calc_pres - (var2 / 4096.0)) * 6250.0) / var1);
  var1 = ((double)calib.par_p9 * calc_pres * calc_pres) / 2147483648.0;
  var2 = calc_pres * (((double)calib.par_p8) / 32768.0);
  var3 = ((calc_pres / 256.0) * (calc_pres / 256.0) * (calc_pres / 256.0) *
          (calib.par_p10 / 131072.0));
  calc_pres =
      calc_pres + (var1 + var2 + var3 + ((double)calib.par_p7 * 128.0)) / 16.0;

  return (float)(calc_pres / 100.0); // hPa
}

static float BME680_Compensate_Humidity(uint16_t hum_adc) {
  double var1, var2, var3, var4, var5, var6, calc_hum;
  var1 = ((double)t_fine) - 76800.0;
  var2 = (((double)calib.par_h4) * 64.0 +
          (((double)calib.par_h5) / 16384.0) * var1);
  var3 = hum_adc - var2;
  var4 = ((double)calib.par_h2) / 65536.0;
  var5 = (1.0 + (((double)calib.par_h3) / 67108864.0) * var1);
  var6 = 1.0 + (((double)calib.par_h6) / 67108864.0) * var1 * var5;
  var6 = var3 * var4 * (var5 * var6);
  calc_hum = var6 * (1.0 - ((double)calib.par_h1) * var6 / 524288.0);

  if (calc_hum > 100.0)
    calc_hum = 100.0;
  else if (calc_hum < 0.0)
    calc_hum = 0.0;

  return (float)calc_hum;
}

void BME680_ReadAll(I2C_HandleTypeDef *hi2c, BME680_Data_t *data) {
  // Trigger Forced Mode
  uint8_t ctrl_meas;
  HAL_I2C_Mem_Read(hi2c, BME680_I2C_ADDR, BME680_REG_CTRL_MEAS, 1, &ctrl_meas,
                   1, 100);
  ctrl_meas = (ctrl_meas & 0xFC) | 0x01; // Set mode to 01 (Forced)
  HAL_I2C_Mem_Write(hi2c, BME680_I2C_ADDR, BME680_REG_CTRL_MEAS, 1, &ctrl_meas,
                    1, 100);

  // Wait for measurement (datasheet says typ 1.82ms for T/P/H x1, safe 10ms)
  HAL_Delay(10);

  uint8_t raw_data[8];
  // Read Pressure (0x1F), Temp (0x22), Hum (0x25) - Burst read
  HAL_I2C_Mem_Read(hi2c, BME680_I2C_ADDR, BME680_REG_PRESS_MSB, 1, raw_data, 8,
                   100);

  uint32_t press_adc =
      (uint32_t)(((uint32_t)raw_data[0] << 12) | ((uint32_t)raw_data[1] << 4) |
                 ((uint32_t)raw_data[2] >> 4));
  uint32_t temp_adc =
      (uint32_t)(((uint32_t)raw_data[3] << 12) | ((uint32_t)raw_data[4] << 4) |
                 ((uint32_t)raw_data[5] >> 4));
  uint16_t hum_adc =
      (uint16_t)(((uint16_t)raw_data[6] << 8) | (uint16_t)raw_data[7]);

  data->Temperature = BME680_Compensate_Temperature(temp_adc);
  data->Pressure = BME680_Compensate_Pressure(press_adc);
  data->Humidity = BME680_Compensate_Humidity(hum_adc);
}
