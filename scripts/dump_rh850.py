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
    parser = argparse.ArgumentParser(prog='dump_stm32f4xx')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-a', "--addr", required=True, nargs='+', type=str)
    parser.add_argument('-f', "--file", required=True, type=str)
    args = parser.parse_args()
    
    conn = serial.Serial(args.port, 9600, timeout=0.1)
    f = open(args.file, 'wb')
    
    start = int(args.addr[0], 16)
    end = int(args.addr[1], 16)

    assert ((start % 0x1000) == 0)
    assert (((end + 1) % 0x1000) == 0)

    time.sleep(0.1)
    conn.read(conn.in_waiting)
    time.sleep(0.1)
    
    print("\nTry dump range 0x{:x}-0x{:x} to {} file\r\n".format(start, end, args.file))
    pkt = make_pkt(b'\x15' + struct.pack('>II', start, end))
    hexdump(pkt)
    conn.write(pkt)
    time.sleep(0.1)

    resp = conn.read(conn.in_waiting)
    hexdump(resp)

    assert (b'\x81\x00\x01\x15\xea\x03' in resp)

    rem = end - start
    data = b""
    all_data = b""
    while rem > 0:
        conn.write(b'\x81\x00\x01\x15\xea\x03')

        length = conn.read(3) 
        length = struct.unpack('!H', length[1:])[0] - 1

        print("\nwaiting for {:x} bytes".format(length))
        resp = b""
        while len(resp) < length + 3: 
            resp += conn.read(conn.in_waiting)
            time.sleep(0.05)

        print("Got {:x}, remainder {:x}".format(length, rem))
        hexdump(resp)
        data = resp[1:-2]
        f.write(data)
        f.flush()
        all_data += data
        rem -= length 

print("\nDump range 0x{:x}-0x{:x} to {} file. Read data size 0x{:x}\r\n".format(start, end, args.file, len(all_data)))
