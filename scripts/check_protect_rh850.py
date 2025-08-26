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

def get_errcode(code):
    errdef = [[0xC0, 'Unsupported command error'],
              [0xC1, 'Packet error (Illegal length, Missing ETX, and so forth)'],
              [0xC2, 'Checksum error'],
              [0xC3, 'Flow error'],
              [0xD0, 'Address error'],
              [0xD4, 'Baud rate margin error'],
              [0xDA, 'Protection error'],
              [0xDB, 'ID mismatch error'],
              [0xDC, 'Serial programming disable error'],
              [0xE1, 'Erase error'],
              [0xE2, 'Write error'],
              [0xE3, 'Compare error'],
              [0xE7, 'Sequencer error']]
    st = '->Unknown error.'
    for i in range(0, len(errdef)):
        if code == errdef[i][0]:
            st = hex(code) + ' -> ' + errdef[i][1] +'.'
    return st

if __name__ == '__main__':
        
    parser = argparse.ArgumentParser(prog='dump_stm32f4xx')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-d', "--debugout", required=False, action='store_true', help='debug information out')
    parser.add_argument('-f', "--freq", required=False, type=int, help='CPU frequency(Hz)')
    parser.add_argument('-pass', "--password", required=False, type=str, help='ID code')    
    args = parser.parse_args()

    conn = serial.Serial(args.port, 9600, dsrdtr=True, timeout=0.2)
    
    if args.debugout == False:
        debug = False
    else:
        debug = True

    if args.freq == None:
        i = 16000000
    else:
        i = args.freq
    print("\nMCU frequency =", i, "(Hz)")
    freq = struct.pack(">I", i)
    
    if  args.password != None:
        st = args.password
    else:
        st = 'ffffffffffffffffffffffffffffffff'        
    if len(st) != 32:
        print('Password length error.')
        exit()
    print('Password is: ' + st)
    idcode = bytearray.fromhex(st)    
        
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
            conn = serial.Serial(args.port, dsrdtr=True, timeout=0.2)
            print("err: sync")
            continue

        # Get device type
        time.sleep(0.1)
        conn.write(b'\x01\x00\x01\x38\xc7\x03') # Device Type Acquisition Command
        resp = conn.read(6)
        if debug:
            hexdump(resp)        
        if  resp != b'\x81\x00\x01\x38\xc7\x03':
            print("err: dev type", get_errcode(resp[4]))
            exit()
        time.sleep(0.1)
        conn.write(b'\x81\x00\x01\x38\xc7\x03')
        resp = conn.read(30)
        if debug:
            hexdump(resp)        
        if  resp[0:4] != b'\x81\x00\x19\x38':
            print("err: dev type", get_errcode(resp[4]))
            exit()

        # Set frequency
        time.sleep(0.1)
        pkt = make_data(b'\x32'+ freq +resp[20:24])
        conn.write(pkt)
        resp = conn.read(7)
        if debug:
            hexdump(resp)        
        if resp != b'\x81\x00\x01\x32\xcd\x03':
            print("err: freq set", get_errcode(resp[4]))
            exit()
        time.sleep(0.2)
        conn.write(b'\x81\x00\x01\x32\xcd\x03')
        resp = conn.read(14)
        if debug:
            hexdump(resp)        
        if resp[0:4] != b'\x81\x00\x09\x32':
            print("err: freq check", get_errcode(resp[4]))
            exit()

        # Set baudrate
        time.sleep(0.1)
        conn.write(b'\x01\x00\x05\x34\x00\x00\x25\x80\x22\x03')
        resp = conn.read(6)
        if debug:
            hexdump(resp)        
        if  resp != b'\x81\x00\x01\x34\xcb\x03':
            print("err: baudrate set", get_errcode(resp[4]))
            exit()  

        # Synchronize!
        time.sleep(0.1)
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
            
        # Check IDCODE protection
        conn.write(b'\x01\x00\x01\x2C\xD3\x03')
        resp = conn.read(6) 
        if debug:
            hexdump(resp) 
        if resp[0:4] != b'\x81\x00\x01\x2C':
            print("err: Check IDCODE protection", get_errcode(resp[4]))
            exit() 
        time.sleep(0.1)
        conn.write(b'\x81\x00\x01\x2C\xD3\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)         
        if resp[0:4] != b'\x81\x00\x02\x2C':
            print("err: Check IDCODE protection", get_errcode(resp[4]))
            exit()
        if(resp[4]) != 0xFF:
            print("IDCODE Security active.")
            pkt = make_data(b'\x30' + idcode)
            conn.write(pkt)
            resp = conn.read(6)
            if debug:
                hexdump(resp)
            if resp != b'\x81\x00\x01\x30\xCF\x03':
                print("Password incorrect.", get_errcode(resp[4]))
                exit()
            else:
                print("Password accepted.")
        else:
            print("IDCODE protection disabled.")
       
        # Silicon Signature
        time.sleep(0.1)
        conn.write(b'\x01\x00\x01\x3A\xC5\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)         
        if resp != b'\x81\x00\x01\x3A\xC5\x03':
            print("err: read SS Error", get_errcode(resp[4]))
            exit()
        time.sleep(0.1)
        conn.write(b'\x81\x00\x01\x3A\xC5\x03')
        resp = conn.read(64)
        if debug:
            hexdump(resp) 
        if resp[0:4] != b'\x81\x00\x3B\x3A':
            print("err: read SS")
            exit()
        cfsize = 0x10000 + resp[33]*0x8000
        dfsize = int.from_bytes(resp[60:62], byteorder='big') * 0x40
        print("Signature:\n   MCU type: ", resp[4:20], "\n   CodeFlash size: ", hex(cfsize), "\n   DataFlash size: ", hex(dfsize))

        # Read SSR
        time.sleep(0.1)
        conn.write(b'\x01\x00\x01\x21\xDE\x03')
        resp = conn.read(7)
        if debug:
            hexdump(resp)
        if resp != b'\x81\x00\x01\x21\xDE\x03':
            print("err: Read SSR", get_errcode(resp[4])) 
        else:
            time.sleep(0.1)
            conn.write(resp)
            resp = conn.read(7)
            if debug:
                hexdump(resp) 
            if resp[0:4] != b'\x81\x00\x02\x21':
                print("err: Read SSR", get_errcode(resp[4]))
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
        
        # Check OCD code
        time.sleep(0.1)
        conn.write(b'\x01\x00\x01\x2B\xD4\x03')
        resp = conn.read(6) 
        if debug:
            hexdump(resp) 
        if resp[0:4] != b'\x81\x00\x01\x2B':
            print("err: Read OCD code", get_errcode(resp[4]))
            exit()    
        time.sleep(0.1)
        conn.write(b'\x81\x00\x01\x2B\xD4\x03')
        resp = conn.read(22)
        if debug:
            hexdump(resp)         
        if resp[0:4] != b'\x81\x00\x11\x2B':
            print("err: Read OCD code", get_errcode(resp[4]))
            exit()
        else:
            st = ''
            for i in range(4,20):
                st += hex(resp[i])[2:]
            print('IDCODE readed: ', st)
        exit()
        
