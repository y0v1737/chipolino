# Chip'olino: The Playbook

**Chip'olino** is a hardware-software platform designed for performing glitch attacks via power supply. It includes the main Chip'olino board and a set of addons for various microcontrollers. Chip'olino addresses two main tasks:
- Performing known power supply attacks on microcontrollers.    
- Research and experiments in the field of power supply attacks.

### Targets
Currently, the project includes algorithms for conducting attacks on:
* [nRF52 (The entire nRF5 family)](nRF52.md)
* [Renesas RH850 (The entire RH850 family)](Renesas.md)
* [NXP (LPC214x, LPC134x)](NXP.md)
* [STM32 (Different series of chips)](STM32.md)

In addition to the main algorithms, the repository contains auxiliary scripts for locking test chips, reading firmware, erasing the chip, and determining the current protection level.


![600](pics/pcb_pics/ALLIN2.png)
##### Author's note.
Chip'olino implements algorithms for unlocking a large number of chips, but this does not eliminate the need to read the documentation and thoroughly understand what is happening. One needs to understand both how the protection of the target MCU works and how the attack is carried out. Power supply attacks are a complex and fascinating topic with many nuances.

Below is the functional diagram of the Chip'olino.

![](Chipolino_logic_sch/Chipolino_logic_sch.png)

To carry out an attack on the MCU, it needs to be desoldered and soldered onto a ready-made addon, which can be connected to the main board. This approach allows for precise control of the timing parameters of the attack and significantly improves the repeatability of the attack.

### Features
* **External power supply for attacks:** Support for external power supply (0–5V, 100mA) through the X2 connector (External Target Supply). This is useful when the built-in power source is insufficient or when a lower voltage is required (e.g., for some chips that require less than the standard 3.3V). It allows you to connect a lab power supply and experiment with different power levels.    
- **High-speed SWD interface:** The implementation of SWD on the PIO in the Chip'olino module ensures direct and fast interaction with microcontrollers (NRF52, STM32, etc.) without the delays typical of USB programmers (J-Link and similar), speeding up command cycling by hundreds of times.    
- **SWD multiplexer:** The ability to switch between the built-in SWD interface and an external debugger connected to the X11 connector, which is convenient for quickly connecting an external programmer right after the attack is completed.    
- **Variety of power sources:** Several built-in power sources with independent control and support for external 5V power supply for the entire device, reducing the impact of noise and interference.    
- **Universal addon:** A module for connecting and testing any boards and devices.    
- **Auxiliary scripts:** A set of ready-made scripts for automating tasks and simplifying work with microcontrollers.

##### Note
```
All values for pulse widths and offset shifts for glitches provided in the instructions are based on the RP2040 clock frequency of 250 MHz.
```

### Files

```
.
├── The_Playbook
│   └── Chipolino_The_Playbook.md    # Chip'olino Playbook 
├── fw                               # Chip'olino firmware project 
│   ├── includes
│   ├── pio_sm
│   └── targets
├── pcb                              # PCB projects
│   ├── ChipOlino_revA               # Chip'olino PCB project
│   ├── Nordic                       # Nordic addons PCB
│   │   ├── nRF52832-CIAA
│   │   ├── nRF52840-QIAA
│   │   └── nRF52833-QDAA
│   ├── NXP                          # NXP addons PCB
│   │   ├── LPC1343FBD48_revA
│   │   └── LPC2148FBD64_revA
│   ├── STM                          # STM addons PCB
│   │   ├── STM32F446RCT6_revA
│   │   └── STM32F411CCU_revA
│   ├── Renesas                      # Renesas addons PCB
│   │   └── RH850_F1L_revB   
│   ├── template_addon_revA          # template addon project
│   └── universal_addon_revA         # universal addon project
├── scripts                           
│   ├── check_ssr_rh850.py    
│   ├── chipctrl.py                  # Chip'olino control script (main for use)
│   ├── chipolino.py                 # class for Chip'olino board 
│   ├── dump_rh850.py                
│   ├── dump_stm32.py 
│   ├── dump_lpc1343.py
│   ├── dump_lpc2148.py
│   ├── erase_lpc2148.py
│   ├── erase_lpc1343.py
│   ├── erase_rh850.py               
│   ├── requirements.txt             # pip install -r requirements.txt
│   └── test_erase_stm32.py          
└── ...
```

### Quick start

1) Install all the necessary packages:
```
pip install -r ./scripts/requirements.txt
```
2) Read **The Playbook**.