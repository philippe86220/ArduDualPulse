/*
  ArduDualPulse — Exemple démonstratif de la *dualité* des cadences Timer0
  -----------------------------------------------------------------------

  Ce programme illustre visuellement les DEUX rythmes produits simultanément :

  ┌───────────────┬───────────┬───────────────────────────────┬────────────┬─────────────────────────┐
  │   Cadence     │  Source   │       Action observable       │ Fréquence  │          Rôle           │
  ├───────────────┼───────────┼───────────────────────────────┼────────────┼─────────────────────────┤
  │ 1 ms          │ Compare A │ LED clignote lentement (0.5 Hz)│ 1000 Hz → divisé dans loop() │ Rythme "humain"      │
  │ N = 200 µs    │ Compare B │ D8 génère un signal carré rapide│ 5000 Hz    │ Rythme électronique rapide │
  └───────────────┴───────────┴───────────────────────────────┴────────────┴─────────────────────────┘

  🎯 Ce qu’on démontre :
     - Les deux cadences tournent **en même temps**
     - Timer0 conserve son overflow système → `millis()`, `micros()` et `delay()` restent fonctionnels
     - Compare A → rythme lent → LED
     - Compare B → rythme rapide → signal sur la broche D8

  👀 Comment observer la dualité :
     - La LED (D13) clignote calmement (500 ms ON / 500 ms OFF)
     - La broche D8 oscille à haute fréquence :
         * visible à l’oscilloscope
         * ou analyser logique
         * ou même audible → brancher un petit piezo entre D8 et GND

  🧠 Conclusion :
     Compare A cadencé à **1 ms** → rythme “visuel / humain”
     Compare B cadencé à **N µs** → rythme “rapide / électronique”
     → Les deux coexistent **sans casser** le timing Arduino.
*/

#include <ArduDualPulse.h>

// Flags (schéma recommandé : ISR = pose drapeau → loop() = exécute)
volatile bool flag_1ms = false;
volatile bool flag_N   = false;

// Appelé toutes les 1 ms (Compare A)
void on_timer0_1ms(void) { flag_1ms = true; }

// Appelé toutes les N µs (Compare B)
void on_timer0_Nus(void) { flag_N = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(8, OUTPUT); // D8 → oscillation rapide

  // Démonstration : N = 200 µs → Compare B ≈ 5000 Hz
  timer0_dual_start(200);
}

void loop() {
  // ----- Voie "lente" (1 ms) -----
  if (flag_1ms) {
    flag_1ms = false;

    static uint16_t counter = 0;
    if (++counter >= 500) {  // 500 × 1ms = 500 ms
      counter = 0;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }

  // ----- Voie "rapide" (N µs) -----
  if (flag_N) {
    flag_N = false;

    // Toggle D8 en *1 cycle CPU* → fréquence très propre
    PINB = _BV(PB0);   // PB0 = D8 sur Arduino Uno/Nano
  }
}
