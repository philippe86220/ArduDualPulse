#include <ArduDualPulse.h>

// Keep ISRs short: set flags and handle work in loop
volatile bool flag_1ms = false;
volatile bool flag_N   = false;

void on_timer0_1ms(void) { flag_1ms = true; }
void on_timer0_Nus(void) { flag_N   = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // Start with N = 200 us (will be rounded to nearest multiple of 4)
  timer0_dual_start(200);
}

void loop() {
  if (flag_1ms) {
    flag_1ms = false;
    // Example: blink every 500 ms
    static uint16_t cnt = 0;
    if (++cnt >= 500) {
      cnt = 0;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }

  if (flag_N) {
    flag_N = false;
    // Do your N us task here (non-blocking if possible)
  }

  // Your normal code; millis()/micros()/delay() keep working
  // delay(1); // ok here (not in ISR)
}

