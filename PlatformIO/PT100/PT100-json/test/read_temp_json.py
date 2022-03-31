import serial
import json

MAX31865_FAULT_HIGHTHRESH = 0x80
MAX31865_FAULT_LOWTHRESH = 0x40
MAX31865_FAULT_REFINLOW = 0x20
MAX31865_FAULT_REFINHIGH = 0x10
MAX31865_FAULT_RTDINLOW = 0x08
MAX31865_FAULT_OVUV = 0x04

s = serial.Serial(port='COM9', baudrate=115200, timeout=1)

try:
    while True:
        line = s.readline()
        try:
            line_decoded = line.decode('ascii')
            temp_object = json.loads(line_decoded.strip())
        except:
            print("ERROR")
            continue
        temp = temp_object["temp"]
        fault = temp_object["fault"]
        if fault:

            if fault & MAX31865_FAULT_HIGHTHRESH:

                print("RTD High Threshold")

            if fault & MAX31865_FAULT_LOWTHRESH:

                print("RTD Low Threshold")

            if fault & MAX31865_FAULT_REFINLOW:

                print("REFIN- > 0.85 x Bias")

            if fault & MAX31865_FAULT_REFINHIGH:

                print("REFIN- < 0.85 x Bias - FORCE- open")

            if fault & MAX31865_FAULT_RTDINLOW:

                print("RTDIN- < 0.85 x Bias - FORCE- open")

            if fault & MAX31865_FAULT_OVUV:

                print("Under/Over voltage")
        else:
            print(temp)

except KeyboardInterrupt:
    s.close()
