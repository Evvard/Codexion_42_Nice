*This project has been created as part of the 42 curriculum by eolivier*

# Description

Maîtrisez la programmation concurrente en C grâce à une simulation où des codeurs luttent contre le burnout tout en se disputant de rares clés USB (dongles). Le programme met en œuvre des threads POSIX, des mutex, une variable de condition et un ordonnanceur à file de priorité (FIFO/EDF) pour coordonner le partage des dongles, éviter les interblocages et garantir un accès équitable, jusqu'à ce que tous les codeurs aient atteint leur quota de compilations — ou qu'un codeur burn out, ce qui arrête la simulation.
</br>


# Instructions

Pour compiler le projet :
```zsh
make
```

Les autres règles du Makefile :
```zsh
make all      # identique à make
make clean    # supprime les fichiers objets
make fclean   # supprime les objets et le binaire
make re       # fclean puis all
```

Le programme prend 8 arguments, tous obligatoires, dans cet ordre :
```
- number_of_coders : nombre de codeurs, égal au nombre de dongles sur la table

- time_to_burnout : temps en ms au-delà duquel un codeur burn out, compté depuis
  le DÉBUT de sa dernière compilation (ou depuis le début de la simulation)

- time_to_compile : temps de compilation en ms (le codeur tient 2 dongles)

- time_to_debug : temps de debug en ms

- time_to_refactor : temps de refactorisation en ms

- number_of_compiles_required : nombre de compilations que CHAQUE codeur doit
  atteindre pour que la simulation s'arrête

- dongle_cooldown : temps en ms pendant lequel un dongle reste indisponible
  après avoir été relâché

- scheduler : "fifo" (premier arrivé, premier servi) ou "edf" (Earliest Deadline
  First : priorité au codeur dont la deadline last_compile_start +
  time_to_burnout est la plus proche ; à deadlines égales, priorité au
  coder_id le plus élevé)
```

Exemples :
```zsh
./codexion [coders] [burnout] [compile] [debug] [refactor] [nb_compilations] [cooldown] [mode]

./codexion 4 800 200 200 60 5 60 edf     # succes : tous atteignent le quota
./codexion 5 200 100 100 100 10 60 fifo  # burnout : "201 1 burned out"
```

**Le burnout est inévitable si `time_to_burnout` \< `time_to_compile` + `time_to_debug` + `time_to_refactor`** : le codeur ne peut pas revenir compiler avant sa deadline.

Attention, ce n'est qu'une borne inférieure. Les codeurs sont en cercle et deux voisins partagent un dongle, donc au plus `n / 2` codeurs compilent en parallèle. Une condition de faisabilité plus réaliste est :

```
time_to_burnout  >  ceil(n / 2) x (time_to_compile + dongle_cooldown)
```

Exemple : `5 800 200 200 60 5 60` donne 3 x 260 = 780 ms pour une deadline de 800 ms. La marge de 20 ms est trop faible et un burnout survient de manière intermittente (observé environ 1 fois sur 12, identiquement en fifo et en edf : c'est un problème de paramètres, pas d'ordonnancement). Prévoir une marge confortable.

# Resources

Youtube playlist : [Ytb](https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)  
https://www.geeksforgeeks.org/c/multithreading-in-c/  
Gemini 3.1

### Ia Usage
L'IA a été utilisée pour la rédaction du Makefile, un appui ponctuel sur la logique d'ordonnancement, et le débogage.

# Cas de blocage gérés

***1. Prévention de l'interblocage (Deadlock) et conditions de Coffman***  
Le scénario classique d'interblocage des philosophes (où chaque thread détient une ressource et attend indéfiniment la seconde) est évité par conception, sur deux niveaux. D'abord, la condition *Hold and Wait* est éliminée par l'ordonnanceur : dans `request_dongles`, un codeur n'obtient l'autorisation de compiler que lorsque ses **deux** dongles sont simultanément libres et hors cooldown (`is_ready`), et il ne touche aux mutex physiques qu'après cette validation. Ensuite, la condition d'*attente circulaire* est cassée par l'ordre d'acquisition : `take_physical_dongles` verrouille toujours le mutex d'indice le plus bas en premier.

