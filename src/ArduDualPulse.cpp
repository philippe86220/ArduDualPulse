#include "ArduDualPulse.h"
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Timer0 @ 16MHz, prescaler=64 -> 1 tick = 4us
static volatile uint8_t t0_delta_A = 250; // 1000 us -> 250 ticks
static volatile uint8_t t0_delta_B = 50;  // N us -> N/4 ticks (set at runtime)

// Default weak callbacks
void on_timer0_1ms(void) {}
void on_timer0_Nus(void) {}

// Compare A ISR: 1000 us
ISR(TIMER0_COMPA_vect) {
  OCR0A = (uint8_t)(OCR0A + t0_delta_A);
  on_timer0_1ms();
}

// Compare B ISR: N us
ISR(TIMER0_COMPB_vect) {
  OCR0B = (uint8_t)(OCR0B + t0_delta_B);
  on_timer0_Nus();
}

// Configure N (20..1000). Rounded to multiple of 4. Returns actual N.
uint16_t timer0_set_N_us(uint16_t N) {
  if (N < 20)   N = 20;
  if (N > 1000) N = 1000;
  N = (uint16_t)((N / 4) * 4);

  uint8_t ticks = (uint8_t)(N / 4);
  if (ticks < 5)   ticks = 5;     // 20 us
  if (ticks > 250) ticks = 250;   // 1000 us

  t0_delta_B = ticks;

  uint8_t sreg = SREG;
  cli();
  OCR0B = (uint8_t)(TCNT0 + t0_delta_B); // prime next match from current position
  SREG = sreg;

  return (uint16_t)ticks * 4;
}

// Start both compares while keeping core overflow untouched
void timer0_dual_start(uint16_t N_us) {
  // Do not touch TCCR0A/TCCR0B or prescaler: keep Arduino core config
  TIFR0 = _BV(OCF0A) | _BV(OCF0B); // clear pending compare flags

  uint8_t sreg = SREG;
  cli();

  t0_delta_A = 250;               // 1000 us
  (void)timer0_set_N_us(N_us);    // sets t0_delta_B and primes OCR0B

  OCR0A = (uint8_t)(TCNT0 + t0_delta_A); // prime A

  TIMSK0 |= _BV(OCIE0A) | _BV(OCIE0B);   // enable compare A and B

  SREG = sreg;
}

void timer0_dual_stop(void) {
  TIMSK0 &= (uint8_t)~(_BV(OCIE0A) | _BV(OCIE0B));
}
