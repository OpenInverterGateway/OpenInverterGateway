#!/usr/bin/env python3
import requests
import sys

host = sys.argv[1] if len(sys.argv) > 1 else "192.168.4.34"
start = int(sys.argv[2]) if len(sys.argv) > 2 else 1
end = int(sys.argv[3]) if len(sys.argv) > 3 else 1200

for reg in range(start, end + 1):
    try:
        r = requests.post(f"http://{host}/postCommunicationModbus_p",
                         data={"operation": "R", "registerType": "H",
                               "type": "16b", "reg": str(reg), "val": ""},
                         timeout=1)
        if "value" in r.text:
            val = int(r.text.split("value")[-1])
            if val == 99 or val == 990:
                print(f"HR{reg}: {val}")
    except:
        pass