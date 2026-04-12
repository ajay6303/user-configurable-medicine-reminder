# User-Configurable Medicine Reminder System
An embedded systems project built on the **ARM7 (LPC2148)** architecture to help patients track medication timing.

## Features
* **Real-Time Tracking:** Uses internal RTC to track time, day, and date.
* **Customizable Alarms:** Users can set up to 3 different medicine timings via a 4x4 Keypad.
* **Smart Alert:** 60-second buzzer alert with a countdown timer on the LCD.
* **Non-Blocking Logic:** Designed with polling-based timing to keep the system responsive.

## Hardware Components
* Microcontroller: LPC2148 (ARM7 TDMI-S)
* Display: 16x2 LCD (8-bit mode)
* Input: 4x4 Matrix Keypad
* Indicator: Piezo Buzzer

## Tools Used
* **Keil µVision4:** For C coding and HEX file generation.
* **Proteus 8:** For circuit simulation and testing.
* **GCC & GDB:** Used for cross-compilation and debugging.

## How to Run
1. Load the `.hex` file into the LPC2148 in Proteus.
2. Press 'Play' to see the real-time clock.
3. Use the external interrupts to enter the configuration menu.

![Circuit Diagram](project_circuit.jpeg)
