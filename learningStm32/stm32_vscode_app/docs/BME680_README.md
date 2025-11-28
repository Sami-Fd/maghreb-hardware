# BME680 Sensor Driver Documentation

## Overview
The **BME680** is an integrated environmental sensor developed specifically for mobile applications and wearables where size and low power consumption are key requirements. It can measure:
*   **Temperature**
*   **Pressure**
*   **Humidity**
*   **Gas Resistance** (Indoor Air Quality)

In this project, we are utilizing the Temperature, Pressure, and Humidity sensing capabilities over the **I2C** interface.

## Hardware Interface
*   **Communication Protocol**: I2C
*   **I2C Address**:
    *   `0x76` (SDO connected to GND)
    *   `0x77` (SDO connected to VDDIO/3.3V)
    *   *Current Project Setting*: `0x77` (Defined in `bme680.h`).

## Hardware Connections (Nucleo-L432KC)

| Sensor Pin | Nucleo Pin | STM32 Pin | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3V3 | - | 3.3V Power Supply |
| **GND** | GND | - | Ground |
| **SCL** | **A6** | **PA7** | I2C Clock Line |
| **SDA** | **D12** | **PB4** | I2C Data Line |
| **SDO** | 3V3 | - | Sets I2C Address to `0x77` |
| **CS** | 3V3 | - | Chip Select (High for I2C mode) |

> **Note**: Connect SDO to GND if you want to use address `0x76`.

## Porting to Other STM32 Microcontrollers

To use this driver on a different STM32 series (e.g., F1, F4, G0):

1.  **Hardware Setup**:
    *   Configure an I2C peripheral in STM32CubeMX.
    *   Connect the sensor to the corresponding SCL/SDA pins.

2.  **Code Integration**:
    *   Add `bme680.c` and `bme680.h` to your project.
    *   In `bme680.h`, check the `#include "stm32l4xx_hal.h"` line. Change it to match your MCU family (e.g., `#include "stm32f1xx_hal.h"` or simply `#include "main.h"`).

3.  **Function Calls**:
    *   Pass the correct I2C handle to the functions.
    *   Example for I2C2:
        ```c
        BME680_Init(&hi2c2);
        BME680_ReadAll(&hi2c2, &myBmeData);
        ```

## Driver Implementation Details

### Files
*   `Core/Src/bme680.c`: Source code for initialization, calibration reading, and compensation logic.
*   `Core/Inc/bme680.h`: Header file containing register definitions, calibration structs, and function prototypes.

### Initialization Sequence (`BME680_Init`)
1.  **Chip ID Check**:
    *   Reads register `0xD0`.
    *   Expected value: `0x61`.
2.  **Soft Reset**:
    *   Writes `0xB6` to register `0xE0` to reset the device.
    *   Waits 100ms for startup.
3.  **Calibration Data Read**:
    *   The BME680 has factory-trimmed calibration parameters stored in its internal ROM.
    *   The driver reads these parameters from two memory regions:
        *   `0x89` to `0xA1`
        *   `0xE1` to `0xF0`
    *   These coefficients (`par_t1`, `par_p1`, `par_h1`, etc.) are stored in the `BME680_CalibData_t` struct and are **essential** for calculating accurate physical values.
4.  **Configuration**:
    *   **Humidity Oversampling**: Set to x1 (Register `0x72`).
    *   **Temp/Press Oversampling**: Set to x1 (Register `0x74`).
    *   **IIR Filter**: Coefficient 3 (Register `0x75`).

### Measurement Mode: Forced Mode
The BME680 is typically used in "Forced Mode" for low power consumption.
1.  **Sleep**: Sensor is idle.
2.  **Trigger**: Host writes `01` (Forced Mode) to `CTRL_MEAS` register (`0x74`).
3.  **Measure**: Sensor performs T, P, H measurements sequentially.
4.  **Return to Sleep**: Sensor automatically returns to sleep mode after measurement.

### Data Acquisition (`BME680_ReadAll`)
1.  **Trigger Measurement**: Sets mode to Forced.
2.  **Wait**: Delays for ~10ms to allow measurement to complete.
3.  **Burst Read**: Reads 8 bytes starting from `0x1F` (Pressure MSB).
    *   Pressure: 20-bit value
    *   Temperature: 20-bit value
    *   Humidity: 16-bit value

### Compensation Algorithms
The raw ADC values read from the sensor are uncalibrated and meaningless without compensation. The driver implements the complex compensation formulas provided by Bosch Sensortec.

1.  **Temperature Compensation**:
    *   Uses `par_t1`, `par_t2`, `par_t3`.
    *   Calculates a fine resolution temperature value (`t_fine`) which is also used for pressure and humidity calculation.
2.  **Pressure Compensation**:
    *   Uses `par_p1` through `par_p10` and `t_fine`.
    *   Compensates for temperature dependency and linearity.
3.  **Humidity Compensation**:
    *   Uses `par_h1` through `par_h7` and `t_fine`.
    *   Compensates for temperature dependency.

## Usage Example
```c
BME680_Data_t envData;

// Initialize
if (BME680_Init(&hi2c3) == 0) {
    // Success
}

// Read Loop
while(1) {
    BME680_ReadAll(&hi2c3, &envData);
    printf("Temp: %.2f C, Press: %.2f hPa, Hum: %.2f %%\n", 
           envData.Temperature, envData.Pressure, envData.Humidity);
    HAL_Delay(1000); // Read every second
}
```