***2. Prévention de la famine (Starvation)***  
Sous les deux ordonnanceurs, l'arbitrage passe par un min-heap de requêtes. En mode FIFO, l'ordre est donné par un compteur de séquence monotone (`seq_counter`), incrémenté à chaque nouvelle requête ; la première requête de chaque codeur est amorcée avec son propre identifiant, ce qui rend le démarrage déterministe. En mode EDF, la priorité va au codeur dont la deadline `last_compile_start + time_to_burnout` est la plus proche, et à deadlines exactement égales au codeur ayant le `coder_id` le plus élevé (`cmp_edf`).

L'ordonnancement est **work-conserving** et non pas strictement global : un codeur n'est pas obligé d'attendre la tête du heap. Il démarre s'il est en tête, ou si aucun de ses deux voisins immédiats n'est simultanément en attente, plus prioritaire que lui et prêt à démarrer (`can_grant`). C'est ce qui permet à plusieurs codeurs non adjacents de compiler en parallèle au lieu de sérialiser inutilement toute la table — et donc ce qui évite des burnouts artificiels.

***3. Gestion du Cooldown des Dongles***  
Lorsqu'un codeur termine sa compilation, il enregistre pour ses deux dongles une date de fin de cooldown (`dongle_cooldown_ends`) sous le verrou de l'ordonnanceur, avant même de relâcher les mutex physiques. Tout codeur qui évalue la disponibilité des ressources considère un dongle comme indisponible tant que l'heure courante n'a pas dépassé cette date (`is_ready`), ce qui empêche toute reprise anticipée. Un codeur en attente d'un cooldown ne tourne pas à vide : `compute_timeout` calcule la date de fin de cooldown la plus proche et le thread se rendort jusque-là.

***4. Détection Précise du Burnout***  
Un thread monitor distinct parcourt en boucle l'état de tous les codeurs et compare le temps écoulé depuis le début de leur dernière compilation à `time_to_burnout`. Sa période de scrutation est de 500 µs, ce qui place l'affichage du message de burnout largement sous la limite des 10 ms imposée par le sujet (0 à 1 ms mesuré). Le monitor arrête aussi la simulation lorsque tous les codeurs ont atteint le quota de compilations.

***5. Sérialisation Stricte des Logs***  
Pour éviter tout entrelacement de lignes sur la sortie standard, tout l'affichage passe par `print_status`, qui prend le verrou de l'ordonnanceur puis un mutex dédié aux logs avant d'écrire. Cette double prise garantit qu'un message est cohérent avec l'état qu'il décrit et qu'aucun log ne peut sortir après la fin de la simulation : `print_status` ne réaffiche rien si `simulation_end` est déjà positionné, ce qui garantit que la ligne `burned out` est bien la dernière du programme.

***6. Fin de Simulation Sans Blocage***  
Un codeur endormi dans `pthread_cond_wait` ne doit ni rester bloqué à la fin de la simulation, ni provoquer un réveil sur une condition inexistante. Le nombre de threads réellement en attente est donc suivi explicitement (`waiters`), et un `pthread_cond_broadcast` n'est émis que si ce compteur est non nul — que le réveil vienne d'un codeur qui relâche ses dongles ou du monitor qui termine la simulation. À la sortie de l'attente, chaque codeur revérifie `simulation_end` avant de continuer, ce qui garantit la terminaison propre de tous les threads.

# Mécanismes de synchronisation des threads

***1. `pthread_mutex_t *dongle_mutex`***  
Un tableau de mutex, un par dongle USB, qui matérialise la possession physique de la ressource. Un codeur verrouille toujours le mutex d'indice le plus bas en premier, puis le plus élevé (`take_physical_dongles`), ce qui empêche structurellement toute attente circulaire. C'est aussi le point d'émission des deux messages `has taken a dongle`.

***2. `pthread_mutex_t log_lock`***  
Un mutex dédié à la sérialisation des écritures sur la sortie standard, pris dans `print_status` et par le monitor lors de l'affichage du burnout. Il garantit que les messages de threads différents ne se chevauchent jamais sur une même ligne.

***3. `pthread_mutex_t sched_lock`***  
Le verrou de section critique de l'ordonnanceur. Il protège l'ensemble de l'état partagé : le min-heap des requêtes, les drapeaux `dongle_held`, les dates `dongle_cooldown_ends`, les compteurs de compilation, les `last_compile_start` et le drapeau `simulation_end`. Le monitor et les codeurs le prennent tous, ce qui exclut toute lecture/écriture concurrente sur ces données (validé par `valgrind --tool=helgrind` : 0 erreur).

