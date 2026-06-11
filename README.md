*This project has been created as part of the 42 curriculum by eolivier*

# Description

Maîtrisez la programmation concurrente en C grâce à une simulation intense où les programmeurs luttent contre l'épuisement professionnel tout en se disputant de rares clés USB. Mettez en œuvre des threads POSIX, des mutex, des variables de condition et des algorithmes de planification sophistiqués (FIFO/EDF) pour coordonner le partage des ressources, éviter les blocages et garantir un accès équitable, tout en maintenant la productivité de vos programmeurs avant que la date limite n'arrive et donc la fin du programme
</br>


# Instructions
Pour compiler le projet il faut executer:    
```zsh
make
make all
make clean
make fclean
make re
```
Puis ils faut mettres les arguments :
```
- number_of_coders

- time_to_burnout : temps en ms avant le burnout des codeurs

- time_to_compile : temps de compiltion en ms

- time_to_debug : temps de debug en ms

- time_to_refactor : temps de factorisation en ms  

- number_of_compiles_required : nombre de codeur  

- dongle_cooldown: temps avant de pouvoir reutiliser les cles usb pour compiler  

- scheduler : "fifo"(premier arrive premier servis) ou "edf"(premier a bientot mourrir premier servis)
```

exemple :  
```zsh
./codexion [coder] [burnout] [compile] [debug] [refactor] [nb_compilation] [cle_usb_cooldown] [mode]

./codexion 4 500 10 30 10 10 20 edf - succes
./codexion 4 40 10 30 10 10 20 edf - burn out
```
**Le burnout est obligatoire si `time_to_burnout` \> `time_to_compile` + `time_to_debug` + `time_to_refactor`**

# Resources


Youtube playlist: [Ytb](#https://www.youtube.com/watch?v=d9s_d28yJq0&list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2).  
https://www.geeksforgeeks.org/c/multithreading-in-c/<br>
Gemini 3.1  

### Ia Usage
Makefile et un peu de logique supplement debugging

# Cas de blocage gérés

***1. Prévention de l'interblocage (Deadlock) et conditions de Coffman***  
Le scénario classique d'interblocage des philosophes (où chaque thread détient une ressource et attend indéfiniment la seconde) est évité par conception. Les codeurs doivent acquérir leurs dongles gauche et droit en toute sécurité. Pour éliminer la condition Hold and Wait (Détenir et Attendre), le Min-Heap orchestre l'accès : un codeur ne tente de verrouiller les mutex physiques que lorsqu'il a validé sa priorité et la disponibilité du matériel auprès de l'ordonnanceur.  

***2. Prévention de la famine (Starvation)***  
Sous les deux ordonnanceurs (FIFO et EDF), l'équité est garantie. En mode FIFO, les requêtes sont accordées strictement dans l'ordre d'arrivée en utilisant un timestamp d'entrée. En mode EDF, le codeur dont la deadline de burnout est la plus proche est prioritaire. Pour assurer un déterminisme parfait et empêcher la famine systémique, une règle de départage (tie-breaker) exige de prioriser le codeur ayant le coder_id le plus élevé en cas d'égalité exacte des deadlines.  

***3. Gestion du Cooldown des Dongles***  
Lorsqu'un codeur relâche ses dongles, une période de refroidissement (cooldown) matériel est appliquée. L'heure exacte de fin du cooldown est enregistrée en toute sécurité sous un mutex d'état global. Tout codeur vérifiant la disponibilité des ressources bypassera les dongles si le temps système actuel n'a pas encore dépassé le timestamp de refroidissement, empêchant ainsi le vol prématuré des ressources (race condition).  

***4. Détection Précise du Burnout***  
Un thread monitor distinct vérifie constamment le statut de tous les codeurs. Il calcule le temps écoulé depuis le début de leur dernière compilation. Si un codeur manque sa deadline, le monitor stoppe la simulation et imprime le log de burnout dans la limite stricte des 10 ms imposée par le sujet.  

***5. Sérialisation Stricte des Logs et Blocs Atomiques***  
Pour éviter les data races (accès concurrents) et l'entrelacement des lignes sur la sortie standard, tous les logs sont sérialisés à l'aide d'un mutex global dédié (log_mutext). De plus, les actions séquentielles rapides (comme l'acquisition des deux dongles suivie instantanément de la compilation) sont regroupées dans un seul bloc d'impression atomique. Cela empêche l'ordonnanceur de l'OS de suspendre un thread au milieu de son action, garantissant une cohérence chronologique parfaite et éliminant tout risque d'invalidation due à des logs entremêlés.  

***6. Précision Temporelle à Haute Concurrence (ft_usleep)***  
La fonction standard usleep() est soumise aux latences de l'OS (context-switching), qui s'accumulent de manière désastreuse lors de la gestion de dizaines ou centaines de threads (ex: 200 codeurs), entraînant des retards artificiels et de faux burnouts. Pour garantir une précision absolue, ce projet utilise un ft_usleep personnalisé. Il s'agit d'une attente active segmentée en micro-sommeils de 500µs, vérifiant en continu le temps exact écoulé via gettimeofday() tout en surveillant le flag simulation_end pour éviter tout freeze en fin de programme.  

# Mécanismes de synchronisation des threads

***1. `pthread_mutex_t dongle_mutext[]`***  
Un tableau de mutex où chaque dongle USB individuel est protégé. Les codeurs doivent strictement verrouiller le dongle avec l'ID le plus bas en premier, puis le dongle avec l'ID le plus élevé (ordre mathématique asymétrique). Cela force une stratégie d'allocation des ressources qui empêche structurellement les deadlocks circulaires.  

***2. `pthread_mutex_t log_mutext`***  
Un mutex dédié à la sérialisation des sorties dans le terminal lors des appels à print_status(), garantissant que les messages d'état des différents threads ne se chevaucheront jamais.  

***3. `pthread_mutex_t state_mutext`***  
Un mutex de section critique protégeant toutes les variables de simulation partagées (simulation_end, deadlines, compteurs de compilation, et les tableaux de cooldown). Il empêche les données d'être corrompues par des lectures/écritures simultanées entre le monitor et les codeurs.  

***4. `pthread_cond_t queue_cond`***  
Au lieu d'un polling actif (des boucles infinies de usleep destructrices pour le CPU), la synchronisation des files d'attente est gérée via une variable de condition. Lorsqu'un codeur attend ses dongles, il se met en sommeil avec pthread_cond_wait, déverrouillant automatiquement le state_mutext pour les autres. Il est instantanément réveillé de manière asynchrone par un signal pthread_cond_broadcast émis dès qu'un autre codeur relâche ses ressources ou que la simulation est interrompue.