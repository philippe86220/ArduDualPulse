# ArduDualPulse

Exécuter **deux callbacks périodiques sur le Timer0** tout en conservant l’**overflow système (1024 µs)** utilisé par `millis()`, `micros()` et `delay()`.

- Compare **A** → période fixe **1000 µs**
- Compare **B** → période **N µs** configurable (`20..1000`, pas **4 µs**)
- Utilise une **planification relative** (`OCR0x += delta`) pour éviter toute dérive dans le temps
- **Ne modifie pas** le prescaler ni le mode Timer0 configurés par le noyau Arduino

> **Attribution :** Technique et code originaux conçus par **ChatGPT (GPT-5), OpenAI**.  
> Le présent dépôt se contente de packager, documenter et démontrer la technique.

---

## Contexte

Sur Arduino, **Timer0** est utilisé par le cœur système :

- L’**overflow** toutes les **1024 µs** alimente `millis()`, `micros()` et `delay()`.
- Modifier Timer0 casserait donc tout le fonctionnement temporel de base.

**ArduDualPulse** ajoute **deux évènements périodiques** supplémentaires sur les unités **Compare A** et **Compare B**, **sans toucher à l’overflow** ni à la configuration du Timer0.

---

## Fonctionnalités

- `millis()` / `micros()` / `delay()` continuent de fonctionner normalement
- Appel périodique chaque **1 ms** (Compare A)
- Appel périodique chaque **N µs** (Compare B), avec :
  - `N ∈ [20..1000]`
  - Résolution **4 µs** (liée au tick du Timer0)
- Conçu pour AVR @ **16 MHz** (Uno, Nano, Pro Mini)

---

## Installation

1. Créez un dépôt GitHub nommé **ArduDualPulse**.
2. Ajoutez les fichiers du projet via **Add file → Upload files**.
3. Pour l’IDE Arduino :
   - Utilisez `Sketch → Include Library → Add .ZIP Library...`
   - ou placez le dossier dans `Documents/Arduino/libraries/`.

---

## API

```c
// Démarre les deux cadences : 1 ms (A) et N µs (B).
void timer0_dual_start(uint16_t N_us);

// Arrête les deux comparaisons (l’overflow système est conservé).
void timer0_dual_stop(void);

// Fixe N (20..1000), arrondi au multiple de 4 µs. Retourne N réellement appliqué.
uint16_t timer0_set_N_us(uint16_t N);
```
**Callbacks utilisateur (faibles)**

```c
void on_timer0_1ms(void);  // appelé toutes les 1000 µs
void on_timer0_Nus(void);  // appelé toutes les N µs
```

Pour les utiliser : définissez-les simplement dans votre sketch.

---

**Exemple**

Voir examples/DualPulse/DualPulse.ino.

```c++
volatile bool flag_1ms = false;
volatile bool flag_N   = false;

void on_timer0_1ms(void) { flag_1ms = true; }
void on_timer0_Nus(void) { flag_N   = true; }

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  timer0_dual_start(200); // N = 200 µs (arrondi 4 µs)
}

void loop() {
  if (flag_1ms) {
    flag_1ms = false;
    // tâche périodique 1 ms
  }

  if (flag_N) {
    flag_N = false;
    // tâche périodique N µs
  }

  // Votre code normal ici
  // millis(), micros() et delay() fonctionnent toujours
}
```

**Notes de conception**

- Timer0 (noyau Arduino) → __Fast PWM, prescaler = 64__  
→ 1 tick = **4 µs**
- Donc :
 - 1000 µs → ```250``` ticks
 - N µs → ```N/4``` ticks ```(5..250)```
- La planification se fait en **relatif** :

```c
OCR0x = OCR0x + delta;
```

Ce choix :
- maintient la **phase constante**
- élimine la **dérive cumulative**
- gère automatiquement l’**overflow 8 bits**

 --- 
Résolution & Gigue
```
+-----------------------+------------------------------------+ 
| Facteur               | Valeur                             |
+ --------------------- +------------------------------------+
| Résolution temporelle | **4 µs** (limitée par Timer0)      |
| Source de gigue       | Latence des interruptions          |
| Recommandation        | **ISRs courtes et non bloquantes** |
+ ----------------------+------------------------------------+
```
Éviter absolument :
- ```delay()``` dans une ISR
- ```Serial.print()``` dans une ISR
- Appels bloquants dans une ISR
- 
→ Utiliser le ***schéma drapeau → traitement dans** ```loop()``` (comme dans l’exemple).

---
- Licence & Attribution
- Licence : **MIT** (**voir LICENSE**)
- Code et stratégie de synchronisation : **ChatGPT (GPT-5), OpenAI**
- Le dépôt ne fait que l’**emballer** et le **documenter** pour usage communautaire.