***4. `pthread_cond_t queue_cond`***  
Plutôt qu'un polling actif, l'attente des dongles passe par une variable de condition. `block_until_signal` appelle `pthread_cond_wait` lorsque le codeur attend seulement la libération d'un dongle, et `pthread_cond_timedwait` lorsqu'il attend en plus la fin d'un cooldown, avec pour échéance la date calculée par `compute_timeout`. Dans les deux cas `sched_lock` est relâché pendant l'attente et repris au réveil. Les réveils sont émis par `pthread_cond_broadcast`, uniquement lorsque `waiters > 0`, soit à la libération des dongles, soit à la fin de la simulation.




README corrigé, français conservé, intitulés de sections inchangés. Aucun code touché (README.md seul modifié, build et norminette toujours OK).

Erreurs factuelles corrigées

1. ft_usleep n'existe pas. C'était le plus grave. La section 6 décrivait « une attente active segmentée en micro-sommeils de 500 µs » via un ft_usleep personnalisé — introuvable dans tout le projet (grep -rn ft_usleep ne le trouvait que dans le README). Le cont. Le chiffre de 500 µs venait en réalité de monitor.c:93, la période descrutation du monitor. La section 6 est réécrite sur ce qui existe vraiment : la terminaison sans blocage via le compteur waiters et les broadcasts gardés.

2. Noms de mutex inventés. Le README citait log_mutext, state_mutext, dongle_mutext[] — aucun n'existe. Les vrais sont log_lock, sched_lock, dongle_mutex. Et il n'y a pas de mutex
d'état séparé : sched_lock joue les deux rôles. Un évaluateur qui grep le hedécalage.

3. Inégalité inversée (l'ancienne ligne 44) : > → <, vérifié dans les deux s

4. « bloc d'impression atomique » fictif. La section 5 affirmait que les deu compiling étaient regroupés dans une seule impression atomique. Faux :print_status prend et relâche les verrous à chaque message. Remplacé par le vrai garde-fou, qui est meilleur à défendre : print_status n'imprime rien si simulation_end est déjà posé,
ce qui garantit que burned out est la dernière ligne.

5. number_of_compiles_required : nombre de codeur → nombre de compilations qre.

6. FIFO « timestamp d'entrée » → c'est un compteur de séquence monotone seq_ation « accordées strictement dans l'ordre d'arrivée » était fausse :can_grant est work-conserving. La nuance est maintenant explicite, avec la justification (c'est ce qui autorise le parallélisme des non-voisins et évite les faux burnouts).

7. Section 1 (deadlock) incomplète : elle n'attribuait la prévention qu'au heap. L'ordre d'acquisition croissant des mutex dans take_physical_dongles — le vrai casseur d'attente
circulaire — n'était pas mentionné.

8. Divers : lien YouTube cassé (# collé devant l'URL), make clean/fclean/re  compilation, « temps de factorisation » → refactorisation, typo « compiltion », time_to_burnout sans préciser qu'il court depuis le début de la dernière compilation.

Un point découvert en route

Les deux exemples du README (4 500 10 30 10 10 20 et 4 40 10 30 10 10 20) utilisaient des timings sous 60 ms, que la grille interdit de tester. Je les ai remplacés par deux exemples
vérifiés : 4 800 200 200 60 5 60 edf (0 burnout sur 18 runs) et 5 200 100 10d out, 6/6).

En les cherchant j'ai trouvé que 5 800 200 200 60 5 60 burn out environ 1 foifo et en edf. Ce ne sont pas les threads : avec 5 codeurs en cercle, au plus 2 compilent en parallèle, donc un tour complet coûte 3 × (200 + 60) = 780 ms pour une deadline de 800 ms — 20 ms de marge. Ce sont des paramètres infaisables en pratique, pas un défaut d'ordonnancement, et la grille prévoit ce cas (« discuss borderline c la condition de faisabilité time_to_burnout > ceil(n/2) × (time_to_compile + dongle_cooldown) dans les Instructions — utile à l'oral, et surtout à ne pas utiliser comme démo de succès.