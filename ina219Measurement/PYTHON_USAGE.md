# Python Data Logger Usage Guide

## Installation

### 1. Install Python Dependencies
```bash
pip install -r requirements.txt
```

Or install manually:
```bash
pip install pyserial matplotlib numpy
```

### 2. Check Arduino Connection
```bash
# Linux/Mac - list available ports
ls /dev/tty*

# Windows - check Device Manager or try:
python -c "import serial.tools.list_ports; print([p.device for p in serial.tools.list_ports.comports()])"
```

## Usage Options

### Option 1: Advanced Logger with Real-time Plotting

```bash
# Basic usage (console logging)
python ina219_logger.py

# With real-time plot
python ina219_logger.py --plot

# Specify port and duration
python ina219_logger.py -p /dev/ttyUSB0 -t 300 --plot

# Custom output file
python ina219_logger.py -o my_measurement.csv -t 60
```

**Command Line Options:**
- `-p, --port`: Serial port (default: /dev/ttyUSB0)
- `-b, --baudrate`: Baud rate (default: 115200)
- `-t, --time`: Duration in seconds (0 = infinite)
- `-o, --output`: Output CSV filename
- `--plot`: Show real-time plot
- `--no-save`: Don't save data to CSV

### Option 2: Simple Logger (Basic)

```bash
python simple_logger.py
```

Edit the script to change:
- `PORT = '/dev/ttyUSB0'` (your Arduino port)
- `DURATION = 60` (logging time in seconds)

## Real-time Plotting Features

The advanced logger (`ina219_logger.py --plot`) provides:

- **Live Current Graph**: Shows instantaneous and average current
- **Live Power Graph**: Shows power consumption over time
- **Auto-scaling**: Axes automatically adjust to data range
- **Statistics**: Real-time min/max/average calculations

### Plot Controls
- **Zoom**: Mouse wheel or toolbar zoom
- **Pan**: Click and drag to pan
- **Save**: Use toolbar to save plot as image
- **Close**: Close window to stop logging and save data

## Data Analysis Examples

### 1. Basic Statistics
```python
import pandas as pd
import numpy as np

# Load CSV data
df = pd.read_csv('ina219_data_20240831_120000.csv')

# Basic statistics
print(f"Average current: {df['Current_uA'].mean():.1f} μA")
print(f"Peak current: {df['Current_uA'].max():.1f} μA")
print(f"Current range: {df['Current_uA'].max() - df['Current_uA'].min():.1f} μA")
print(f"Standard deviation: {df['Current_uA'].std():.1f} μA")
```

### 2. Power Analysis
```python
# Power consumption analysis
total_energy = df['Power_mW'].sum() * 0.1  # Approximate energy in mWs
avg_power = df['Power_mW'].mean()
print(f"Average power: {avg_power:.2f} mW")
print(f"Estimated energy: {total_energy:.1f} mWs")

# Battery life estimation (example: 1000mAh battery at 3.7V)
battery_capacity_mwh = 1000 * 3.7  # 3700 mWh
estimated_hours = battery_capacity_mwh / avg_power
print(f"Estimated battery life: {estimated_hours:.1f} hours")
```

### 3. Peak Detection
```python
from scipy.signal import find_peaks

# Find current peaks
peaks, _ = find_peaks(df['Current_uA'], height=df['Current_uA'].mean() + 50)
print(f"Found {len(peaks)} current peaks")

# Analyze peak characteristics
peak_currents = df['Current_uA'].iloc[peaks]
print(f"Peak current range: {peak_currents.min():.1f} - {peak_currents.max():.1f} μA")
```

### 4. Plotting with Matplotlib
```python
import matplotlib.pyplot as plt

# Create time axis in seconds
time_s = df['Time_ms'] / 1000

# Plot current over time
plt.figure(figsize=(12, 6))
plt.subplot(2, 1, 1)
plt.plot(time_s, df['Current_uA'], 'b-', linewidth=0.5, label='Current')
plt.plot(time_s, df['Avg_uA'], 'r-', linewidth=2, label='Average')
plt.ylabel('Current (μA)')
plt.legend()
plt.grid(True, alpha=0.3)

plt.subplot(2, 1, 2)
plt.plot(time_s, df['Power_mW'], 'g-', linewidth=1)
plt.xlabel('Time (s)')
plt.ylabel('Power (mW)')
plt.grid(True, alpha=0.3)

plt.tight_layout()
plt.show()
```

## Troubleshooting

### Common Issues

1. **"Permission denied" on Linux**
   ```bash
   sudo usermod -a -G dialout $USER
   # Then logout and login again
   ```

2. **"Port not found" error**
   - Check Arduino is connected
   - Verify port name: `ls /dev/tty*` (Linux) or Device Manager (Windows)
   - Try different USB port

3. **"No module named 'serial'"**
   ```bash
   pip install pyserial
   ```

4. **Matplotlib not displaying**
   ```bash
   # Linux - install tkinter
   sudo apt-get install python3-tk
   
   # Or use different backend
   export MPLBACKEND=TkAgg
   ```

5. **Data appears corrupted**
   - Check baud rate (must be 115200)
   - Verify Arduino code is uploaded correctly
   - Try resetting Arduino

### Performance Tips

1. **For long recordings**: Use `--no-save` during recording, then save manually
2. **For high-speed logging**: Increase buffer size in code
3. **For real-time analysis**: Use the plotting mode for immediate feedback

## Example Workflows

### 1. Quick Current Check
```bash
# 30-second measurement with plot
python ina219_logger.py -t 30 --plot
```

### 2. Battery Life Test
```bash
# 24-hour logging (save every hour to prevent data loss)
python ina219_logger.py -t 86400 -o battery_test.csv
```

### 3. Development Debugging
```bash
# Continuous monitoring with real-time plot
python ina219_logger.py --plot
# Press Ctrl+C when done
```

### 4. Automated Testing
```python
# Custom script for automated measurements
import subprocess
import time

test_conditions = ['idle', 'active', 'sleep']
for condition in test_conditions:
    print(f"Testing {condition} mode...")
    input("Set device to {condition} mode and press Enter...")
    
    # Log for 2 minutes
    subprocess.run([
        'python', 'ina219_logger.py', 
        '-t', '120', 
        '-o', f'test_{condition}.csv'
    ])
```

## Data Export Formats

The logger saves data in CSV format compatible with:
- **Excel/Google Sheets**: Direct import
- **MATLAB**: `readtable('filename.csv')`
- **Python pandas**: `pd.read_csv('filename.csv')`
- **R**: `read.csv('filename.csv')`
- **LabVIEW**: CSV import functions

Ready to start logging! 🐍📊