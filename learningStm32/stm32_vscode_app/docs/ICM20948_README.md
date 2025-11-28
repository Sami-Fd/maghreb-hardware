# ICM-20948 Sensor Driver Documentation

## Overview
The **ICM-20948** is a low-power 9-axis MotionTracking device that is ideally suited for Smartphones, Tablets, Wearable Sensors, and IoT applications. It combines a 3-axis gyroscope, a 3-axis accelerometer, and a 3-axis compass (magnetometer) in a small package.

In this project, we are utilizing the **Accelerometer** and **Gyroscope** functionalities over the **I2C** interface.

## Hardware Interface
*   **Communication Protocol**: I2C
*   **I2C Address**: `0x68` (AD0 = GND) or `0x69` (AD0 = VCC).
    *   *Current Project Setting*: `0x68` (Defined in `icm20948.h`).
*   **Bus Speed**: Standard Mode (100 kHz) or Fast Mode (400 kHz).

## Hardware Connections (Nucleo-L432KC)

| Sensor Pin | Nucleo Pin | STM32 Pin | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3V3 | - | 3.3V Power Supply |
| **GND** | GND | - | Ground |
| **SCL** | **D5** | **PB6** | I2C Clock Line |
| **SDA** | **D4** | **PB7** | I2C Data Line |
| **AD0** | GND | - | Sets I2C Address to `0x68` |
| **NCS** | 3V3 | - | Chip Select (High for I2C mode) |

> **Note**: If using a breakout board, ensure AD0 is connected to GND for address `0x68`. If connected to VCC, change the address in `icm20948.h` to `0x69`.

## Porting to Other STM32 Microcontrollers

If you want to use this driver with a different STM32 board (e.g., STM32F103, STM32F4, etc.), follow these steps:

1.  **Hardware Configuration (CubeMX / .ioc)**:
    *   Enable an I2C peripheral (e.g., I2C1, I2C2) in your project configuration.
    *   Note the pins assigned to SCL and SDA.
    *   Generate the code.

2.  **Driver Integration**:
    *   Copy `icm20948.c` and `icm20948.h` to your project's `Src` and `Inc` folders.
    *   In `main.c`, include `icm20948.h`.

3.  **Update I2C Handle**:
    *   The driver functions accept a pointer to `I2C_HandleTypeDef`.
    *   If your new project uses `hi2c2` instead of `hi2c1`, simply pass `&hi2c2` to the init and read functions:
        ```c
        ICM20948_Init(&hi2c2);
        ICM20948_ReadAll(&hi2c2, &data);
        ```

4.  **Dependencies**:
    *   Ensure `stm32l4xx_hal.h` is replaced with the appropriate HAL header for your family (e.g., `stm32f4xx_hal.h`) in `icm20948.h`.
    *   Alternatively, include `main.h` in `icm20948.h` which usually includes the correct HAL header automatically.

## Driver Implementation Details

### Files
*   `Core/Src/icm20948.c`: Source code for initialization and data reading.
*   `Core/Inc/icm20948.h`: Header file containing register definitions, struct definitions, and function prototypes.

### Initialization Sequence (`ICM20948_Init`)
1.  **Chip ID Check**:
    *   Reads register `WHO_AM_I` (`0x00`).
    *   Expected value: `0xEA`.
    *   If the ID does not match, initialization returns an error.
2.  **Power Management**:
    *   By default, the sensor starts in sleep mode.
    *   Writes `0x01` to `PWR_MGMT_1` (`0x06`) to wake up the sensor and select the Auto-Select Clock source (best available clock).
3.  **Configuration** (Optional/Default):
    *   The current driver uses default sensitivity settings:
        *   **Accel Range**: ±2g
        *   **Gyro Range**: ±250 dps

### Data Acquisition (`ICM20948_ReadAll`)
The driver reads 12 consecutive bytes of data starting from register `ACCEL_XOUT_H` (`0x2D`).

**Register Map for Data:**
| Register Address | Data |
| :--- | :--- |
| `0x2D` | Accel X High Byte |
| `0x2E` | Accel X Low Byte |
| `0x2F` | Accel Y High Byte |
| `0x30` | Accel Y Low Byte |
| `0x31` | Accel Z High Byte |
| `0x32` | Accel Z Low Byte |
| `0x33` | Gyro X High Byte |
| `0x34` | Gyro X Low Byte |
| `0x35` | Gyro Y High Byte |
| `0x36` | Gyro Y Low Byte |
| `0x37` | Gyro Z High Byte |
| `0x38` | Gyro Z Low Byte |

### Data Conversion
The raw data is a 16-bit signed integer (Two's Complement). To convert this to physical units, it must be divided by the sensitivity scale factor.

**Accelerometer Formula:**
$$ \text{Accel}(g) = \frac{\text{Raw}_{16bit}}{16384.0} $$
*   Scale Factor for ±2g: 16384 LSB/g

**Gyroscope Formula:**
$$ \text{Gyro}(dps) = \frac{\text{Raw}_{16bit}}{131.0} $$
*   Scale Factor for ±250 dps: 131 LSB/dps

## Usage Example
```c
ICM20948_Data_t myData;

// Initialize
if (ICM20948_Init(&hi2c1) == 0) {
    // Success
}

// Read Loop
while(1) {
    ICM20948_ReadAll(&hi2c1, &myData);
    printf("Accel X: %.2f g\n", myData.Accel_X);
}
```
