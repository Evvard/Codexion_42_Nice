# Analyse threads — Codexion

Document d'analyse de la concurrence (séparé du `README.md`). Il explique le
modèle de synchronisation, les corrections apportées suite à la peer-review, et
les résultats de tests (`helgrind`, `memcheck`, exclusion mutuelle, burnout).

---

## 1. Modèle de synchronisation

Toute la **logique** du simulateur est protégée par un seul mutex d'état :

| Primitive | Rôle |
|-----------|------|
| `sched_lock` (mutex) | Section critique unique : protège `simulation_end`, `dongle_held[]`, `dongle_cooldown_ends[]`, le tas (heap), `seq_counter`, `waiters`, et tous les champs `coder[].*` (`waiting`, `req`, `compile_count`, `last_compile_start`). |
| `queue_cond` (cond var) | Réveille les codeurs en attente quand une clé est libérée, quand un cooldown expire ou quand la simulation se termine. Toujours associée à `sched_lock`. |
| `log_lock` (mutex) | Sérialise les écritures sur la sortie standard (`print_status`, log de burnout) pour qu'aucune ligne ne s'entremêle. |
| `dongle_mutex[]` (mutex) | Verrou *physique* par clé USB. Sert uniquement à matérialiser « has taken a dongle » avec blocage réel ; l'exclusion logique est déjà garantie par `dongle_held[]`. |

**Ordre de verrouillage fixe** : `dongle_mutex[]` → `sched_lock` → `log_lock`.
Aucune inversion possible → pas de deadlock entre mutex.

### Pourquoi il n'y a pas de data race
Chaque variable partagée est lue **et** écrite uniquement sous `sched_lock` :
- le monitor lit `last_compile_start` / `compile_count` sous `sched_lock` ;
- les codeurs les écrivent sous `sched_lock` (`do_grant`, `execute_compile`) ;
- `print_status` lit `simulation_end` sous `sched_lock` avant d'imprimer.

`memcheck` et `helgrind` le confirment (section 5).

---

## 2. Correctif helgrind — avertissement « dubious broadcast »

### Symptôme (avant)
```
pthread_cond_{signal,broadcast}: dubious: associated lock is not held by any thread
ERROR SUMMARY: 11 errors from 4 contexts
```
Ce n'était **pas** une vraie data race : tous les `pthread_cond_broadcast`
étaient déjà émis en tenant `sched_lock`. helgrind les signalait parce qu'au
démarrage un `broadcast` était émis **avant** qu'un seul thread n'ait exécuté un
`pthread_cond_wait` — helgrind n'avait donc pas encore appris l'association
condvar ↔ mutex.

### Correction
1. Ajout d'un compteur `int waiters` dans `t_environnement` (protégé par `sched_lock`).
2. Dans `do_grant`, `waiters++` avant la boucle d'attente et `waiters--` après.
   L'incrément précède le test `can_grant` ⇒ pas de réveil perdu (tout est
   sérialisé par `sched_lock`).
3. Chaque `broadcast` est gardé par `if (e->waiters > 0)` : on ne signale jamais
   une condition que personne n'attend.
4. Suppression du `broadcast` redondant de `request_dongles` : ajouter une requête
   au tas ne **libère** aucune ressource, donc ne peut jamais rendre un autre
   codeur éligible — réveiller les voisins était inutile, et c'était justement le
   `broadcast` qui partait en premier au démarrage.

Les seuls `broadcast` utiles restants : `execute_compile` (libération des clés)
et `monitor_routine` (fin de simulation). Le cas « cooldown pas encore expiré »
est géré par `pthread_cond_timedwait` (réveil autonome).

### Résultat (après)
`helgrind` : **0 errors, 0 dubious**, sur toutes les configurations (section 5).

---

## 3. Déterminisme du démarrage

### Symptôme
Au démarrage, la paire de codeurs servie en premier variait d'une exécution à
l'autre ({1,3} ou {2,4}), car `seq` était attribué via un compteur partagé
incrémenté au moment où chaque thread gagnait la course au `sched_lock`.

### Correction
Dans `request_dongles`, la **première** requête de chaque codeur reçoit
`seq = id` (au lieu du compteur) ; `seq_counter` est initialisé à `n` pour éviter
toute collision avec les `seq` suivants (`n, n+1, ...`) :
```c
if (c->compile_count == 0)
    c->req.seq = c->nb_of_coder - 1;   /* ordre de démarrage déterministe */
else
    c->req.seq = e->seq_counter++;     /* vrai ordre d'arrivée FIFO ensuite */
```
À `t=0`, le tas est donc ordonné par id ⇒ le codeur 1 est toujours servi en
premier, sous FIFO comme sous EDF (les deadlines EDF sont toutes égales à `t=0`
et le départage se fait sur `seq`).

### « 1 puis 3 » est correct, pas un bug
Les codeurs 1 et 3 sont **non adjacents** (ils ne partagent aucune clé), ils
peuvent donc compiler simultanément : c'est l'ordonnancement optimal
(*work-conserving*). Forcer un ordre strictement séquentiel 1→2→3→4 réduirait le
parallélisme et provoquerait plus de burnouts. L'exclusion mutuelle entre voisins
n'est **jamais** violée (vérifié, section 5).

---

## 4. EDF vs FIFO — pourquoi les sorties sont (quasi) identiques

**Conclusion : il n'existe aucun input où FIFO et EDF diffèrent significativement.**
Ce n'est pas un bug, c'est une propriété du problème symétrique. Deux raisons se
cumulent.

