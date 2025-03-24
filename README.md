# RTOS-Implementation-TI-Tiva-C-Series-LaunchPad

# ⚙️ RTOS Lab 5: TI Tiva C Launchpad + uC/OS-II

This project demonstrates the use of a Real-Time Operating System (RTOS) on the **TI Tiva C Series LaunchPad (EK-TM4C123GXL)** using **Micrium uC/OS-II**. Through multiple concurrent tasks, semaphores, and event flags, the system simulates real-time control and communication across tasks triggered by button presses.

> 🎓 Course: ECE 473 — Microcontrollers  
> 👨‍💻 Student: Devin Pen    
> 🗓️ Date: March 2024

---

## 📌 Summary

The system implements and runs four concurrent tasks:
- **ButtonMonitor:** Detects SW1 and SW2 button presses and sets event flags.
- **Blinky:** Toggles an LED at a randomly generated delay rate, and adjusts rate on SW1 press using a non-blocking event flag (`OSFlagAccept()`).
- **ButtonAlert:** Waits for SW2 press using a blocking event flag (`OSFlagPend()`), then prints a message to UART.
- **DebuggingVars:** Continuously prints system stats to the UART, including CPU usage, idle counter, and context switch count.

---

## 🧠 Key RTOS Concepts Used

### 🔹 Event Flags
- `OSFlagPost()` used to set flags when buttons are pressed.
- `OSFlagAccept()` (non-blocking) used in `Blinky` to keep blinking LED without pausing.
- `OSFlagPend()` (blocking) used in `ButtonAlert` to wait until SW2 is pressed.

### 🔹 Semaphores
- `UARTSemaphore` ensures safe UART access so only one task can print at a time.
- Prevents data collision between `ButtonAlert` and `DebuggingVars`.

### 🔹 Debug Output (via PuTTY)
- `OSCPUUsage`: Percentage of CPU load
- `OSIdleCtr`: Number of idle cycles (drops with more button interaction)
- `OSCtxSwCtr`: System-wide context switch count
- Task-specific context switch counters (e.g., `Blinky`)

---

## 🔧 How It Works

### ➕ On Startup:
- All four tasks are created with different priorities.
- Tasks begin running independently and communicating via flags and semaphores.

### 🔘 When Button 1 (SW1) is pressed:
- `ButtonMonitor` posts `sw1Flag`
- `Blinky` detects it and updates the LED blink rate randomly

### 🔘 When Button 2 (SW2) is pressed:
- `ButtonMonitor` posts `sw2Flag`
- `ButtonAlert` unblocks and sends a message over UART

### 💻 UART Output (PuTTY)
- Connect via UART at **115200 baud**
- Displays real-time debug info:
