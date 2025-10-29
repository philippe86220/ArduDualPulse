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

See examples/DualPulse/DualPulse.ino.

```c
volatile bool flag_1ms = false, flag_N = false;

void on_timer0_1ms(void) { flag_1ms = true; }
void on_timer0_Nus(void) { flag_N = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  timer0_dual_start(200); // N = 200 us (rounded to 4 us step)
}

void loop() {
  if (flag_1ms) { flag_1ms = false; /* 1ms task */ }
  if (flag_N)   { flag_N   = false; /* N us task */ }
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

## License & Attribution
- License: MIT (see LICENSE)
- Original code and timing strategy designed by ChatGPT (GPT-5), OpenAI
- This repository packages, documents, and demonstrates the technique.
