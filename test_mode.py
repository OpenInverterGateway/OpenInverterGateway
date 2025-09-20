#!/usr/bin/env python3
"""
Test reading input register 118 (CurrentMode).
"""

import requests
import sys

def read_input_register(host, register):
    """Read a single 16-bit input register."""
    url = f"http://{host}/postCommunicationModbus_p"
    data = {
        "operation": "R",
        "registerType": "I",  # Input register
        "type": "16b",
        "reg": str(register),
        "val": ""
    }
    headers = {"Content-Type": "application/x-www-form-urlencoded"}

    try:
        response = requests.post(url, data=data, headers=headers, timeout=2)
        if response.status_code == 200 and "value" in response.text:
            value_str = response.text.split("value")[-1].strip()
            return int(value_str)
    except:
        return None

def main():
    host = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.34"

    print("Testing Input Register 118 (Current Mode):")
    print("=" * 45)

    value = read_input_register(host, 118)
    if value is not None:
        modes = {0: "Load-first", 1: "Battery-first", 2: "Grid-first"}
        mode_name = modes.get(value, f"Unknown ({value})")
        print(f"IR118 (CurrentMode): {value} = {mode_name}")
    else:
        print("Failed to read IR118")

    # Also check the status JSON
    print("\nChecking Status JSON:")
    print("-" * 45)
    try:
        response = requests.get(f"http://{host}/status")
        if response.status_code == 200:
            import json
            data = response.json()
            if "CurrentMode" in data:
                value = data["CurrentMode"]
                modes = {0: "Load-first", 1: "Battery-first", 2: "Grid-first"}
                mode_name = modes.get(value, f"Unknown ({value})")
                print(f"CurrentMode: {value} = {mode_name}")
            else:
                print("CurrentMode not found in status JSON")
    except Exception as e:
        print(f"Failed to get status: {e}")

if __name__ == "__main__":
    main()