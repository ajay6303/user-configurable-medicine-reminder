# 💊 User-Configurable Medicine Reminder System

[![Platform: ARM7 LPC2148](https://img.shields.io/badge/Platform-ARM7%20LPC2148-blue?style=for-the-badge&logo=arm)](https://www.arm.com/)
[![Toolchain: Keil uVision 4](https://img.shields.io/badge/Toolchain-Keil%20%CE%BCVision%204-orange?style=for-the-badge)](https://www.keil.com/)
[![Simulation: Proteus 8](https://img.shields.io/badge/Simulation-Proteus%208-brightgreen?style=for-the-badge)](https://www.labcenter.com/)
[![Flashing: Flash Magic](https://img.shields.io/badge/Flashing-Flash%20Magic-red?style=for-the-badge)](https://www.flashmagictool.com/)

An advanced, reliable, and user-configurable firmware solution deployed on the **ARM7 (LPC2148)** microcontroller architecture. This system acts as a real-time assistive medical device, allowing patients or caregivers to schedule medication times dynamically via an on-board hardware interface, completely eliminating reliance on external smart devices or internet connectivity.

---

## 📌 Project Overview & Aim

The core objective of this project is to build a standalone **User-Configurable Medicine Reminder System** that synchronizes real-time clock data with flash/controller memory registers to track medication compliance. 

### 🎯 Key Objectives:
* **Live Synchronization:** Continuously read and render real-time calendar and clock data from the internal RTC onto a 16x2 alphanumeric display.
* **Dynamic Configuration:** Intercept normal execution using hardware interrupts to allow users to set custom alarms.
* **Active Alerting:** Emit physical multi-frequency audio patterns via a Piezo buzzer combined with synchronized visual alerts when scheduled times match the clock.
* **Fail-Safe Timeout:** Maintain a strict 60-second non-blocking countdown window for manual user acknowledgment, falling back to autonomous safety recovery if unanswered.

---
## 🏗️ System Architecture & Block Diagram

The system coordinates several synchronous and asynchronous peripherals through the LPC2148 central processing unit:

![image alt](https://github.com/ajay6303/user-configurable-medicine-reminder/blob/bf6723d0c720efc397452e1e6c79a8063e58c299/interface.png)

---
### 🛠️ Core Hardware Requirements

* 🧠 **LPC2148 Microcontroller:** 32-bit ARM7TDMI-S core, 512KB on-chip Flash, 40KB RAM, integrated RTC.
* 📺 **16x2 Alphanumeric LCD:** Liquid crystal display configured in high-throughput 8-bit parallel interfacing mode.
* ⌨️ **4x4 Matrix Keypad:** 16-key matrix layout utilized to scan user numeric inputs without exhausting GPIO pins.
* 🔔 **Piezo Buzzer:** High-decibel audio alert transducer driven via a transistor amplification switch.
* 🔘 **Tactile Push Buttons:** Hardware switches wired with pull-up networks dedicated to low-latency external interrupts.
* 🔌 **USB-to-UART Converter (CP2102 / PL2303):** For ISP (In-System Programming) via Flash Magic.

---

### 📍 Physical Microcontroller Pin Interfacing Map

| Peripheral Module | LPC2148 Pin | Direction | Functional Description |
| :--- | :--- | :--- | :--- |
| 🔘 **Switch 1 (Setup)** | `P0.1 / EINT0` | **INPUT** | Triggers Mode Transition to Configuration Menu |
| 🔘 **Switch 2 (Ack)** | `P0.3 / EINT1` | **INPUT** | Acknowledges / Silences Active Medicine Alarm |
| 🔔 **Piezo Buzzer** | `P0.23` | **OUTPUT** | Drives Audio Alarm Waveform (Active High) |
| 📺 **16x2 LCD Data** | `P0.8` to `P0.15` | **OUTPUT** | Transmits Character Parallel ASCII Data Bytes (8-Bit Mode) |
| 🎛️ **16x2 LCD Control** | `P0.18`, `P0.17`, `P0.19` | **OUTPUT** | Controls Register Select (RS), Read/Write (RW), and Enable (E) |
| ⌨️ **4x4 Keypad Rows** | `P1.16` to `P1.19` | **OUTPUT** | Sequential Scanning Drive Rows |
| ⌨️ **4x4 Keypad Cols** | `P1.20` to `P1.23` | **INPUT** | Monitored Input Pins with Internal Pull-Ups |
## ⚙️ Low-Level Driver Architecture & Firmware Design

The firmware is engineered modularly, featuring isolated hardware abstraction layer (HAL) driver files:

### 📺 1. LCD Driver Module (`LCD.h`)
Manages custom delay loops, command latching routines, and type conversions for raw float/integer data rendering.
* `void InitLCD(void);` → Initializes pins, sets 8-bit mode cursor configurations, and wipes GDRAM.
* `void CmdLCD(unsigned char cmd);` → Drives `RS=0` to issue instructions (e.g., cursor shift, clear screen).
* `void CharLCD(unsigned char ascii);` → Drives `RS=1` to project an explicit ASCII byte onto the active address.
* `void FloatLCD(float dec, unsigned char DP);` → Breaks float values down into integer and fractional components for display.

### ⌨️ 2. Keypad Matrix Module (`KPM.h`)
Implements a non-blocking matrix scanning state-machine to evaluate switch coordinates.
* `int RowCheck(void);` / `int ColCheck(void);` → Performs validation scanning to decouple bounce noise.
* `int ColScan(void);` → Pinpoints the logic-low vector intersecting the row driver line.
* `char KeyScan(void);` → Aggregates coordinates into an alphanumeric return literal (`0-9`, `*`, `#`).

### ⚡ 3. Nested Vectored Interrupt Controller Module (`interrupt.h`)
Handles real-time priority context switching for critical user inputs.
* `void Init_intrrupt(void);` → Sets up `PINSEL0` registers to configure `EINT0` and `EINT1` channels.
* `void eint0_isr(void) __irq;` → Asynchronous interrupt sequence for **Switch 1**. Suspends standard background routines to inject configuration operations.
* `void eint1_isr(void) __irq;` → Low-latency interrupt response for **Switch 2** to silence the buzzer immediately.

---

![image alt](https://github.com/ajay6303/user-configurable-medicine-reminder/blob/662f990da34c75aba6c64095cd84b48c8aa50fce/mini%20project%20flow%20chart.png)

---
## 📈 Project Advantages vs. Real-World Limitations

### 🟢 Value-Added Benefits
* **Zero Connectivity Dependency:** Immune to Wi-Fi drops, routing failures, or smartphone battery issues—ideal for elderly patients.
* **True Real-Time Determinism:** Built on bare-metal C using direct Vectored Interrupt Controller (VIC) registers, guaranteeing an immediate response.
* **Low Power Footprint:** Optimized ARM7 peripheral clock states draw minimal milliamps, enabling extended standby using a small backup battery cell.
* **Intuitive Interface:** Simple dual-button override pathways streamline interaction for non-technical users.

### 🔴 Encountered Bottlenecks & Design Challenges
* **Volatile Matrix Ghosting:** Fast sequential keypad entry could occasionally lead to misread inputs. This was fixed by reinforcing software debouncing algorithms and delay barriers inside `ColCheck()`.
* **Blocking Display Overhead:** Massive multi-character loops in the `StrLCD()` function could delay real-time background ticks. This was solved by migrating heavy strings to a localized buffer.
* **Atomic Register Protection:** Adjusting RTC settings while an interrupt routine concurrently checked time configurations posed potential race conditions. This was resolved by wrapping adjustments in safe critical-section routines.

---

## 💻 Technical Setup, Deployment & Verification

### 🔌 Software Simulation Testing (Proteus)
1. Open **Proteus 8 Professional** and load the hardware schematics configuration design file.
2. Double-click the central **LPC2148 MCU schematic block** to open properties.
3. In the *Program File* entry pathway, map to the compiled `.hex` asset located inside your Keil build distribution folder.
4. Set the clock frequency parameter to match your crystal (typically `12.0MHz`).
5. Click **Run Schematic Simulation** (Play button bottom-left) to view the live LCD runtime sequence.

### 🏗️ Hardware Compiling & Deployment Flashing
1. Launch **Keil µVision 4** and open the active project workspace configuration file.
2. Click **Build Target (F7)** to run the GCC-ARM cross-compilation pipeline and export the production production binary `.hex` output.
3. Connect your physical LPC2148 development training board via a USB-to-UART converter cable to your workstation.
4. Launch the **Flash Magic** utility tool.
5. Match the configurations: Select Device: `LPC2148`, COM Port: *(Verify Device Manager)*, Baud Rate: `9600`.
6. Enable the `Erase all Flash+Code Rd Prot` tick mark to clear target blocks safely.
7. Locate your compiled `.hex` image file path, then click **Start** to flash the firmware directly to the controller core.

---

## 👥 Developers & Project Lineage

* **Ajay Kumar Pilla** - Firmware Design, Hardware Implementation, and System Integration.
* Developed under professional technical training guidance at **Vector India**.

***
