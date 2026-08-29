# Codexion — Rôle de chaque fonction

Documentation fonction par fonction, fichier par fichier. Pour le détail de la
synchronisation (mutex, cond var, absence de data race), voir `THREAD_ANALYSIS.md`.

---

## `codexion.h` — types & prototypes

Pas de fonction, mais les structures clés :

| Structure | Rôle |
|-----------|------|
| `t_request` | Une demande de clés : `coder_id`, `deadline` (pour EDF), `seq` (ordre d'arrivée, pour FIFO). |
| `t_heap` | Le tas binaire (min-heap) : tableau de `t_request`, `size`, `capacity`, et un pointeur de fonction `cmp` (`cmp_fifo` ou `cmp_edf`). |
| `t_parsing_list` | Les 8 paramètres de la ligne de commande. |
| `t_info_coder` | L'état d'un codeur : son numéro, sa dernière compilation, son compteur, ses deux clés (gauche/droite), son drapeau `waiting`, sa requête, et un pointeur vers l'environnement. |
| `t_environnement` | L'état global partagé : paramètres, tableau de codeurs, mutex/état des clés, le tas, `sched_lock`, `queue_cond`, `log_lock`, `seq_counter`, `simulation_end`, `waiters`, `start_time`. |

---

## `main.c` — point d'entrée & initialisation

### `get_time_in_ms(void)` → `long long`
Renvoie l'heure courante en millisecondes (via `gettimeofday`). C'est l'horloge
de référence de tout le programme (timestamps des logs, deadlines, cooldowns).

### `init_allocations(t_environnement *env)` → `int`
Alloue tous les tableaux dimensionnés sur le nombre de codeurs `n` : `coder`,
`dongle_mutex`, `dongle_held`, `dongle_cooldown_ends`, le tas et son tableau.
Renvoie `0` si une allocation échoue, `1` sinon.

### `init_coder(t_environnement *env, int i)` → `void`
Initialise le codeur `i` : numéro (`i+1`), compteur à 0, `last_compile_start` =
heure de départ, `waiting` = 0, et surtout ses **deux clés** : gauche = `i`,
droite = `(i+1) % n` (disposition circulaire).

### `init_env_values(t_environnement *env)` → `void`
Initialise l'état global : crée les mutex (`log_lock`, `sched_lock`) et la cond
var (`queue_cond`), met `simulation_end = 0`, `waiters = 0`,
`seq_counter = n` (voir le départage déterministe), enregistre `start_time`,
choisit le comparateur du tas (`cmp_fifo` ou `cmp_edf` selon l'argument), puis
initialise chaque clé (mutex, cooldown à 0, libre) et chaque codeur.

### `main(int ac, char *av[])` → `int`
Orchestre le tout : vérifie le nombre d'arguments, parse, alloue, initialise,
alloue le tableau de threads, lance la simulation (`run_threads`), puis libère
tout (`free_all`).

---

## `parser.c` — validation des arguments

### `is_numeric(char *str)` → `int`
Renvoie `1` si la chaîne ne contient que des chiffres `0-9` (et n'est pas vide),
`0` sinon.

### `is_valid_values(char **things)` → `int`
Valide les 8 arguments : les 7 premiers doivent être des entiers ≥ 0, le nombre
de codeurs ≥ 1, et le scheduler doit être exactement `"fifo"` ou `"edf"`.

### `parser(char **things)` → `t_parsing_list *`
Si les arguments sont valides, alloue et remplit la structure des paramètres.
Renvoie `NULL` en cas d'argument invalide ou d'échec d'allocation.

---

## `heap.c` — tas binaire (file de priorité)

Min-heap : la racine `array[0]` est toujours l'élément de plus haute priorité
selon `cmp`.

### `sift_up(t_heap *h, int i)` → `void` *(static)*
Fait « remonter » l'élément à l'indice `i` tant qu'il est prioritaire sur son
parent. Utilisé après une insertion.

### `sift_down(t_heap *h, int i)` → `void` *(static)*
Fait « descendre » l'élément à l'indice `i` tant qu'un de ses enfants est
prioritaire. Utilisé après une suppression.

### `heap_push(t_heap *h, t_request req)` → `void`
Insère une requête à la fin du tableau puis la fait remonter (`sift_up`) pour
rétablir la propriété du tas.

### `heap_remove(t_heap *h, int coder_id)` → `void`
Retire la requête du codeur `coder_id` (recherche linéaire), la remplace par le
dernier élément, puis rééquilibre (`sift_up` ou `sift_down`). Sert quand un codeur
obtient ses clés ou quand la simulation s'arrête.

---

## `codexion_utils.c` — comparateurs, log, libération

### `cmp_fifo(t_request *a, t_request *b)` → `int`
Comparateur FIFO : `a` est prioritaire si son `seq` (ordre d'arrivée) est plus
petit. Premier arrivé, premier servi.

### `cmp_edf(t_request *a, t_request *b)` → `int`
Comparateur EDF : `a` est prioritaire si sa `deadline` est plus proche. **En cas
d'égalité de deadline, départage par `coder_id` le plus élevé** (conforme au
sujet).

### `heap_peek_id(t_heap *h)` → `int`
Renvoie le `coder_id` au sommet du tas (le plus prioritaire), ou `-1` si le tas
est vide. Sert à savoir si un codeur est en tête de file.

### `print_status(t_info_coder *coder, char *status)` → `void`
Affiche une ligne de log `timestamp coder_id status` de façon **atomique et
sérialisée** : prend `sched_lock` (pour lire `simulation_end` et figer le
timestamp) puis `log_lock` (pour que deux lignes ne s'entremêlent jamais).
N'affiche rien si la simulation est déjà terminée (évite les logs après burnout).

### `free_all(t_environnement *env)` → `void`
Détruit tous les mutex et la cond var, puis libère toute la mémoire allouée
(codeurs, tableaux de clés, tas, paramètres). Appelée à la fin du `main`.

---

## `sched_utils.c` — utilitaires d'ordonnancement

### `compute_timeout(t_info_coder *c)` → `long long`
Calcule l'instant (timestamp absolu en ms) où la prochaine des deux clés du
codeur sortira de cooldown. Renvoie `-1` si aucune clé n'est en cooldown.
Sert à donner un timeout précis à l'attente (`block_until_signal`).

### `block_until_signal(t_environnement *e, long long ms)` → `void`
Met le thread en attente sur `queue_cond` :
- si `ms < 0` (aucun cooldown en cours) → `pthread_cond_wait` (réveil uniquement
  par signal) ;
- sinon → `pthread_cond_timedwait` jusqu'à l'instant `ms` (réveil au plus tard à
  l'expiration du cooldown, même sans signal).

---

## `scheduler.c` — protocole d'acquisition des clés

### `is_ready(t_environnement *e, int idx)` → `int` *(static)*
Renvoie `1` si le codeur `idx` peut physiquement prendre ses clés **maintenant** :
ses deux clés ne sont pas détenues (`dongle_held`) **et** ne sont plus en cooldown.

### `can_grant(t_info_coder *c)` → `int` *(static)*
Décide si on accorde les clés au codeur `c`. Logique *work-conserving* :
1. si ses clés ne sont pas prêtes → non ;
2. s'il est en tête du tas → oui ;
3. sinon, il cède uniquement à un **voisin** (gauche/droite) qui est en attente,
   plus prioritaire **et** prêt — sinon il prend (un codeur non adjacent prioritaire
   ne le bloque pas, d'où le parallélisme : 1 et 3 compilent ensemble).

### `do_grant(t_info_coder *c)` → `void` *(static)*
Cœur de l'attente. Incrémente `waiters`, boucle tant que la simulation tourne et
que `can_grant` est faux (en se mettant en sommeil via `block_until_signal`),
puis décrémente `waiters`. Une fois accordé : retire sa requête du tas, marque ses
deux clés comme détenues et met à jour `last_compile_start`. Si la simulation s'est
terminée pendant l'attente, sort sans rien prendre. Tout se passe sous `sched_lock`.

### `request_dongles(t_info_coder *c)` → `void`
Point d'entrée appelé par le codeur à chaque cycle. Sous `sched_lock` : construit
sa requête (`coder_id`, `deadline = last_compile_start + time_to_burnout`, et
`seq` — **déterministe = id à la 1re requête**, sinon compteur global), la pousse
dans le tas, puis appelle `do_grant`. Libère `sched_lock` au retour.

---

## `thread.c` — threads des codeurs

### `execute_compile(t_info_coder *c)` → `void`
Exécute la phase de compilation : affiche « is compiling », dort
`time_to_compile`, puis sous `sched_lock` incrémente le compteur de compilations,
arme le cooldown des deux clés, les marque libres, et réveille les éventuels codeurs
en attente (`if (waiters > 0) broadcast`). Relâche enfin les deux mutex physiques
des clés.

### `take_physical_dongles(t_info_coder *c)` → `void` *(static)*
Verrouille les deux mutex **physiques** des clés dans l'ordre croissant d'id (clé
basse puis clé haute — ordre asymétrique qui empêche structurellement le deadlock
circulaire), en affichant « has taken a dongle » à chaque prise.

### `handle_single_coder(t_info_coder *c)` → `void *` *(static)*
Cas particulier d'**un seul codeur** : il n'y a qu'une clé, donc il ne peut jamais
en avoir deux. Il prend l'unique clé, attend la fin de la simulation (il finira par
burn out), puis relâche.

### `coder_routine(void *arg)` → `void *`
Fonction de thread d'un codeur. Boucle tant que la simulation tourne :
`request_dongles` → (vérifie la fin) → `take_physical_dongles` → `execute_compile`
→ « is debugging » (dort `time_to_debug`) → « is refactoring » (dort
`time_to_refactor`). Bascule sur `handle_single_coder` si `n == 1`.

### `run_threads(t_environnement *env, pthread_t *mon, pthread_t *co)` → `int`
Crée le thread monitor puis les `n` threads codeurs, puis attend (`pthread_join`)
tous les codeurs et enfin le monitor. Renvoie `0` si une création échoue, `1` sinon.

---

## `monitor.c` — détection du burnout & fin de simulation

### `sim_ended(t_environnement *env)` → `int`
Lit `simulation_end` sous `sched_lock` (lecture thread-safe). Utilisée par les
codeurs pour savoir s'ils doivent s'arrêter.

### `check_burnout(t_environnement *env, int i)` → `int` *(static)*
Vérifie si le codeur `i` a dépassé sa deadline (`now - last_compile_start >
time_to_burnout`). Si oui : met `simulation_end = 1`, affiche « burned out » (sous
`log_lock`) et renvoie `1`.

### `all_reached_quota(t_environnement *env)` → `int` *(static)*
Renvoie `1` si **tous** les codeurs ont atteint `number_of_compiles_required`
compilations (et que ce quota est > 0). Condition d'arrêt « succès ».

### `monitor_check(t_environnement *env)` → `int` *(static)*
Parcourt tous les codeurs pour détecter un burnout ; sinon vérifie le quota
global. Renvoie `1` (et arme `simulation_end`) si la simulation doit s'arrêter.

### `monitor_routine(void *arg)` → `void *`
Fonction de thread du monitor. Boucle : prend `sched_lock`, teste fin/burnout/quota
(`monitor_check`) ; si arrêt, réveille les codeurs en attente
(`if (waiters > 0) broadcast`) et sort. Sinon relâche le verrou et dort 500 µs
(détection du burnout bien en deçà des 10 ms exigées par le sujet).
