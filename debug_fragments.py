#!/usr/bin/env python3
"""
Debug script to verify holding register fragments are being read correctly.
"""

import requests
import sys

def read_holding_register(host, register):
    """Read a single 16-bit holding register."""
    url = f"http://{host}/postCommunicationModbus_p"
    data = {
        "operation": "R",
        "registerType": "H",
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

    # Fragment definitions from Growatt307.cpp
    fragments = [
        {"name": "Active Power Rate", "start": 3, "size": 1},
        {"name": "Date/Time", "start": 45, "size": 6},
        {"name": "Export Limit", "start": 122, "size": 2},
        {"name": "Grid First settings", "start": 1070, "size": 2},
        {"name": "Grid First time slots", "start": 1080, "size": 9},
        {"name": "Battery First settings", "start": 1090, "size": 3},
        {"name": "Battery First time slots", "start": 1100, "size": 9}
    ]

    print("Checking Holding Register Fragments for Protocol 307:")
    print("=" * 60)

    for frag in fragments:
        print(f"\n{frag['name']} (HR{frag['start']}-{frag['start']+frag['size']-1}):")
        print("-" * 40)

        for offset in range(frag['size']):
            reg = frag['start'] + offset
            value = read_holding_register(host, reg)
            if value is not None:
                # Special formatting for time values
                if 1080 <= reg <= 1088 or 1100 <= reg <= 1108:
                    if value >= 256:  # Likely a time value
                        time_str = f" ({(value>>8):02d}:{(value&0xFF):02d})"
                    else:
                        time_str = ""
                    print(f"  HR{reg}: {value}{time_str}")
                else:
                    print(f"  HR{reg}: {value}")
            else:
                print(f"  HR{reg}: <read failed>")

    # Now check the status JSON
    print("\n" + "=" * 60)
    print("Checking Status JSON:")
    print("-" * 40)

    try:
        response = requests.get(f"http://{host}/status")
        if response.status_code == 200:
            import json
            data = response.json()

            # Check for Grid First values
            grid_keys = [k for k in data.keys() if "Grid" in k or "grid" in k]
            if grid_keys:
                print("\nGrid-related keys found:")
                for key in sorted(grid_keys):
                    print(f"  {key}: {data[key]}")
            else:
                print("\nNo Grid-related keys found in status JSON!")

            # Check for Battery First values
            batt_keys = [k for k in data.keys() if "Batt" in k or "batt" in k]
            if batt_keys:
                print("\nBattery-related keys found:")
                for key in sorted(batt_keys):
                    print(f"  {key}: {data[key]}")
    except Exception as e:
        print(f"Failed to get status: {e}")

if __name__ == "__main__":
    main()