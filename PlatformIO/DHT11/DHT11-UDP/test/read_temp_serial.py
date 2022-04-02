import serial

s = serial.Serial(port='COM9', baudrate=115200, timeout=2)

try:
    while True:
        line = s.readline()
        line_decoded = line.decode('ascii')
        print(line_decoded)
except KeyboardInterrupt:
    s.close()