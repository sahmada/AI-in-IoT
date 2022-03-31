from socket import *
s = socket(AF_INET, SOCK_DGRAM)
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
s.sendto('{"test":2}'.encode('ascii'), ('255.255.255.255', 61000))
