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


***Cas de blocage gérés***:
Blocage gerer grace au mutexte dans les fonction qui sont lock ou delock en fonction de la situation et tous relier a la structure env donc vu que touts les threads y sont relier cela permet sans difficulter d'eviter toutes racecondition ou autre


***Mécanismes de synchronisation des threads***
