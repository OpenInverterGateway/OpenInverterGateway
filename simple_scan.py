#!/usr/bin/env python3
"""
Simple scan: Read all 16-bit holding registers and report those with value 99 or 990.
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
    # Parse arguments
    if len(sys.argv) < 3:
        print("Usage: python3 simple_scan.py <host> <start> <end>")
        print("Example: python3 simple_scan.py 192.168.4.34 1 1200")
        sys.exit(1)

    host = sys.argv[1]
    start_reg = int(sys.argv[2])
    end_reg = int(sys.argv[3])

    # Look for time values: 8:00 (2048) and 10:00 (2560)
    target_values = [2048, 2560]

    print(f"Scanning HR{start_reg} to HR{end_reg} on {host}")
    print(f"Looking for values: {target_values}")
    print("8:00 = 2048 (0x0800), 10:00 = 2560 (0x0A00)")
    print("-" * 40)

    found_values = {}

    for reg in range(start_reg, end_reg + 1):
        value = read_holding_register(host, reg)

        if value in target_values:
            if value not in found_values:
                found_values[value] = []
            found_values[value].append(reg)
            time_str = f"{(value>>8):02d}:{(value&0xFF):02d}"
            print(f"HR{reg}: {value} ({time_str}) ← FOUND!")
        elif reg % 100 == 0:
            # Progress indicator every 100 registers
            print(f"...scanned up to HR{reg}")

    # Results summary
    print("\n" + "=" * 40)
    print("RESULTS:")
    print("-" * 40)

    for value, regs in found_values.items():
        time_str = f"{(value>>8):02d}:{(value&0xFF):02d}"
        print(f"\nValue {value} ({time_str}): {len(regs)} found")
        for reg in regs:
            print(f"  HR{reg}")

    if not found_values:
        print("No registers found with target values")

if __name__ == "__main__":
    main()