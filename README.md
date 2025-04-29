# Embedded OS Application — Bare-metal and RTOS Implementation

## Project Overview

The aim was to design and implement two embedded applications on the ESP32 platform:
- A **bare-metal** application (without an operating system).
- A corresponding **FreeRTOS** application.

Both implementations demonstrate basic event detection, input/output device control, interrupt handling, and task-based concurrency.

---

## Hardware and Tools

- **Platform**: ESP32-WROOM-32
- **Programming Language**: C
- **Operating System**: FreeRTOS (for RTOS version)
- **Development Tools**: ESP-IDF, CMake, Visual Studio Code

**Peripherals Used**:
- **Input**: Push Button (GPIO25)
- **Output**: Two LEDs (GPIO2 - Green LED, GPIO4 - Blue LED)

---

## Project Requirements

- Bare-metal application with:
  - Event detection via **polling** and **interrupts**.
  - Control of input (button) and output (LEDs) peripherals.
- RTOS application with:
  - Separation of activities into **FreeRTOS tasks**.
  - Use of **semaphores** for synchronization.
  - Use of **time management** functions (e.g., `vTaskDelay()`).

---

## Bare-metal Implementation

The bare-metal application continuously polls the button state and updates the status of a Green LED accordingly:
- **Polling** is used to detect button presses.
- **Debouncing** is performed via short delays (`vTaskDelay` with 10 ms).
- **Simple GPIO control** is used for reading button input and setting LED output.

> This version operates directly over hardware without an operating system, providing low-level control but requiring manual management of all concurrency and timing issues.

---

## RTOS Implementation (FreeRTOS)

The FreeRTOS-based application splits functionality into separate tasks and uses semaphores to manage events:
- **Button interrupt** triggers a semaphore release.
- **Task 1** (`led1_task`) waits for the semaphore and toggles the Green LED when the button is pressed.
- **Task 2** (`led2_task`) continuously blinks the Blue LED if no higher-priority events (button press) occur.
- **Debounce logic** is implemented within the interrupt handler to avoid noise.

> This version leverages FreeRTOS multitasking, making the design cleaner, more modular, and scalable.

---

## Key Concepts and Features

- Polling vs Interrupt-driven event handling.
- Task creation and management using FreeRTOS (`xTaskCreate`).
- Semaphore synchronization between ISR and tasks (`xSemaphoreGiveFromISR` / `xSemaphoreTake`).
- Time-based task scheduling (`vTaskDelay`).
- GPIO input/output configuration and usage.
- Handling debouncing logic in a real-time environment.

---

## Lessons Learned

- **Bare-metal programming** requires careful timing and manual concurrency handling, which can become complex for larger systems.
- **RTOS-based design** simplifies concurrency by abstracting low-level details, allowing for better scalability and maintainability.
- Understanding **interrupt management** and **synchronization primitives** (like semaphores) is crucial for reliable embedded system design.
- Proper debouncing is essential to ensure correct input detection, especially in hardware-based systems.
