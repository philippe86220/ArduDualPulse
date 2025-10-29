#pragma once
#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Attribution:
// Original technique and code designed by ChatGPT (GPT-5), OpenAI.
// This library packages, documents and demonstrates the solution.

// Start both schedules: 1ms (A) and N microseconds (B).
void timer0_dual_start(uint16_t N_us);

// Stop both compare schedules (keeps Arduino core overflow intact).
void timer0_dual_stop(void);

// Set N period (20..1000), rounded to multiple of 4. Returns the actual N applied.
uint16_t timer0_set_N_us(uint16_t N);

// User callbacks (weak). Override them in your sketch if needed.
void on_timer0_1ms(void) __attribute__((weak));
void on_timer0_Nus(void) __attribute__((weak));

#ifdef __cplusplus
}
#endif

