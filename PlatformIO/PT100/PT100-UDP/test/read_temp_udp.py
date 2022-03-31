from socket import *
import json
from rtd import *

s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', 61000))

try:
    while True:
        b, a = s.recvfrom(1024)

        try:
            line_decoded = b.decode('ascii')
            temp_object = json.loads(line_decoded.strip())
        except:
            continue

        temp = temp_object["temp"]
        fault = temp_object["fault"]

        log(temp, fault)
except KeyboardInterrupt:
    s.close()
