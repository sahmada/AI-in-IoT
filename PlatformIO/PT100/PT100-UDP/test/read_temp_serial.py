import serial
import json
from rtd import *

s = serial.Serial(port='COM1', baudrate=9600, timeout=1)

try:
    while True:
        line = s.readline()
        try:
            line_decoded = line.decode('ascii')
            temp_object = json.loads(line_decoded.strip())
        except:
            continue

        temp = temp_object["temp"]
        fault = temp_object["fault"]

        log(temp, fault)
except KeyboardInterrupt:
    s.close()
