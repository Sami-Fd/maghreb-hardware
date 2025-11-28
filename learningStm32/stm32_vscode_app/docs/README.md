# STM32L432KC Sensor Interface Project

This project demonstrates how to interface a **Nucleo-L432KC** board with two sensors simultaneously using separate I2C buses:
1.  **ICM-20948**: 9-Axis Inertial Measurement Unit (Accel, Gyro, Mag).
2.  **BME680**: Environmental Sensor (Temperature, Pressure, Humidity, Gas).

## Hardware Connections

The project uses two separate I2C peripherals to avoid address conflicts and simplify wiring.

### 1. ICM-20948 (IMU)
Connected to **I2C1**.

| Sensor Pin | Nucleo Pin | STM32 Pin | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3V3 | - | 3.3V Power |
| **GND** | GND | - | Ground |
| **SCL** | **D5** | **PB6** | I2C1 Clock |
| **SDA** | **D4** | **PB7** | I2C1 Data |

### 2. BME680 (Environmental)
Connected to **I2C3**.

| Sensor Pin | Nucleo Pin | STM32 Pin | Description |
| :--- | :--- | :--- | :--- |
| **VCC** | 3V3 | - | 3.3V Power |
| **GND** | GND | - | Ground |
| **SCL** | **A6** | **PA7** | I2C3 Clock |
| **SDA** | **D12** | **PB4** | I2C3 Data |
| **SDO** | 3V3 | - | Selects Address 0x77 |

> **Note**: The BME680 address is configured to `0x77` in software. Ensure the SDO pin is pulled high (to 3V3). If SDO is grounded, the address is `0x76`.

---

## Software Implementation Details

### 1. Project Structure
*   **Core/Src/main.c**: Main application loop. Initializes peripherals and polls sensors.
*   **Core/Src/icm20948.c**: Driver implementation for the IMU.
*   **Core/Src/bme680.c**: Driver implementation for the Environmental sensor.
*   **Core/Src/stm32l4xx_hal_msp.c**: Low-level hardware initialization (Pin muxing for I2C1 and I2C3).

### 2. ICM-20948 Driver (`icm20948.c`)
*   **Bus**: Uses `hi2c1` (I2C1).
*   **Initialization**:
    *   Checks Chip ID (Register `0x00`, expected `0xEA`).
    *   Wakes the sensor from sleep mode (Register `0x06`).
    *   Configures the clock source to Auto-Select.
*   **Data Reading**:
    *   Reads 12 bytes starting from `ACCEL_XOUT_H`.
    *   Converts raw 16-bit signed integers to physical values (g-force and dps).
    *   **Scaling**:
        *   Accelerometer: Default ±2g range (Sensitivity: 16384 LSB/g).
        *   Gyroscope: Default ±250dps range (Sensitivity: 131 LSB/dps).

### 3. BME680 Driver (`bme680.c`)
*   **Bus**: Uses `hi2c3` (I2C3).
*   **Address**: `0x77` (Defined in `bme680.h`).
*   **Initialization**:
    *   Checks Chip ID (Register `0xD0`, expected `0x61`).
    *   Performs a Soft Reset.
    *   **Calibration**: Reads factory calibration coefficients from internal ROM (Registers `0x89-0xA1` and `0xE1-0xF0`). These are required to calculate accurate physical values.
    *   **Configuration**: Sets oversampling for Humidity, Temperature, and Pressure to x1. Configures IIR filter.
*   **Data Reading**:
    *   Sets sensor to **Forced Mode** to trigger a single measurement.
    *   Waits for measurement completion.
    *   Reads raw ADC values for Temp, Pressure, and Humidity.
    *   **Compensation**: Applies the Bosch Sensortec compensation formulas using the stored calibration coefficients to convert raw ADC data into human-readable float values (Deg C, hPa, %RH).

### 4. Main Loop (`main.c`)
The `main()` function performs the following steps:
1.  **System Init**: Sets up the system clock (80MHz) and HAL library.
2.  **Peripheral Init**: Initializes GPIO, UART2 (for printf), I2C1, and I2C3.
3.  **Sensor Init**: Calls `ICM20948_Init` and `BME680_Init`. Prints status to UART.
4.  **Infinite Loop**:
    *   Reads data from both sensors.
    *   Prints formatted data to the serial console.
    *   **Float Printing**: Since standard `printf` float support is often disabled to save space, the code manually formats floats by printing the integer part and the fractional part separately (e.g., `printf("%d.%02d", val_int, val_frac)`).

## How to Build and Run

1.  **Build**:
    ```bash
    cmake --build build/Debug
    ```
2.  **Flash**:
    Use the VS Code "Run" or "Debug" tab, or use STM32CubeProgrammer with the generated `.elf` or `.bin` file.
3.  **Monitor**:
    Open a serial terminal (e.g., PuTTY, Serial Monitor) connected to the Nucleo's COM port.
    *   **Baud Rate**: 115200
    *   **Data Bits**: 8
    *   **Parity**: None
    *   **Stop Bits**: 1
