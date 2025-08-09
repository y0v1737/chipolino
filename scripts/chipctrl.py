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

def exit_close():
    # SERP.close()
    exit()

def exit_gracefully(signal, frame):
    print('\r\nStop signal')
    pcb.stop_command_send()
    sys.exit(0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='chipolino')
    parser.add_argument('-p', "--port", required=True, type=str, help='serial port')
    parser.add_argument('-b', "--bootrom", help='bootrom', action='store_true')
    parser.add_argument('-r', help='reboot')
    
    parser.add_argument('-status', action='store_true', help='status command')
    parser.add_argument('-g', "--glitch", action='store_true', help='glitch command')
    parser.add_argument('-t', "--target", type=str, help='target')
    parser.add_argument('-o', "--offset", nargs='+', type=int) 
    parser.add_argument('-w', "--width", nargs='+', type=int)
    parser.add_argument('-gp', '--gpio', nargs='+', type=int) 
    parser.add_argument('-step', type=int) 
    parser.add_argument('-attempt', type=int) 
    parser.add_argument('-stop', action='store_true',)
    parser.add_argument('-state', action='store_true',)
    parser.add_argument('-dump', action='store_true',)
    parser.add_argument('-swd',  type=str)
    parser.add_argument('-trig', type=str)
    args = parser.parse_args()

    signal.signal(signal.SIGINT, exit_gracefully)
    ser = serial.Serial(args.port, 115200)

    pcb = Chipolino(ser)

    if args.bootrom:
        pcb.bootmode()
        exit_close()
    
    if args.trig:
        pcb.set_trigger_vcc(args.trig)
        exit_close()  
    
    if args.swd:
        pcb.set_external_swd(args.swd)
        exit_close() 

    if args.gpio:
        assert (len(args.gpio) == 2)
        assert (args.gpio[0] < 28)
        assert (args.gpio[1] == 0 or args.gpio[1] == 1)
        pcb.gpio_set_cmd(args.gpio[0], args.gpio[1])
        exit_close()

    if args.step:
        pcb.step_cmd(args.step)
        exit_close()

    if args.attempt:
        pcb.attempt_cmd(args.attempt)
        exit_close()

    if args.status:
        pcb.get_board_status()
        exit_close()  

    if args.state:
        pcb.get_state_app()
        print(pcb.is_glitching())
        exit_close()  

    if args.stop:
        pcb.stop_command_send()
        exit()

    if args.dump:
        data, mem_addr = pcb.get_memory_cmd(0x200) 
        print("Memory addr 0x{:x}".format(mem_addr))               
        hexdump(data)
        exit_close()
        exit() 

    if args.glitch:
        if args.target:
            assert (args.target in pcb.targets_list)
            if args.offset:
                assert (len(args.offset) == 2)
                assert (args.offset[0] <= args.offset[1])
                if args.width:
                    assert (len(args.width) == 2)
                    assert (args.width[0] <= args.width[1])
                    pcb.do_glitch(args.target, args.offset[0], args.offset[1], args.width[0], args.width[1])
                else:
                    print("Need set WIDTH (-w/--width) <start_width, end_width>")
            else:
                print("Need set OFFSET (-o/--offset) <start_offset, end_offset>")
        else:
            print("Need set TARGET (-t/--target)")
        exit_close()    
