<p align="center">
  <img src="https://dummyimage.com/1000x200/000/fff&text=ArduDualPulse" alt="ArduDualPulse banner" />
</p>

# ArduDualPulse

Run **two periodic callbacks on Timer0** while keeping the **Arduino core overflow (1024 µs)** intact for `millis()`, `micros()`, and `delay()`.

- Compare **A**: fixed **1000 µs** period
- Compare **B**: configurable **N µs** period (**20..1000**, step **4 µs**)
- Uses **relative scheduling** (`OCR0x += delta`) to avoid drift and handle modulo-256 wrap
- **Does not** modify Timer0 prescaler or mode set by the Arduino core

> **Attribution:** Original technique and code designed by **ChatGPT (GPT-5), OpenAI**.  
> This repository only packages, documents, and demonstrates the technique.

---

## Why

Timer0 is used by the Arduino core (overflow at **1024 µs**) to power `millis()`, `micros()` and `delay()`.  
ArduDualPulse adds **two additional periodic schedules** on the **compare units A and B** without breaking the overflow.

---

## Features

- Keeps `millis() / micros() / delay()` working normally
- 1 ms periodic callback (Compare A)
- N µs periodic callback (Compare B), `N in [20..1000]` with `4 µs` resolution
- Minimal, dependency-free, AVR @ 16 MHz (ATmega328P Uno/Nano)

---

## Installation

1. Create a new repo named **ArduDualPulse** on GitHub.
2. Click **Add file → Upload files**, and drop the files of this project (see tree above).
3. In Arduino IDE, use **Sketch → Include Library → Add .ZIP Library...** if you prefer a ZIP, or place this folder into your `Documents/Arduino/libraries/`.

---

## API

```c
// Start both schedules: 1ms (A) and N microseconds (B).
void timer0_dual_start(uint16_t N_us);

// Stop both schedules (overflow kept by the core).
void timer0_dual_stop(void);

// Set N period (20..1000), rounded to multiple of 4. Returns the actual N.
uint16_t timer0_set_N_us(uint16_t N);
```
## User callbacks (weak, override them in your sketch)
```c
void on_timer0_1ms(void);  // called every 1000 us (Compare A)
void on_timer0_Nus(void);  // called every N us    (Compare B)
```

## Example

## Example

The example `examples/DualPulse/DualPulse.ino` visually demonstrates the *dual scheduler* behavior:

| Cadence        | Source    | Visible Effect                         | Frequency                         | Role                     |
|----------------|-----------|----------------------------------------|-----------------------------------|--------------------------|
| **1 ms**       | Compare A | LED blinks slowly (approx. 0.5 Hz)     | 1000 Hz → divided in `loop()`     | Human-scale timing       |
| **N = 200 µs** | Compare B | Fast square wave on D8                 | ~**2500 Hz** *(f = 1 / (2·N))*    | High-speed periodic event |


You can verify this with:

- a **LED** on pin `LED_BUILTIN` (blinks every ~500ms)
- a **scope / logic analyzer** on pin **D8**
- or even a **piezo** on D8 → audible tone 🎧

**This is the core purpose of ArduDualPulse:**  
two periodic tasks, one slow and one fast, **running simultaneously**, without breaking `millis()`, `micros()`, or `delay()`.

```c++
volatile bool flag_1ms = false;
volatile bool flag_N   = false;

void on_timer0_1ms(void) { flag_1ms = true; }
void on_timer0_Nus(void) { flag_N   = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(8, OUTPUT);  // D8 will output the high-speed square wave

  // Demonstration: N = 200 µs → ~2500 Hz on D8
// (toggle at each Compare B → full period = 2 × N)

  timer0_dual_start(200);
}

void loop() {
  // ----- 1 ms event (Compare A) -----
  if (flag_1ms) {
    flag_1ms = false;

    static uint16_t counter = 0;
    if (++counter >= 500) {     // 500 × 1 ms = 500 ms
      counter = 0;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }

  // ----- N µs event (Compare B) -----
  if (flag_N) {
    flag_N = false;

    // Toggle D8 in a single CPU cycle → clean square wave
    PINB = _BV(PB0); // PB0 = D8 on Arduino Uno
  }
}

```

**Design Notes**
- Arduino core config → Timer0 = Fast PWM, __prescaler = 64__  
→ Timer tick = **4 µs.**
- Therefore:
  - ```1000 µs``` → ```250 ticks```
  - ```N µs``` → ```N / 4``` ticks ```(5..250)```
- Next events are scheduled **relatively**:

```c
OCR0x = OCR0x + delta;
```
This:
- preserves timing phase
- prevents cumulative drift
- automatically handles 8-bit wrap-around (mod 256)
  
**Resolution & Jitter**
- Minimum timing resolution: **4 µs**
- Small jitter may occur due to **interrupt latency**
- **Keep ISRs short** (or use flag + loop technique)

---
## Limitations

- **Timer0 resolution:** 1 tick = **4 µs** (16 MHz, prescaler=64). Periods are therefore quantized to multiples of 4 µs.
- **Jitter:** Mainly due to **interrupt latency** (nested/disabled interrupts, other ISRs, Flash wait states). Keep ISRs **very short**.
- **Do not block in ISRs:** Avoid `delay()`, blocking `Serial.print()`, I²C/SPI transactions that wait, etc. In an ISR the global interrupt flag is cleared (I=0), so core timekeeping may stall.
- **Throughput:** With **N = 20 µs** you get **50 kHz** compare-B interrupts → extremely high CPU load. Use only ultra-short handlers (set a flag).
- **Core compatibility:** This library assumes the **Arduino core’s Timer0 configuration** (Fast PWM, prescaler=64, 1024 µs overflow). If another library reconfigures Timer0, timing will be affected.

## Troubleshooting

- **`delay()` freezes / time drifts**  
  Cause: `delay()` (and core timekeeping) needs Timer0 overflow interrupts, which are suspended while inside ISRs.  
  Fix: **Never call `delay()` inside ISRs**. Use the **flag + `loop()`** pattern (see example).

- **Visible jitter on 1 ms / N µs tasks**  
  Cause: ISR latency (other interrupts, long handlers).  
  Fix: Keep ISRs **as short as possible**, move work to `loop()`, avoid `Serial.print()` in ISRs, reduce N’s frequency if needed.

- **`Serial.print()` behaves oddly inside callbacks**  
  Cause: Printing can block and depends on interrupts.  
  Fix: Do not print in ISRs. Accumulate data and print later in `loop()`.

- **Another library breaks the timing**  
  Cause: It reconfigures Timer0 (mode/prescaler).  
  Fix: Restore Arduino core Timer0 settings or remove the conflicting library. This project **does not** modify Timer0 mode/prescaler by design.

- **Need sub-microsecond stability**  
  Note: Timer0 cannot deliver <4 µs resolution.  
  Fix: Use **Timer1** with prescaler=1 in CTC mode (but that’s outside this library and may conflict with other code).

---

## License & Attribution
- License: MIT (see LICENSE)
- Original code and timing strategy designed by ChatGPT (GPT-5), OpenAI
- This repository packages, documents, and demonstrates the technique.
