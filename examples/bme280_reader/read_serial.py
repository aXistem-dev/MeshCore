#!/usr/bin/env python3
"""
Simple serial monitor for BME280 reader
Usage: python3 read_serial.py
"""
import serial
import sys
import time

PORT = '/dev/cu.usbmodem1101'
BAUD = 115200

try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Connected to {PORT} at {BAUD} baud")
    print("Reading sensor data... (Press Ctrl+C to exit)\n")
    
    # Wait a moment for connection to stabilize
    time.sleep(2)
    
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(line)
        time.sleep(0.1)
        
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    print(f"\nMake sure:")
    print(f"  1. The device is connected to {PORT}")
    print(f"  2. No other program is using the serial port")
    print(f"  3. The device is powered on")
    sys.exit(1)
except KeyboardInterrupt:
    print("\n\nExiting...")
    if 'ser' in locals():
        ser.close()
    sys.exit(0)

