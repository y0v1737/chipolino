import serial
import time
import struct
from hexdump import hexdump
import argparse

def make_data(data):
    pkt = b"\x01\x00"
    pkt += bytes([len(data) & 0xff])
    pkt += data
    chk = (256 - sum(pkt[1:])) & 0xff
    pkt += bytes([chk, 0x03])
    return pkt    

if __name__ == '__main__':        
    parser = argparse.ArgumentParser(prog='check_ssr_rh850')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-d', "--debugout", action='store_false', help='debug information out')
    parser.add_argument('-f', "--freq", required=True, type=int, help='CPU frequency(Hz)')
    args = parser.parse_args()

    conn = serial.Serial(args.port, 9600, dsrdtr=True, timeout=0.2)
    if args.debugout:
        debug = False
    else:
        debug = True
    freq = struct.pack(">I", args.freq) 
    print("\nMCU frequency =", args.freq, "(Hz)")
    while(1):
        conn.dtr = True # Set reset line
        time.sleep(0.1)
        conn.dtr = False # Release reset line
        time.sleep(0.2)

        conn.write(b'\x00' * 10 + b'\x55')
        time.sleep(0.1)
        resp = conn.read(2) 
        if debug:
            hexdump(resp)
        if resp[1:] != b'\xc1' :
            conn.close()
            conn = serial.Serial(args.port, dsrdtr=True, timeout=0.1)
            print("err: sync")
            continue

        time.sleep(0.1)
        # Get device type
        conn.write(b'\x01\x00\x01\x38\xc7\x03') # Device Type Acquisition Command
        resp = conn.read(6)
        if debug:
            hexdump(resp)        
        if  resp != b'\x81\x00\x01\x38\xc7\x03':
            print("err: dev type")
            exit()

        time.sleep(0.1)
        conn.write(b'\x81\x00\x01\x38\xc7\x03')
        resp = conn.read(30)
        if debug:
            hexdump(resp)        
        if  resp[0:4] != b'\x81\x00\x19\x38':
            print("err: dev type")
            exit()

        time.sleep(0.1)
        # Set frequency
        pkt = make_data(b'\x32'+ freq +resp[20:24])
        conn.write(pkt)
        resp = conn.read(7)
        if debug:
            hexdump(resp)        
        if resp != b'\x81\x00\x01\x32\xcd\x03':
            print("err: freq set")
            exit()

        time.sleep(0.2)
        conn.write(b'\x81\x00\x01\x32\xcd\x03')
        resp = conn.read(14)
        if debug:
            hexdump(resp)        
        if resp[0:4] != b'\x81\x00\x09\x32':
            print("err: freq check")
            exit()

        time.sleep(0.1)
        conn.write(b'\x01\x00\x05\x34\x00\x00\x25\x80\x22\x03')
        resp = conn.read(6)
        if debug:
            hexdump(resp)        
        if  resp != b'\x81\x00\x01\x34\xcb\x03':
            print("err: baudrate set")
            exit()  
        
        print("\n")

        time.sleep(0.1)
        # Synchronize!
        conn.write(b'\x01\x00\x01\x00\xff\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)         
        if resp == b'\x81\x00\x01\x00\xff\x03':
            print("Serial connection unlocked.")
            time.sleep(0.1)
        elif resp == b'\x81\x00\x02\x80\xDC\xA2\x03':
            print("Serial connection is prohibited for this device.")
            exit()
        else:            
            hexdump(resp)
            print("Unknown protection type.")
            exit()      

        # Read SSR
        time.sleep(0.1)
        conn.write(b'\x01\x00\x01\x21\xDE\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)
        if resp != b'\x81\x00\x01\x21\xDE\x03':
            print("err: read SSR", hex(resp[5])) 
        else:
            time.sleep(0.1)
            conn.write(resp)
            resp = conn.read(7)
            if debug:
                hexdump(resp) 
            if resp[0:4] != b'\x81\x00\x02\x21':                
                print("err: read SSR", hex(resp[5]))
                exit()
            else:
                ssr = resp[4]
                print("Security Status Register:", hex(ssr))
                if ssr & 0x80 > 0 :
                    print("   Read: OK")
                else:
                    print("   Read: Prohibited")
                if ssr & 0x40 > 0 :
                    print("   Write: OK")
                else:
                    print("   Write: Prohibited")
                if ssr & 0x20 > 0 :
                    print("   Erase: OK")
                else:
                    print("   Erase: Prohibited")
        exit()
        
