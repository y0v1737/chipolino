import serial
import time
import struct
from hexdump import hexdump
import argparse

def make_pkt(data):
    pkt = b"\x01"
    pkt += struct.pack('!H', len(data))
    pkt += data
    # Checksum
    chk = (256 - sum(pkt[1:])) & 0xff
    pkt += bytes([chk, 0x03])
    return pkt

def make_data(data):
    pkt = b"\x01\x00"
    pkt += bytes([len(data) & 0xff])
    pkt += data
    chk = (256 - sum(pkt[1:])) & 0xff
    pkt += bytes([chk, 0x03])
    return pkt

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='erase_rh850')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-i', "--init", required=True, type=int, help='startup initialization')
    parser.add_argument('-d', "--debugout", action='store_false', help='debug information out')
    args = parser.parse_args()
    # conn = serial.Serial(args.port, 9600, dsrdtr=True, timeout=0.2)
    conn = serial.Serial(args.port, 9600)
    if args.debugout:
        debug = False
    else:
        debug = True    

    conn.read_all()

    if args.init == 1 :
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
            conn = serial.Serial(args.port, dsrdtr=True, timeout=0.2)
            print("err: sync")
            #continue

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
        pkt = make_data(b'\x32\x00\xf4\x24\x00'+resp[20:24])
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
            time.sleep(0.1)
        # Synchronize!
        conn.write(b'\x01\x00\x01\x00\xff\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)         
        if resp == b'\x81\x00\x01\x00\xff\x03':
            print("Serial connection unlocked.")
        elif resp == b'\x81\x00\x02\x80\xDC\xA2\x03':
            print("Serial connection is prohibited for this device.")
        else:            
            print("Unknown protection type.")
        time.sleep(0.1)
    #init end


    # Check OCD protection
    conn.write(b'\x01\x00\x01\x2C\xD3\x03')
    resp = conn.read(7) 
    if debug:
        hexdump(resp) 
    if resp[0:4] != b'\x81\x00\x01\x2C':
        print("err: Check OCD protection - stage 1")
        exit() 

    time.sleep(0.1)
    conn.write(b'\x81\x00\x01\x2C\xD3\x03')
    resp = conn.read(7)
    if debug:
        hexdump(resp)         
    if resp[0:4] != b'\x81\x00\x02\x2C':
        print("err: Check OCD protection - stage 2")
        exit()

    #Try default OCD code
    if(resp[4]) != 0xFF:
        print("OCD Security active.")
        pkt = make_data(b'\x30' + b'\xFF' * 16)
        conn.write(pkt)
        resp = conn.read(6)
        if debug:
            hexdump(resp)
        if resp != b'\x81\x00\x01\x30\xCF\x03':
            print("Defaul password incorrect.")
            exit()
        else:
            print("Defaul password accepted.")
    else:
        print("OCD protection disabled.")


    time.sleep(0.1)        
    # Silicon Signature
    conn.write(b'\x01\x00\x01\x3A\xC5\x03')
    resp = conn.read(7)
    if debug:
        hexdump(resp)         
    if resp != b'\x81\x00\x01\x3A\xC5\x03':
        print("err: read SS. Error", hex(resp[4]))
        exit()

    time.sleep(0.1)
    conn.write(b'\x81\x00\x01\x3A\xC5\x03')
    resp = conn.read(64)
    if debug:
        hexdump(resp) 
    if resp[0:4] != b'\x81\x00\x3B\x3A':
        print("err: read SS st2")
        exit()
    flsize = 0x10000 + resp[33]*0x8000
    print("Signature:\n   MCU type: ", resp[4:20], "\n   CodeFlash size:", hex(flsize))    
    # Chip full erase
    print("\nChip full erase started. Please wait...", end="")
    # Memery define: codeflash, dataflash, userboot -> [start, end, step]
    memdef = [[0, flsize, 0x8000],[0xFF200000, 0xFF208000, 0x40],[0x01000000, 0x01008000, 0x8000]]
    for i in range(0, len(memdef)):
        j = memdef[i][0]
        print(".",end="")
        while j<memdef[i][1] :
            pkt = make_pkt(b'\x12' + struct.pack('>I', j))
            conn.write(pkt)
            time.sleep(0.1)
            resp = conn.read(6)
            if debug:
                hexdump(resp)
            if resp != b'\x81\x00\x01\x12\xED\x03':
                print("err: erase at addr:", hex(j))
                exit()
            if j < 0x10000 :
                j += 0x2000
            else:
                j += memdef[i][2]
    time.sleep(0.1)
    # DoItNew!
    conn.write(b'\x01\x00\x01\x1C\xE3\x03')
    time.sleep(0.8)
    resp = conn.read(6) 
    if debug:
        hexdump(resp)

    print("\n")
    if resp != b'\x81\x00\x01\x1C\xE3\x03':
        print("ERR: Renew chip fail")
        exit()
    else:
        print("Chip erased sucsessfully.")

