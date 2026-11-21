#!/usr/bin/env python3
"""
Simple INA219 Data Logger
Basic example for logging current measurements to CSV
"""

import serial
import csv
import time
from datetime import datetime

def main():
    # Configuration
    PORT = '/dev/ttyUSB0'  # Change to your Arduino port (Windows: 'COM3', etc.)
    BAUDRATE = 115200
    DURATION = 60  # Log for 60 seconds
    
    print("Simple INA219 Data Logger")
    print(f"Port: {PORT}")
    print(f"Duration: {DURATION} seconds")
    print("-" * 40)
    
    try:
        # Connect to Arduino
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        time.sleep(2)  # Wait for Arduino to initialize
        print("Connected to Arduino")
        
        # Create CSV file with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"ina219_simple_{timestamp}.csv"
        
        with open(filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            
            # Write header
            writer.writerow(['Time_ms', 'Bus_V', 'Shunt_mV', 'Current_mA', 
                           'Power_mW', 'Current_uA', 'Avg_uA'])
            
            start_time = time.time()
            sample_count = 0
            
            print("Logging data...")
            print("Sample | Current(μA) | Power(mW)")
            print("-" * 35)
            
            while (time.time() - start_time) < DURATION:
                try:
                    line = ser.readline().decode('utf-8').strip()
                    
                    if line and ',' in line:
                        # Parse CSV line
                        parts = line.split(',')
                        if len(parts) == 7 and parts[0].isdigit():
                            # Write to CSV
                            writer.writerow(parts)
                            csvfile.flush()  # Ensure data is written
                            
                            # Display progress
                            sample_count += 1
                            current_ua = float(parts[5])
                            power_mw = float(parts[4])
                            
                            if sample_count % 10 == 0:  # Print every 10th sample
                                print(f"{sample_count:6d} | {current_ua:8.1f}   | {power_mw:6.2f}")
                
                except (ValueError, UnicodeDecodeError, IndexError):
                    continue  # Skip invalid lines
        
        print(f"\nLogging complete!")
        print(f"Samples collected: {sample_count}")
        print(f"Data saved to: {filename}")
        
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        print("Check that:")
        print("1. Arduino is connected")
        print("2. Correct port is specified")
        print("3. No other programs are using the port")
    
    except KeyboardInterrupt:
        print("\nLogging stopped by user")
    
    except Exception as e:
        print(f"Error: {e}")
    
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial port closed")

if __name__ == '__main__':
    main()