#include <ArduDualPulse.h>

// Pins (adapter si besoin)
const uint8_t PIN_STEP = 8;   // D8 (PORTB0)
const uint8_t PIN_DIR  = 9;   // D9 (PORTB1)
const uint8_t PIN_ENA  = 10;  // option (LOW = enable)

// Cadence Compare B (IT rapide)
const uint16_t N_us = 100;    // 100 us -> 10 k ISR/s

// Pulse STEP
const uint16_t pulse_width_us = 4; // largeur HIGH minimale

// Vitesse et rampe (pas/s)
volatile int32_t step_hz = 0;       // vitesse courante
volatile int32_t target_hz = 800;   // consigne
const int32_t accel_hz_per_ms = 10; // rampe (delta par ms)

// Mouvement (pas restants; -1 = infini)
volatile int32_t steps_remaining = -1;

// Internes ISR rapide
volatile uint16_t ticks_per_step = 0xFFFF; // 0xFFFF = stop
volatile uint16_t tick_cnt = 0;
volatile uint8_t  pulse_ticks = 0;
volatile uint8_t  dir_state = 1; // 1 = HIGH, 0 = LOW

// Helpers
static inline uint32_t isrB_hz(uint16_t N) { return 1000000UL / N; }

static inline uint16_t compute_ticks_per_step(uint32_t hz, uint16_t N) {
  if (hz == 0) return 0xFFFF;
  uint32_t tps = isrB_hz(N) / hz;
  if (tps == 0) tps = 1;
  if (tps > 0xFFFF) tps = 0xFFFF;
  return (uint16_t)tps;
}

static inline uint8_t compute_pulse_ticks(uint16_t width_us, uint16_t N) {
  uint32_t t = (width_us + N - 1) / N; // ceil
  if (t == 0) t = 1;
  if (t > 255) t = 255;
  return (uint8_t)t;
}

// ----- ISR lente 1 ms (Compare A) : rampe + direction -----
void on_timer0_1ms(void) {
  int32_t s = step_hz;
  if (s < target_hz) { s += accel_hz_per_ms; if (s > target_hz) s = target_hz; }
  else if (s > target_hz) { s -= accel_hz_per_ms; if (s < target_hz) s = target_hz; }
  step_hz = s;

  // Direction selon le signe de la consigne
  if (target_hz < 0 && dir_state != 0) { dir_state = 0; PORTB &= ~_BV(PB1); } // DIR LOW
  else if (target_hz >= 0 && dir_state != 1) { dir_state = 1; PORTB |= _BV(PB1); } // DIR HIGH

  uint32_t abs_hz = (s >= 0) ? (uint32_t)s : (uint32_t)(-s);
  ticks_per_step = compute_ticks_per_step(abs_hz, N_us);
}

// ----- ISR rapide N us (Compare B) : STEP pulses + decompte -----
void on_timer0_Nus(void) {
  uint16_t tps = ticks_per_step;
  if (tps == 0xFFFF) {
    if (pulse_ticks) { if (--pulse_ticks == 0) PORTB &= ~_BV(PB0); } // STEP LOW
    return;
  }

  if (pulse_ticks) { if (--pulse_ticks == 0) PORTB &= ~_BV(PB0); } // terminer pulse en cours

  if (++tick_cnt >= tps) {
    tick_cnt = 0;

    if (steps_remaining == 0) { ticks_per_step = 0xFFFF; return; }
    if (steps_remaining > 0) steps_remaining--;

    PORTB |= _BV(PB0); // STEP HIGH (front montant)
    pulse_ticks = compute_pulse_ticks(pulse_width_us, N_us);
  }
}

void setup() {
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR,  OUTPUT);
  pinMode(PIN_ENA,  OUTPUT);
  digitalWrite(PIN_ENA, LOW);   // enable
  digitalWrite(PIN_DIR, HIGH);  // sens defaut
  dir_state = 1;

  timer0_dual_start(N_us);      // Compare A = 1 ms, Compare B = N_us

  noInterrupts();
  step_hz = 0;
  target_hz = 800;
  ticks_per_step = compute_ticks_per_step(0, N_us); // stop au depart
  steps_remaining = -1; // mouvement continu par defaut
  interrupts();

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Exemple: code bloquant autorise; le moteur tourne et la rampe evolue quand meme
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(3000);

  // Exemple de commande haute niveau (une seule fois)
  static bool started = false;
  if (!started) {
    noInterrupts();
    target_hz = 1200;       // nouvelle consigne
    steps_remaining = 2000; // deplacement de 2000 pas
    interrupts();
    started = true;
  }

  // Ici pourraient vivre: protocole serie, UI, etc.
}
