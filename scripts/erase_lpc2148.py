import serial
import time
import struct
from hexdump import hexdump
import argparse
import binascii

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser(prog='erase_lpc2148')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    args = parser.parse_args()

    conn = serial.Serial(args.port, 9600, timeout=0.1)

    for i in range(5):
        conn.write("R {:d} {:d}\r\n".format(0, 4).encode('utf-8'))        
        time.sleep(0.07)
        res = conn.read(conn.in_waiting)
        conn.write(b"OK\r\n")  
        time.sleep(0.07)
        res = conn.read(conn.in_waiting) 


    # Unlock cmd
    conn.write(b"U 23130\r\n")        
    time.sleep(0.1)
    res = conn.read(conn.in_waiting)    
    print(res)
    if b"\r\n0\r" in res:
        print("Unlock Ok\n")

    # Prepare sector for write operation
    conn.write(b"P 0 0\r\n")        
    time.sleep(0.1)
    res = conn.read(conn.in_waiting)    
    print(res)
    if b"\r\n0\r" in res:
        print("Prepare Ok\n")

    # Erase sector
    conn.write(b"E 0 0\r\n")        
    time.sleep(0.2)
    res = conn.read(conn.in_waiting)    
    print(res)
    if b"\r\n0\r" in res:
        print("Erase Ok")