### Raison 1 — preuve mathématique (offset constant)
Pour la requête courante d'un codeur (cycle k) :
- `seq` correspond à l'instant d'arrivée `T_R = last_compile_start_{k-1} + C`,
  où `C = compile + debug + refactor` ;
- `deadline = last_compile_start_{k-1} + time_to_burnout`.

Donc `deadline = T_R + (time_to_burnout − C)`. Le terme `(time_to_burnout − C)`
est une **constante identique pour tous les codeurs** (paramètres globaux). L'ordre
des deadlines égale donc exactement l'ordre des arrivées ⇒ **EDF ≡ FIFO**. Même un
temps d'attente variable `W` ne change rien : il décale `T_R` et `deadline` de la
même quantité.

### Raison 2 — le départage des égalités (corrigé)
Quand les deadlines sont égales (toujours au démarrage, et souvent après arrondi
ms), `cmp_edf` se rabattait sur `seq`, c.-à-d. **exactement l'ordre FIFO**. Le
sujet exige le départage par **`coder_id` le plus élevé**. Corrigé :
```c
return (a->coder_id > b->coder_id);   /* départage EDF conforme au sujet */
```
Cela dit, même corrigé, l'effet reste invisible : l'ordonnanceur *work-conserving*
accorde les clés immédiatement plutôt que d'accumuler une file en attente, donc le
départage ne s'active quasiment jamais.

### Preuve empirique (test de contrôle)
On compare le bruit non-déterministe (même politique, 2 exécutions) à la
différence inter-politiques :
```
16 500 6 14 6 5 10 :  fifo-vs-fifo (bruit) = 374 lignes  |  fifo-vs-edf = 368 lignes
ordre des compilations (6 codeurs) : fifo-vs-fifo = 14 diff | fifo-vs-edf = 12 diff
```
**`fifo-vs-edf` < `fifo-vs-fifo`** : la différence inter-politiques est inférieure
au simple bruit d'ordonnancement de l'OS. Les deux politiques sont donc
statistiquement indiscernables — il n'y a pas d'input qui les sépare.

---

## 5. Résultats de tests

> ⚠️ Le shell par défaut est **zsh**, qui ne découpe pas `$args` non quoté.
> Utiliser des arguments littéraux ou `./codexion ${=args}`, sinon le programme
> reçoit un seul argument et n'affiche que l'usage (aucun thread créé).

### Compilation / norme
```
make re          # build propre, aucun warning (-Wall -Wextra -Werror -pthread)
make             # « Rien à faire » → pas de relink inutile
norminette *.c *.h   # tous les fichiers : OK!
```

### helgrind — 0 erreur, plus aucun « dubious »
```
valgrind --tool=helgrind ./codexion 4 800 10 30 10 3 20 edf  → ERROR SUMMARY: 0 errors, dubious=0
valgrind --tool=helgrind ./codexion 4 40 10 30 10 0 20 edf   → ERROR SUMMARY: 0 errors, dubious=0
valgrind --tool=helgrind ./codexion 1 100 10 30 10 2 20 edf  → ERROR SUMMARY: 0 errors, dubious=0
valgrind --tool=helgrind ./codexion 6 1500 8 20 8 5 15 fifo  → ERROR SUMMARY: 0 errors, dubious=0
```
(les compteurs « suppressed » élevés confirment que les threads ont bien tourné.)

### memcheck — 0 valeur non initialisée, 0 fuite
```
valgrind --tool=memcheck --leak-check=full --track-origins=yes -q ./codexion 4 800 10 30 10 3 20 edf
valgrind ... ./codexion 4 40 10 30 10 0 20 edf
valgrind ... ./codexion 1 100 10 30 10 2 20 fifo
→ uninit/invalid = 0, definitely/indirectly lost = 0
```

### Déterminisme (10 exécutions chacune)
```
fifo  premier codeur à compiler : 1 1 1 1 1 1 1 1 1 1
edf   premier codeur à compiler : 1 1 1 1 1 1 1 1 1 1
première paire parallèle        : 1 3
```

### Exclusion mutuelle
```
./codexion 4 100000 30 5 5 200 1 edf  → « OK: no adjacent simultaneous compiles »
```
(aucun couple de codeurs voisins ne compile en même temps)

### Burnout
```
Faisable   4 800 10 30 10 5 20 edf   → 0 burnout, 20 compilations (4×5)
Faisable   3 1000 10 30 10 10 20 fifo→ 0 burnout, 30 compilations (3×10)
Infaisable 4 40 10 30 10 0 20 edf    → 1 « burned out », dernière ligne, rien après
Infaisable 5 45 10 25 10 0 15 fifo   → 1 « burned out », dernière ligne, rien après
Infaisable 2 30 20 40 20 0 10 edf    → 1 « burned out », dernière ligne, rien après
```

---

## 6. Fichiers modifiés

| Fichier | Changement |
|---------|-----------|
| `codexion.h` | champ `int waiters` + prototypes `compute_timeout`, `block_until_signal` |
| `scheduler.c` | `do_grant` simplifié (waiters inc/dec), `request_dongles` (seq déterministe, suppression du broadcast redondant) |
| `sched_utils.c` | **nouveau** : `compute_timeout` + `block_until_signal` (respect de la norme ≤ 5 fonctions/fichier) |
| `thread.c` | broadcast de `execute_compile` gardé par `waiters > 0` |
| `monitor.c` | broadcast de `monitor_routine` gardé par `waiters > 0` |
| `main.c` | `waiters = 0`, `seq_counter = number_of_coder` |
| `codexion_utils.c` | `cmp_edf` : départage des deadlines égales par `coder_id` le plus élevé (conforme au sujet) |
| `Makefile` | ajout de `sched_utils.c` aux sources |
