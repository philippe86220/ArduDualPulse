# Blink_DualTimer (Compare A + Compare B using Timer0 only)

## English

This example demonstrates how ArduDualPulse provides:
- A **1 ms periodic event** using Timer0 Compare-A (suitable for human-scale timing)
- A **high-speed periodic event** using Timer0 Compare-B (for precise, sub-millisecond timing)
- While keeping Arduino time functions working normally (`millis()`, `micros()`, `delay()`)

### What happens in this example
- The LED toggles at a **slow human-visible rhythm** (500 ms ON / 500 ms OFF) based on the **1 ms tick**.
- Meanwhile, a second pin toggles at a **much higher frequency** (determined by `N_us`).
- Both events are driven from **Timer0**, without affecting Arduino timing.

### Key points
- Compare-A (1 ms interrupt) is good for user-level tasks, periodic checks, state machines.
- Compare-B (N µs fast interrupt) is used for precise timing (e.g. stepper pulses, servo timing).
- The main `loop()` does not need to handle time — CPU remains free.

---

## Français

Cet exemple montre comment ArduDualPulse fournit :
- Un **événement périodique toutes les 1 ms** via Compare-A du Timer0 (pour les actions « humaines »)
- Un **événement très rapide** via Compare-B du Timer0 (pour les timings en dessous de la milliseconde)
- Tout en conservant le fonctionnement normal de `millis()`, `micros()` et `delay()`

### Ce que fait concrètement cet exemple
- La LED clignote **lentement** (500 ms ON / 500 ms OFF) grâce à l’**accumulateur basé sur 1 ms**.
- En parallèle, une autre broche commute à **haute fréquence** (définie par `N_us`).
- Les deux rythmes proviennent **du même Timer0**, sans casser l’horloge Arduino.

### Points importants
- Compare-A (1 ms) sert aux tâches « logiques » : états, compteurs, événements lents.
- Compare-B (N µs) sert aux actions **précises** : impulsions moteur, signaux réguliers, etc.
- `loop()` reste disponible pour la logique haut niveau ou peut même être bloquante si besoin.

---

**This example helps illustrate the fundamental purpose of ArduDualPulse:**  
→ *One timer, two time scales, no loss of Arduino timing.*  

