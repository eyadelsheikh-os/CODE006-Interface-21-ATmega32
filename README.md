# ATmega32 Embedded Drivers

A layered driver library for the **ATmega32** (8 MHz), written from scratch in C. The repo contains six progressively more complex test applications that exercise every peripheral driver in the stack — from basic GPIO/keypad/LCD up through multi-MCU protocols (USART, SPI, I²C).

---

## Architecture

```
┌──────────────────────────────────────────────────────┐
│                       APP                            │
└───────────────┬──────────────────────┬───────────────┘
                │                      │
        ┌───────▼──────┐        ┌──────▼──────┐
        │    HAL-C     │◄──────►│     HAL     │
        └───────┬──────┘        └──────┬──────┘
                │                      │
        ┌───────▼──────┐        ┌──────▼──────┐        ┌───────────┐
        │   MCAL-C     │        │    MCAL     │───────►│  MCAL-G   │
        │ mcal_registers        │  mcal_dio   │        │mcal_interrupts
        └──────────────┘        └─────────────┘        └───────────┘
                ▲                      ▲                     ▲
                └──────────────────────┴─────────────────────┘
                                       │
                                 ┌─────▼──────┐
                                 │  Common    │
                                 └────────────┘
```

| Layer | Contents |
|-------|----------|
| **Common** | `std_types.h`, `gpio_types.h`, `std_macros.h`, `ud_config.h` |
| **MCAL-C** | `mcal_registers.h` — raw register definitions |
| **MCAL** | DIO, Timers (TC0/TC1/TC2), ADC, USART, SPI, I²C, EEPROM |
| **MCAL-G** | Interrupt control (`cli`/`sei`, INT0/INT1/INT2) |
| **HAL-C** | Common HAL types (LED/BUTTON active type enums) |
| **HAL** | LED, Button, 7-Segment, Keypad, LCD (4-bit & 8-bit) |
| **STR** | Circular queue |

---

## Project 1 — Keypad Test

**Files:** `1. keypad_test.c`

Two 4×4 matrix keypads wired to two 7-segment displays. One keypad is polled on press, the other on release, and each key is decoded to a hex digit on its own display.

**Drivers exercised:** `hal_keypad`, `hal_sevenseg`, `mcal_dio`.

---

## Project 2 — LCD Test

**Files:** `2. lcd_test-1.c` (8-bit), `2. lcd_test-2.c` (4-bit)

Two variants of the same keypad-echo application, one for each LCD driver width. A keypad character is echoed to the LCD with automatic line wrapping (line 1 → line 2 → line 1).

**Drivers exercised:** `hal_lcd8`, `hal_lcd4`, `hal_keypad`, `mcal_dio`.

---

## Project 3 — Multi-MCU Clock

**Files:** `3. clock_test-1.c`, `3. clock_test-2.c`, `3. clock_test-3.c`

A cascaded 8-digit clock built from three ATmega32 MCUs linked over USART. Each board owns a slice of digits and passes a carry request (`REQ_CLKn_ACC_INC`) to the next board when its own digit wraps.

- **Board 1** — 1 digit driven by a keypad, saves state in EEPROM, sends carries to Board 2
- **Board 2** — 3 digits multiplexed on a single 7-segment, cascades to Board 3
- **Board 3** — 4 digits, terminal board (no further cascade)

**Drivers exercised:** `hal_sevenseg`, `hal_keypad`, `hal_led` (for digit selects), `mcal_usart`, `mcal_eeprom`, `mcal_timers`, `mcal_interrupts`.

---

## Project 4 — SPI Multi-Slave Temperature

**Files:** `4. spi_test-1.c` (master), `4. spi_test-2.c` (slave)

A master polls three SPI slaves, each running an ADC channel reading a temperature sensor. The master averages the readings, ranks the slaves (1 = hottest, 0 = outlier), and sends each slave its rank byte. Slaves light an LED corresponding to their rank.

**Drivers exercised:** `mcal_spi`, `mcal_adc`, `hal_led`, `mcal_timers`, `mcal_interrupts`.

---

## Project 5 — I²C Multi-Slave Temperature

**Files:** `5. i2c_test-1.c` (master), `5. i2c_test-2.c` / `-3.c` / `-4.c` (slaves at 0x10 / 0x11 / 0x12)

The I²C equivalent of Project 4. The master (MMC0, address `0x0A`) polls three slaves for temperature, ranks them, and writes each rank back. Each slave drives a bank of LEDs based on its rank and runs an ADC on a temperature sensor.

