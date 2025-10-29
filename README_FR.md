<p align="center">
  <img src="https://dummyimage.com/1000x200/000/fff&text=ArduDualPulse" alt="ArduDualPulse banner" />
</p>


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
## Limitations

- **Résolution Timer0 :** 1 tick = **4 µs** (16 MHz, prescaler=64). Les périodes sont donc quantifiées au multiple de 4 µs.
- **Gigue :** Principalement due à la **latence d’interruption** (autres ISRs, sections critiques, accès Flash). Garder les ISRs **très courtes**.
- **Pas de blocage en ISR :** Éviter `delay()`, `Serial.print()` bloquant, transactions I²C/SPI qui attendent, etc. En ISR, le drapeau global d’interruption est à 0 (I=0) → l’horloge système peut se figer.
- **Débit :** Avec **N = 20 µs**, cela génère **50 kHz** d’interruptions en compare B → charge CPU très élevée. N’utiliser que des handlers ultra-courts (poser un drapeau).
- **Compatibilité cœur Arduino :** La lib suppose la **configuration Timer0 du core** (Fast PWM, prescaler=64, overflow 1024 µs). Si une autre lib reconfigure Timer0, la synchronisation sera affectée.

## Dépannage

- **`delay()` se fige / dérive temporelle**  
  Cause : `delay()` (et l’horloge du core) nécessite l’overflow Timer0, suspendu en ISR.  
  Solution : **Ne jamais appeler `delay()` en ISR**. Utiliser le schéma **drapeau + `loop()`** (voir l’exemple).

- **Gigue visible sur 1 ms / N µs**  
  Cause : latence ISR (autres interruptions, handlers trop longs).  
  Solution : Raccourcir **au maximum** les ISRs, déplacer le travail dans `loop()`, éviter `Serial.print()` en ISR, diminuer la fréquence de N si besoin.

- **`Serial.print()` instable dans les callbacks**  
  Cause : impression potentiellement bloquante et dépendante des interruptions.  
  Solution : Ne pas imprimer en ISR. Accumuler et imprimer plus tard dans `loop()`.

- **Une autre bibliothèque casse le timing**  
  Cause : reconfiguration de Timer0 (mode/prescaler).  
  Solution : Restaurer la config du core Arduino ou retirer la bibliothèque en conflit. Ce projet **ne modifie pas** le mode/prescaler de Timer0.

- **Besoin de stabilité sub-microseconde**  
  Remarque : Timer0 ne peut pas faire mieux que **4 µs** de résolution.  
  Solution : Utiliser **Timer1** en mode CTC prescaler=1 (hors périmètre de cette lib et possible conflit avec d’autres codes).

---
- Licence & Attribution
- Licence : **MIT** (**voir LICENSE**)
- Code et stratégie de synchronisation : **ChatGPT (GPT-5), OpenAI**
- Le dépôt ne fait que l’**emballer** et le **documenter** pour usage communautaire.
