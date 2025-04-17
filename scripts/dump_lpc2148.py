import serial
import time
import struct
from hexdump import hexdump
import argparse
import binascii

CHUNK_SIZE = 0x10

if __name__ == '__main__':
    
    parser = argparse.ArgumentParser(prog='dump_lpc2148')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-a', "--addr", required=True, nargs='+', type=str)
    parser.add_argument('-f', "--file", required=True, type=str)
    args = parser.parse_args()

    conn = serial.Serial(args.port, 9600, timeout=0.1)
    f = open(args.file, 'wb')

    addr_start = int(args.addr[0], 16)  
    addr_end = int(args.addr[1], 16)    

    assert ((addr_start % CHUNK_SIZE) == 0)
    assert ((addr_end % CHUNK_SIZE) == 0)    
    assert (addr_start < addr_end)

    for i in range(5):
        conn.write("R {:d} {:d}\r\n".format(0, 4).encode('utf-8'))        
        time.sleep(0.07)
        res = conn.read(conn.in_waiting)
        conn.write(b"OK\r\n")  
        time.sleep(0.07)
        res = conn.read(conn.in_waiting) 

    time.sleep(0.1)
    data = b''
    for i in range(addr_start, addr_end, CHUNK_SIZE):
        conn.write("R {:d} {:d}\r\n".format(i, CHUNK_SIZE).encode('utf-8'))
        
        time.sleep(0.1)
        res = conn.read(conn.in_waiting)
        # print(res)
        # print(res.    split(b"\r\n"))
        if len(res.split(b"\r\n")) >= 3:
            r1 = res.split(b"\r\n")[0]
            r2 = res.split(b"\r\n")[1]
            r3 = res.split(b"\r\n")[2]
            
            if b"0" == r2:
                # print("resp cmd ok")
                try:
                    a = binascii.a2b_uu(r3)
                except  Exception:
                    # print("try cut")
                    try:
                        a = binascii.a2b_uu(r3[:-1])
                    except Exception:
                        # print("try cut")
                        try:
                            a = binascii.a2b_uu(r3[:-2])
                        except Exception:
                            print("Error encode data")
                            break
                val = binascii.hexlify(a)

                assert (len(a) == CHUNK_SIZE)
                print("Addr = 0x{:x} {}".format(i,val))
                data += a
                f.write(a)

        conn.write(b"OK\r\n")  
        time.sleep(0.07)
        res = conn.read(conn.in_waiting)    

