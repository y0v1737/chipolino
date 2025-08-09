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

idx_cmd = 0
idx_glitch = 0    

def exit_gracefully(signal, frame):
    print('\r\nStop signal')
    pcb.stop_command_send()
    sys.exit(0)

def exit_close():
    exit()

def do_dump():
    global idx_cmd
    global idx_glitch    
    comp_arr = open(args.comp, "rb").read(STM_CHUNK_SIZE)
    # print(comp_arr)
    cmd = "GLITCH_ADDR " + args.target + " " + str(args.offset[0]) + " " + str(args.offset[1]) + " " + str(args.width[0]) + " " + str(args.width[1]) + " "
    start_addr = int(args.addr[0], 16)
    while 1:
        cur_addr = start_addr
        while True:
            time.sleep(0.3)
            pcb.ser.read_all()
            request = cmd + "0x{:x}\n".format(cur_addr)
            pcb.ser.write(request.encode('utf-8'))   
            idx_cmd += 1    
            pcb.get_state_app()
            while pcb.is_glitching():
                pcb.get_state_app()
                sys.stdout.write('\r' + ' ' * 100 + '\r')
                sys.stdout.write("Dumping ---> Addr: 0x{:x}, Offset: {}/{}, Width: {}/{}, Log: {}".format(cur_addr, pcb.cur_offset, args.offset[1], pcb.cur_width, args.width[1], pcb.parse_log_info()))
                sys.stdout.flush()
                time.sleep(0.5)
            if pcb.is_glitc_succ():
                idx_glitch += 1
                pcb.stop_command_send()
                sys.stdout.write('\r' + ' ' * 100 + '\r')  # Очищение строки
                print("Dumped successfully Addr = {:x}, Offset: {}, Width: {}".format(cur_addr, pcb.cur_offset, pcb.cur_width))
                data, mem_addr  = pcb.get_memory_cmd(STM_CHUNK_SIZE)                
                hexdump(data)
                print("attempts = {}, glitch = {}".format(idx_cmd, idx_glitch))
                print("\n")  
                f = open(args.file, "wb")
                f.write(data)
                f.close()
                if comp_arr != data:
                    print("\r\ncmp buffers is incorrect\r\n")
                    pcb.stop_command_send()
                    sys.exit(0)   

        
if __name__ == '__main__':
    print("\n")
    parser = argparse.ArgumentParser(prog='dump_stm32f4xx')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-t', "--target", required=True, type=str, help='target')
    parser.add_argument('-o', "--offset", required=True, nargs='+', type=int) 
    parser.add_argument('-w', "--width", required=True, nargs='+', type=int)
    parser.add_argument('-a', "--addr", required=True, nargs='+', type=str)
    parser.add_argument('-f', "--file", required=True, type=str)
    parser.add_argument('-c', "--comp", required=True, type=str)

    args = parser.parse_args()

    signal.signal(signal.SIGINT, exit_gracefully)
    ser = serial.Serial(args.port, 115200)

    pcb = Chipolino(ser)

    if args.target:
        # assert (args.target in targets_list)
        assert (args.target in pcb.targets_list)
        if args.offset:
            assert (len(args.offset) == 2)
            assert (args.offset[0] <= args.offset[1])
            if args.width:
                assert (len(args.width) == 2)
                assert (args.width[0] <= args.width[1])
                if args.addr:
                    if args.file:
                        pcb.stop_command_send()
                        do_dump()
                    else:
                        print("Need set OUTPUT FILE (-f/--file)")
                else:
                    print("Need set ADDR (-a/--addr) <start_addr>")
            else:
                print("Need set WIDTH (-w/--width) <start_width, end_width>")
        else:
            print("Need set OFFSET (-o/--offset) <start_offset, end_offset>")
    else:
        print("Need set TARGET (-t/--target)")
    exit_close()  


