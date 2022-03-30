import serial
import json

s = serial.Serial(port='COM9', baudrate=115200, timeout=2)

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
        hum = temp_object["hum"]
        
except KeyboardInterrupt:
    s.close()
