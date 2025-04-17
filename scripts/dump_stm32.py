import sys
import serial
import struct
import time
import signal 
import binascii
import argparse
from hexdump import hexdump
from chipolino import Chipolino

pcb = None

STM_CHUNK_SIZE = 0x100

targets_list = ["stm32f401_spi_rdp1",
                "stm32f407_uart_rdp1",
                "stm32f427_spi_rdp1",
                "stm32f429_spi_rdp1",
                "stm32f446_spi_rdp1",
                ]

def exit_gracefully(signal, frame):
    print('\r\nStop signal')
    pcb.stop_command_send()
    sys.exit(0)

def exit_close():
    exit()
        
if __name__ == '__main__':
    print("\n")
    parser = argparse.ArgumentParser(prog='dump_stm32f4xx')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-t', "--target", required=True, type=str, help='target')
    parser.add_argument('-o', "--offset", required=True, nargs='+', type=int) 
    parser.add_argument('-w', "--width", required=True, nargs='+', type=int)
    parser.add_argument('-a', "--addr", required=True, nargs='+', type=str)
    parser.add_argument('-f', "--file", required=True, type=str)
    args = parser.parse_args()

    signal.signal(signal.SIGINT, exit_gracefully)
    ser = serial.Serial(args.port, 115200)

    pcb = Chipolino(ser)

    if args.target:
        assert (args.target in targets_list)
        if args.offset:
            assert (len(args.offset) == 2)
            assert (args.offset[0] <= args.offset[1])
            if args.width:
                assert (len(args.width) == 2)
                assert (args.width[0] <= args.width[1])
                if args.addr:
                    assert (len(args.addr) == 2)
                    assert (int(args.addr[0], 16) <= int(args.addr[1], 16))
                    if args.file:
                        pcb.do_dump(args.target, args.offset[0], args.offset[1], args.width[0], args.width[1], args.addr[0], args.addr[1], STM_CHUNK_SIZE, args.file)
                    else:
                        print("Need set OUTPUT FILE (-f/--file)")
                else:
                    print("Need set ADDR (-a/--addr) <start_addr, end_addr>")
            else:
                print("Need set WIDTH (-w/--width) <start_width, end_width>")
        else:
            print("Need set OFFSET (-o/--offset) <start_offset, end_offset>")
    else:
        print("Need set TARGET (-t/--target)")
    exit_close()  


