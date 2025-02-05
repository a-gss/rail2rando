# rail2rando
Le but est d'automatiser la recherche de circuits de randonnées accessibles en train (région Occitanie pour l'instant) pour les gens qui n'ont pas de voiture (moi par exemple).

## Comment ça marche
1. On demande au serveur [plan.lio-occitanie.fr](https://plan.lio-occitanie.fr) de calculer un itinéraire en train depuis notre gare de départ à un jour donné [^1]
2. On répète (1) pour toutes les gares de la région. On a alors les trajets les plus lents et les plus rapides.
3. Pour chaque gare, on va interroger [IGNrando](https://ignrando.fr/) et on scrape les données des circuits obtenus [^2]
4. On filtre les résultats, par exemple "à moins de 2km à pied de la gare" car la plupart des randonnées se trouvent à plusieurs km d'une ville
5. On fait une requête météo pour avoir la météo parce que c'est sympa de savoir
6. On arrange le tout dans `stdout` (et/ou export HTML) avec les liens IGNrando pour aller chopper le topo, des infos comme la distance, l'heure à laquelle la randonnée peut commencer, la météo etc, etc...

[^1]: On fixe l'heure à minuit pour être sûr d'avoir le premier train, en supposant qu'une rando commence le plus tôt possible sauf si vous êtes bizarres
[^2]: Pas trouvé quoi que ce soit sur le scraping dans les [CGU](https://ignrando.fr/fr/cgu) donc on va faire genre ça passe

## Objectifs
- Faire un programme CLI (en C évidemment), pas un vieux script shell tout pourri
- Multithreader le scraping
- Exporter en `.html`

## Progrès
J'ai commencé par prototyper l'étape (1) dans un script shell `lio[n].sh`. Chaque version apporte des fonctionnalités en plus. J'ai tiré le maximum du maximum que je pouvais pour réduire le temps d'exécution, par exemple en passant outre le DNS dans la commande `curl` en lui passant directement l'adresse IP du serveur, on peut gratter 1.4 secondes !! c'est crazy dingo. En analysant ce qui prend le plus de temps dans la requête `curl` :
```
DNS: 0.000011 Connect: 0.017311 PreTransfer: 0.067856 StartTransfer: 0.067857 Total: 0.301393
```
On voit `StartTransfer` à 70ms et total à 300ms, donc 230ms sont nécéssaires pour télécharger la réponse du serveur. Les données qui nous intéressent sont au début de la réponse, il faut trouver un moyen d'interrompre `curl` quand on a trouvé ce qui nous intéresse ; chose quasi impossible en bash, ou en tout cas j'ai essayé et c'était vraiment chiant et ça marchait pas. Avec `libcurl` en C c'est beaucoup plus simple. 
