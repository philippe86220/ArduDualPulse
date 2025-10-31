# Exemple : Stepper avec Timer0 uniquement (ArduDualPulse)

Cet exemple montre comment :
- générer les impulsions STEP dans l’ISR rapide (Compare B, N µs)
- gérer la rampe de vitesse dans l’ISR lente (Compare A, 1 ms)
- conserver `millis()`, `micros()` et `delay()` fonctionnels
- permettre à `loop()` d’être bloquante sans arrêter le moteur

## Matériel
- Arduino Uno / Nano / Pro Mini (ATmega328P)
- Driver Step/Dir (A4988, DRV8825, TMC2208, etc.)
- Moteur pas-à-pas (NEMA 17, etc.)

## Branchement
- STEP → D8
- DIR  → D9
- ENA  → D10 (optionnel)
- GND commun entre Arduino + driver

## Fonctionnement
- La vitesse augmente progressivement jusqu’à la consigne (rampe)
- Les impulsions STEP restent régulières même pendant `delay()`
- Le moteur ne “rafale” pas : aucune compensation impossible

## À retenir
- ISR rapide = temps réel strict → toujours **très courte**
- ISR 1 ms = logique moteur (rampe) → courte également
- loop() = commande haut niveau → peut être lente ou bloquante

