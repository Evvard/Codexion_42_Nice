*This project has been created as part of the 42 curriculum by <ins>eolivier</ins>*

***Description***<br>
Maîtrisez la programmation concurrente en C grâce à une simulation intense où les programmeurs luttent contre l'épuisement professionnel tout en se disputant de rares clés USB. Mettez en œuvre des threads POSIX, des mutex, des variables de condition et des algorithmes de planification sophistiqués (FIFO/EDF) pour coordonner le partage des ressources, éviter les blocages et garantir un accès équitable, tout en maintenant la productivité de vos programmeurs avant que la date limite n'arrive et donc la fin du programme.
</br>

***Instructions***
Pour compiler le projet il faut executer: <br>
make<br>
mais aussi: <br>
make all, make clean, make fclean, make re<br>
puis mettre les arguments:<br>
number_of_coders : nombre de codeurs<br>
time_to_burnout : temps en ms avant le burnout des codeurs (si 1 est en burnout : fin du programme)<br>
time_to_compile : temps de compiltion en ms<br>
time_to_debug : temps de debug en ms<br>
time_to_refactor : temps de factorisation en ms<br>
number_of_compiles_required : nombre de codeur<br>
dongle_cooldown: temps avant de pouvoir reutiliser les cles usb pour compiler<br>
scheduler : "fifo"(premier arrive premier servis) ou "edf"(premier a bientot mourrir premier servis)<br>


exemple :<br>
***✗ ./Codexion 4 500 10 30 10 10 20 edf***<br>


***Resources :***

https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2 : toute la playlist.<br>
https://www.geeksforgeeks.org/c/multithreading-in-c/<br>
Gemini 3.1, 

***Ia Usage:***
Makefile et un peu de logique supplement debugging


***Cas de blocage gérés***

**1. Prévention de l'interblocage (deadlock) et conditions de Coffman**

Le problème classique de l'interblocage (chaque thread détient une ressource et attend l'autre indéfiniment) est prévenu par l'ordonnanceur `is_prioritarian()`. Avant toute tentative d'acquisition des dongles, un codeur vérifie s'il est prioritaire par rapport à ses voisins. En mode EDF, un seul codeur à la fois est autorisé à progresser vers l'acquisition des deux ressources, ce qui supprime la condition de Coffman *"hold and wait"* (détenir une ressource en attendant une autre) pour les autres threads.

**2. Prévention de la famine (starvation)**

En mode EDF (*Earliest Deadline First*), `is_prioritarian()` compare le temps restant avant burnout du codeur courant (`last_compile_start + time_to_burnout`) avec celui de ses voisins gauche et droit. Seul le codeur dont la deadline est la plus imminente parmi ses voisins obtient le droit de compiler. Cela garantit qu'aucun codeur ne soit bloqué indéfiniment en attente de ressources jusqu'à atteindre le burnout.

**3. Gestion du cooldown des dongles**

Après chaque compilation, `execute_compile()` enregistre dans `dongle_cooldown_ends[left]` et `dongle_cooldown_ends[right]` l'horodatage de fin de cooldown (`get_time_in_ms() + dongle_cooldown`), sous protection de `state_mutext`. La fonction `try_take_dongles()` vérifie ces timestamps avant toute acquisition : si l'un des deux dongles est encore en cooldown, le codeur abandonne immédiatement sans verrouiller quoi que ce soit, évitant toute attente bloquante sur une ressource temporairement indisponible.

**4. Détection précise du burnout**

Le thread monitor vérifie en continu, sous `state_mutext`, si `get_time_in_ms() - coder[i].last_compile_start > time_to_burnout` pour chaque codeur. Le champ `last_compile_start` est mis à jour atomiquement au tout début de `execute_compile()` (sous `state_mutext`), ce qui garantit que la mesure du temps sans compilation est précise et sans race condition entre le codeur qui met à jour et le monitor qui lit.

**5. Sérialisation des logs**

Toute écriture sur la sortie standard passe par `print_status()`, qui acquiert `log_mutext` avant d'appeler `printf`. Même si plusieurs threads atteignent simultanément un état à journaliser, les lignes `"timestamp id état"` sont émises une à une, sans interleaving, rendant la sortie lisible et exploitable.


***Mécanismes de synchronisation des threads***

**1. `pthread_mutex_t dongle_mutext[N]` — tableau de N mutexes (un par dongle)**

Chaque clé USB dispose de son propre mutex. Pour compiler, un codeur verrouille d'abord `dongle_mutext[left_dongle]` puis `dongle_mutext[right_dongle]`. Les dongles ne sont relâchés qu'à la fin de `execute_compile()`, après la mise à jour du cooldown. L'ordonnanceur réduit la contention en limitant le nombre de codeurs qui tentent simultanément d'acquérir ces verrous.

**2. `pthread_mutex_t log_mutext` — sérialisation de la sortie console**

Mutex global utilisé uniquement dans `print_status()`. Il protège l'intégralité de l'opération d'écriture (lecture du timestamp + `printf`), garantissant que les messages ne se mélangent jamais entre threads. Dans `print_status()`, `log_mutext` est toujours acquis avant `state_mutext` — cet ordre fixe prévient tout interblocage entre ces deux mutex.

**3. `pthread_mutex_t state_mutext` — protection de l'état partagé**

Mutex global couvrant toutes les données partagées sensibles : `simulation_end`, `last_compile_start`, `compile_count`, et `dongle_cooldown_ends[]`. Utilisé par le monitor dans `monitor_routine()` et par les codeurs dans `try_take_dongles()`, `execute_compile()` et `coder_routine()`.

Exemple de prévention de race condition : dans `try_take_dongles()`, la vérification du cooldown et de la priorité est effectuée entièrement sous `state_mutext` avant de relâcher ce verrou et de tenter de verrouiller les dongles. Cela évite qu'un codeur ne lise des timestamps en cours de mise à jour par un autre thread.

**4. Polling actif (sans `pthread_cond_t`)**

Quand un codeur ne peut pas prendre les dongles (cooldown actif ou non prioritaire), il appelle `usleep(50)` et réessaie à la prochaine itération. Le monitor procède de même avec `usleep(1000)` entre chaque vérification globale. Cette approche évite la complexité des variables de condition tout en maintenant une latence de réaction de 50 µs pour les codeurs.

**5. Communication monitor ↔ codeurs via `simulation_end`**

`simulation_end` est un entier partagé dans `t_environnement`, protégé par `state_mutext`. Lorsque le monitor détecte un burnout ou que le quota de compilations est atteint, il positionne `simulation_end = 1`. Chaque codeur vérifie ce flag en début de boucle (sous `state_mutext`) et se termine proprement via `return`. C'est le mécanisme de terminaison coordonnée entre le monitor et tous les threads codeurs.
