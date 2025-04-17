import sys
import serial
import struct
import time
import signal 
import binascii
import argparse
from hexdump import hexdump
from rich.progress import Progress, SpinnerColumn, TextColumn, TimeElapsedColumn

progress = Progress(
    SpinnerColumn(),
    TextColumn("{task.description}"),
    TimeElapsedColumn(),
    # transient=True,
)

CHUNK_SIZE = 0x200

class Chipolino:
    def __init__(self, ser):
        self.ser = ser
        self.get_state_app()
        self.state
        self.cur_width
        self.cur_offset
        self.log_info = 0

    targets_list = ["stm32f401_spi_rdp1", "stm32f401_rdp2", 
                    "stm32f407_uart_rdp1", "stm32f407_rdp2",
                    "stm32f427_spi_rdp1", "stm32f427_rdp2",
                    "stm32f429_spi_rdp1", "stm32f429_rdp2",
                    "stm32f446_spi_rdp1", "stm32f446_rdp2",
                    "stm32h730_uart_rdp1", "stm32h730_rdp2",
                    "lpc2148", "lpc1343", 
                    "nrf52_mosfet",
                    "rh850", "rh850_rp"]

    def send_cmd(self, cmd:bytearray):
        self.ser.write(cmd + b"\n")
        self.ser.read(len(cmd))

    def parse_log_info(self):
        if len(self.log_info) % 2 == 0:
            try:
                pkt = binascii.unhexlify(self.log_info)
            except binascii.Error as err:
                return self.log_info            
            if pkt[0] == 0x55:
                if len(pkt) >= 3:
                    len_ = pkt[1] 
                    type_ = pkt[2] 
                    data = pkt[3:]
                    if type_ == 1: # STRING
                        return data.decode("utf-8")
                    if type_ == 2: # RAW
                        return binascii.hexlify(data)
        return self.log_info

    def bootmode(self):
        self.send_cmd(b"BOOTROM")

    def gpio_set_cmd(self, gp, state):
        self.send_cmd(b"GPIO " + str(gp).encode('utf-8') + b" " + str(state).encode('utf-8'))

    def step_cmd(self, step):
        self.send_cmd(b"STEP " + str(step).encode('utf-8'))

    def attempt_cmd(self, attempt):
        self.send_cmd(b"ATTEMPT " + str(attempt).encode('utf-8'))

    def set_trigger_vcc(self, trig):
        if trig == "1v2": v= b"1v2"
        elif trig == "1v8": v= b"1v8"
        elif trig == "3v3": v= b"3v3"
        else: v= b"Z"
        self.send_cmd(b"TRIG " + v)

    def set_external_swd(self, swd):
        if swd == "int": self.send_cmd(b"SWD 0")
        elif swd == "ext": self.send_cmd(b"SWD 1")
        else: print("ERROR: swd value  1 or 0. 1 - external, 0 - pico swd.")

    def get_board_status(self):
        self.send_cmd(b"STATUS")
        l = self.ser.readline()
        print(l)
        while l:
            l = self.ser.readline()
            print(l)
            if l == b"--- status end ---\r\n":
                break

    def get_state_app(self):
        self.send_cmd(b"STATE")
        l = self.ser.readline()
        i = 0
        while l:
            i += 1
            l = self.ser.readline()
            if l == b"--- state end ---\r\n":
                break
            if l[:7] == b"state: ":
                self.state = int(l.decode('utf-8').split(" ")[1].split("\r")[0], 16)
            if l[:12] == b"cur_offset: ":
                self.cur_offset = int(l.decode('utf-8').split(" ")[1].split("\r")[0], 10)
            if l[:11] == b"cur_width: ":
                self.cur_width = int(l.decode('utf-8').split(" ")[1].split("\r")[0], 10)
            if l[:10] == b"log_info: ":
                self.log_info = l.decode('utf-8').split(" ")[1].split("\r")[0]

    def is_stop(self):
        if self.state & (1 << 0): return True 
        else: return False

    def is_glitching(self):
        if self.state & (1 << 2): return True 
        else: return False

    def is_glitc_succ(self):
        if self.state & (1 << 4): return True 
        else: return False

    def is_synced(self):
        if self.state & (1 << 3): return True 
        else: return False

    def stop_command_send(self):
        self.get_state_app()
        while not self.is_stop():
            self.send_cmd(b"STOP")
            time.sleep(0.05)
            self.get_state_app()

    def get_memory_cmd(self, size):
        assert (size <= CHUNK_SIZE)
        data = bytearray()
        self.ser.write("GET_MEMORY ".encode('utf-8'))
        l = self.ser.readline()
        # print(l)
        i = 0
        while l:
            i += 1
            l = self.ser.readline()
            if l == b"--- get memory end ---\r\n":
                break
            if l[:5] == b"addr ":
                mem_addr = int(l[5:], 16)
            if l[:4] == b"mem ":
                mem_array = l[4:]
                for i in range(size):
                    data.append(int(mem_array[:2], 16))
                    mem_array = mem_array[2:]
        return data, mem_addr

    def do_glitch(self, target, offset_start, offset_end, width_start, width_end):
        cmd = "GLITCH " + target + " " + str(offset_start) + " " + str(offset_end) + " " + str(width_start) + " " + str(width_end)
        self.send_cmd(cmd.encode('utf-8'))
        time.sleep(0.4)
        self.get_state_app()
        if self.is_synced():
            print("Target synchronized")
        else:
            print("Target not synchronized")
            return 0
        self.get_state_app()
        with progress:
            task = progress.add_task("GLITCH...")
            while self.is_glitching():
                descr = "Glitching ---> Offset: {}/{}, Width: {}/{}, Log: {}".format(self.cur_offset, offset_end, self.cur_width, width_end, self.parse_log_info())
                progress.update(task, description=descr)
                time.sleep(0.5)
                self.get_state_app()        
            self.get_state_app()  
            descr = "Glitching ---> Offset: {}/{}, Width: {}/{}, Log: {}".format(self.cur_offset, offset_end, self.cur_width, width_end, self.parse_log_info())
            progress.update(task, description=descr)

        self.stop_command_send()
        if self.is_glitc_succ():
            print("\nGlitch successed\nOffset: {}, Width: {}".format(self.cur_offset, self.cur_width))
        else:
            print("\nGlitch failed\nOffset: {}, Width: {}".format(self.cur_offset, self.cur_width))

    def do_dump(self, target, offset_start, offset_end, width_start, width_end, addr_start, addr_end, chunk_size, file_out):
        f = open(file_out, "wb")
        cmd = "GLITCH_ADDR " + target + " " + str(offset_start) + " " + str(offset_end) + " " + str(width_start) + " " + str(width_end) + " "
        start_addr = int(addr_start, 16)
        end_addr = int(addr_end, 16)
        cur_addr = start_addr
        with progress:
            task = progress.add_task("DUMP...")
            while cur_addr < end_addr:
                time.sleep(0.3)
                self.ser.read_all()
                request = cmd + "0x{:x}\n".format(cur_addr)
                self.ser.write(request.encode('utf-8'))     
                self.get_state_app()
                while self.is_glitching(): 
                    self.get_state_app()
                    descr = "Dumping ---> Addr: 0x{:x}/0x{:x}, Offset: {}/{}, Width: {}/{}, Log: {}".format(cur_addr, end_addr, self.cur_offset, offset_end, self.cur_width, width_end, self.parse_log_info())
                    progress.update(task, description=descr)
                    time.sleep(0.5)
                if self.is_glitc_succ():
                    self.stop_command_send()
                    sys.stdout.write('\r' + ' ' * 100 + '\r')  # Очищение строки
                    print("Dumped successfully Addr = {:x}, Offset: {}, Width: {}".format(cur_addr, self.cur_offset, self.cur_width))
                    data, mem_addr = self.get_memory_cmd(chunk_size) 
                    print("addr 0x{:x}".format(mem_addr))               
                    hexdump(data)
                    print("\n")
                    f.write(data)
                    f.flush()
                    cur_addr += chunk_size
            self.stop_command_send()