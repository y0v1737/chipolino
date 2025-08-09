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

if __name__ == '__main__':
    print("\n")
    
    parser = argparse.ArgumentParser(prog='dump_RH850')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-a', "--addr", required=True, nargs='+', type=str)
    parser.add_argument('-f', "--file", required=True, type=str)
    parser.add_argument('-imm', "--immediately", required=False, action='store_true', help='dump immediately')
    args = parser.parse_args()

    conn = serial.Serial(args.port, 9600, timeout=0.1)
    f = open(args.file, 'wb')

    start = int(args.addr[0], 16)
    end = int(args.addr[1], 16)

    assert ((start % 0x400) == 0)
    assert (((end + 1) % 0x400) == 0)

    time.sleep(0.1)
    conn.read(conn.in_waiting)
    time.sleep(0.1)
    
    print("\nTry dump range 0x{:x}-0x{:x} to {} file\r\n".format(start, end, args.file))

    if args.immediately == False:
        pkt = make_pkt(b'\x15' + struct.pack('>II', start, end))
        hexdump(pkt)
        conn.write(pkt)
        time.sleep(0.1)
        resp = conn.read(conn.in_waiting)
        hexdump(resp)
        if resp != b'\x81\x00\x01\x15\xea\x03':
            print("Bad read response. Try to glitch.")
            exit()            

    rem = end - start
    curaddr = start
    data = b""
    all_data = b""
    while rem > 0:
        conn.write(b'\x81\x00\x01\x15\xea\x03')
        length = conn.read(3) 
        if  length != b'\x81\x04\x01':
            hexdump(length)
            print("Bad glitch. Try again.")
            exit()            
        else:
            print("\nDumping address 0x{:X}:".format(curaddr))
            
        length = struct.unpack('!H', length[1:])[0] - 1
        print("Waiting for {:x} bytes".format(length))
        resp = b""
        while len(resp) < length + 3: 
            resp += conn.read(conn.in_waiting)
            time.sleep(0.05)

        print("Got {:x}, remainder {:x}\n".format(length, rem+1))
        hexdump(resp)
        data = resp[1:-2]
        f.write(data)
        f.flush()
        all_data += data
        rem -= length 
        curaddr += 0x400

print("\nDump range 0x{:x}-0x{:x} to {} file. Read data size 0x{:x}\r\n".format(start, end, args.file, len(all_data)))
