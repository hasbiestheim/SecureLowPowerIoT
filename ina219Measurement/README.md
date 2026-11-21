# INA219 Ultra-High Precision Current Monitor

This project implements an Arduino Uno-based current monitoring system using the INA219 sensor, optimized for detecting μA-level changes in low-power circuits.

## Features

- **Ultra-High Precision**: Detects current changes down to μA levels
- **Fast Sampling**: 10Hz measurement rate for dynamic load analysis
- **Statistical Analysis**: Moving averages, peak detection, min/max tracking
- **Noise Filtering**: Outlier detection and software averaging
- **Real-time Output**: Formatted Serial data for analysis
- **Interactive Commands**: Reset statistics, calibration info, help

## Hardware Requirements

- Arduino Uno
- INA219 Current/Power Monitor breakout board
- Target circuit to monitor (0-400mA range)
- Jumper wires
- Breadboard (optional)

## Wiring Diagram

```
Arduino Uno    INA219 Module
-----------    -------------
5V       -->   VCC
GND      -->   GND
A4 (SDA) -->   SDA
A5 (SCL) -->   SCL

INA219 Module  Target Circuit
-------------  --------------
VIN+     -->   Positive Supply (e.g., 5V)
VIN-     -->   Circuit Input
GND      -->   Common Ground
```

### Detailed Connection Guide

1. **Power Connections**:
   - Connect INA219 VCC to Arduino 5V
   - Connect INA219 GND to Arduino GND

2. **I2C Communication**:
   - Connect INA219 SDA to Arduino A4 (SDA)
   - Connect INA219 SCL to Arduino A5 (SCL)

3. **Current Measurement Setup**:
   - Connect your power supply positive to INA219 VIN+
   - Connect INA219 VIN- to your target circuit's positive input
   - Ensure common ground between all components

## Software Features

### Precision Specifications
- **Current Resolution**: Enhanced to ~1μA via software averaging
- **Voltage Resolution**: 4mV bus voltage, calculated shunt voltage in μV
- **Power Resolution**: Calculated in μW for low-power analysis
- **Sampling Rate**: 10Hz (100ms intervals)

### Statistical Analysis
- **Moving Average**: 10-sample rolling average for noise reduction
- **Peak Detection**: Automatic detection of current spikes >50μA above average
- **Min/Max Tracking**: Continuous monitoring of measurement range
- **Outlier Filtering**: Rejection of measurements >5μA from previous valid reading

### Serial Output Format
Simple CSV format compatible with Arduino Serial Plotter and easy to save:
```
Time_ms,Bus_V,Shunt_mV,Current_mA,Power_mW,Current_uA,Avg_uA
1000,5.000,0.12,1.250,6.25,1250.0,1248.0
1100,5.001,0.08,0.830,4.15,830.0,1089.0
```

### Interactive Commands
Send these commands via Serial Monitor:
- `reset` - Reset all statistics and counters
- `cal` - Display calibration information
- `help` - Show available commands

## Installation

1. **PlatformIO Setup**:
   ```bash
   # Clone or create project
   cd LowCurrentSensor
   
   # Install dependencies (automatic with platformio.ini)
   pio lib install
   
   # Build and upload
   pio run --target upload
   
   # Open serial monitor
   pio device monitor
   ```

2. **Arduino IDE Setup**:
   - Install Adafruit INA219 library
   - Install Adafruit BusIO library
   - Set board to Arduino Uno
   - Set baud rate to 115200

## Usage

1. **Hardware Setup**:
   - Wire the INA219 according to the diagram above
   - Connect your target circuit to be monitored
   - Ensure all connections are secure

2. **Software Operation**:
   - Upload the code to Arduino Uno
   - Open Serial Monitor at 115200 baud
   - Observe real-time current measurements
   - Use interactive commands as needed

3. **Data Analysis**:
   - Monitor the statistical columns for trends
   - Watch for peak detection alerts
   - Use the periodic summary reports
   - Reset statistics when starting new measurements

## Calibration and Accuracy

### Hardware Calibration
- Uses standard 0.1Ω shunt resistor (typical on INA219 breakouts)
- Configured for 32V bus voltage range
- Optimized for 0-400mA current measurements

### Software Enhancement
- 10-sample moving average reduces noise
- Outlier detection improves accuracy
- Statistical analysis reveals measurement quality

### Expected Accuracy
- **Current**: ±1-2μA typical, ±5μA maximum
- **Voltage**: ±4mV typical
- **Power**: Calculated from current × voltage

## Troubleshooting

### Common Issues

1. **"Failed to find INA219 chip!"**
   - Check I2C wiring (SDA/SCL)
   - Verify power connections
   - Confirm I2C address (default 0x40)

2. **Erratic Readings**
   - Check for loose connections
   - Ensure proper grounding
   - Verify target circuit current is within 0-400mA range

3. **No Serial Output**
   - Confirm baud rate is 115200
   - Check USB connection
   - Verify Arduino is powered

### Optimization Tips

1. **For Maximum Precision**:
   - Use short, thick wires for current path
   - Ensure solid ground connections
   - Keep measurement setup away from noise sources

2. **For Stable Readings**:
   - Allow system to warm up for 1-2 minutes
   - Use the moving average values for analysis
   - Reset statistics when changing test conditions

## Technical Specifications

- **Microcontroller**: Arduino Uno (ATmega328P)
- **Sensor**: INA219 Current/Power Monitor
- **Communication**: I2C (400kHz)
- **Current Range**: 0-400mA (optimized for low power)
- **Voltage Range**: 0-26V bus voltage
- **Resolution**: ~1μA current, 4mV voltage
- **Sampling Rate**: 10Hz
- **Data Output**: Serial 115200 baud

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Support

For issues or questions:
1. Check the troubleshooting section
2. Verify hardware connections
3. Confirm software configuration
4. Review serial output for error messages