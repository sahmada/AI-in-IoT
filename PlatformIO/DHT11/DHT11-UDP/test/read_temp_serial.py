import serial

s = serial.Serial(port='COM9', baudrate=115200, timeout=1)

try:
    while True:
        line = s.readline()
        try:
            line_decoded = line.decode('ascii')
            print(line_decoded)
        except:
            continue

        temp = temp_object["temp"]
        hum = temp_object["hum"]

        log(temp, hum)
except KeyboardInterrupt:
    s.close()