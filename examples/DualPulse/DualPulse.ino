/*
  ArduDualPulse — Exemple démonstratif de la *dualité* des cadences Timer0
  ------------------------------------------------------------------------

  Ce programme illustre visuellement les DEUX rythmes produits simultanément :

    Cadence        | Source     | Effet observable                          | Fréquence approx. | Rôle
    ---------------|------------|-------------------------------------------|-------------------|-------------------------
    1 ms           | Compare A  | La LED clignote lentement (~0,5 Hz)       | 1000 Hz → divisé  | Rythme « humain »
    N = 200 µs     | Compare B  | La broche D8 génère un signal carré       | ~2500 Hz          | Évènement rapide

  Ce que ce programme démontre :
    - Les deux cadences fonctionnent en parallèle
    - Le Timer0 conserve son overflow système → `millis()`, `micros()` et `delay()` restent opérationnels
    - Compare A → événement lent (accumulation 1 ms) → blink LED
    - Compare B → événement rapide (200 µs) → oscillation de D8

  Observation :
    - La LED (D13) clignote à l'œil nu.
    - La broche D8 produit un signal carré ≈ 2,5 kHz :
        • visible à l’oscilloscope
        • ou détectable avec un analyseur logique
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
}