**Drivers exercised:** `mcal_i2c` (blocking + interrupt-driven), `mcal_adc`, `hal_led`, `mcal_timers`, `mcal_interrupts`.

---

## Project 6 — Ultrasonic Distance + Non-Blocking I²C ⭐

**Files:** `6. ultrasonic-1.c` (MMC0), `6. ultrasonic-2.c` (SMC0), `6. ultrasonic-3.c` (standalone frequency generator)

The capstone project. Combines ultrasonic time-of-flight measurement, temperature compensation of the speed of sound, non-blocking I²C, and a two-way peer-to-peer link between two MCUs.

### 6.1 — `6. ultrasonic-1.c` (MMC0)

The master-side MCU with the following responsibilities:

- Reads ambient temperature through its ADC every second
- Looks up the temperature-compensated speed of sound from a **256-entry LUT** (15 °C – 35 °C)
- Displays distance (received from SMC0) and temperature on a 4-bit LCD
- On a hardware button press (`INT1`), sends the 2-byte `sound_speed` value to SMC0 using **`I2C_SendAsync`**
- Registers slave callbacks (`SlaveRxCallback` / `SlaveStopCallback`) so it can receive distance readings from SMC0 during idle windows

The MCU is simultaneously master (occasionally) and slave (occasionally) on the same bus. The **non-blocking API** is what makes this possible — a blocking send would freeze the slave role for the duration of the transfer.

### 6.2 — `6. ultrasonic-2.c` (SMC0)

The measurement-side MCU:

- Drives an HC-SR04-style ultrasonic sensor using Timer1 in **input capture mode** (ICP1 = PD6)
- On rising edge, records `TCNT1` into `a`; switches to falling-edge mode
- On falling edge, records `b`, computes `distance = SOUND × (b − a) / 2 / (F_CPU/8)`, applies out-of-range filtering (rejects pulses < 50 µs or > 26 ms), and updates a 3-digit multiplexed 7-segment display
- Re-triggers the sensor and re-arms the capture from within the ISR
- On a hardware button press (`INT0`), sets a flag; the main loop performs a **non-blocking I²C send** of the distance to MMC0
- Accepts incoming `sound_speed` writes from MMC0 through slave callbacks

No part of the I²C transaction runs inside an ISR — the button just raises a flag, and the main loop schedules the send only when `I2C_IsBusy()` is false.

### 6.3 — `6. ultrasonic-3.c` (standalone frequency generator)

A single-MCU utility that converts a user-entered distance into a **square wave whose half-period equals the equivalent ultrasonic echo time**. This was used to replace the physical sensor during bench testing — the receiving MCU measures the pulse width and reports the same distance it would from a real echo.

- User selects a distance (2–450 cm) with the keypad, digit by digit
- `UpdateFreq()` computes `f = SOUND / (4 · distance)` and the corresponding `ICR1` value
- Timer1 runs in CTC with ICR1 as TOP, OC1A in **toggle mode**, generating a 50 % duty cycle on PD5
- The distance and generated frequency are shown live on a 4-bit LCD, and the counter state is persisted in EEPROM

### Project 6 wiring

This application is where all the design decisions from the earlier projects converge:

<img width="1103" height="773" alt="Screenshot 2026-10-13 163342" src="https://github.com/user-attachments/assets/895b8760-4990-4a74-9cb8-182d94883d7a" />


- **Two MCUs, one I²C bus, both capable of being master** — the driver's role tracking and arbitration handling are exercised for real
- **Non-blocking transfers** — button-triggered sends don't block the measurement loop, and the measurement loop doesn't block the button
- **Interrupt-driven I/O** — input capture, ADC, Timer0 display multiplex, INT0, and TWI all share the CPU without starvation
- **Temperature compensation** — the speed of sound matters at the centimeter level; a 10 °C error shifts the reading by ~1.8 %
- **Cross-driver integration** — 7 drivers (DIO, Timers, ADC, USART-adjacent, EEPROM, INT, I²C) working together without conflicts

---

## Building

All projects target **ATmega32 @ 8 MHz** and are tested with:

- **AVR-GCC** (Atmel Studio 7 / Microchip Studio)
- **Proteus** for simulation.

Configure `F_CPU` in `common/ud_config.h`. Each `.c` file has its own `main()` — build one project at a time.

