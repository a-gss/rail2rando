# rail2rando
Le but est d'automatiser la recherche de circuits de randonnées accessibles en train (région Occitanie pour l'instant) pour les gens qui n'ont pas de voiture (moi par exemple).

## Comment ça marche
Comme la plupart des systèmes de transport public, le réseau TER LIO-Occitanie (et celui tout le territoire français en fait) utilise le format [`GTFS`](https://gtfs.org/fr/) pour le décrire. Les données `GTFS` sont rendues publiques sur [le site de la région](https://data.laregion.fr/explore/dataset/reseau-lio/table/). Parmis les fichiers nécéssaires pour calculer des trajets, il faut :
- `calendar.txt`
- `calendar_dates.txt`
- `routes.txt`
- `stop_times.txt`
- `stops.txt`
- `trips.txt`

Le programme liste **tout** les trajets possibles et les trie par ordre décroissant d'arrivée à la randonnée (au plus tôt on arrive, mieux c'est).
1. On parcourt les données `GTFS` afin de calculer tout les itinéraires en train depuis une gare de départ à un jour donné [^1]
2. Pour chaque gare à laquelle on arrive avant une heure limite *(par exemple 10h ; pas besoin de s'encombrer des gares où on arrive à 18h)*, on analyse toutes les données disponibles publiquement ([data.gouv.fr](https://www.data.gouv.fr/fr/datasets/?q=randonn%C3%A9e)) et en webscrappant [IGNrando](https://ignrando.fr/)[^2]
3. On filtre les résultats, par exemple *"à moins de 2km à pied de la gare"* car la plupart des randonnées se trouvent à plusieurs km d'une ville
4. On fait une requête météo pour avoir la météo parce que c'est sympa de savoir
5. On arrange le tout dans `stdout` (et/ou export HTML) avec les liens des topos, la distance, l'heure à laquelle la randonnée peut commencer, la météo etc, etc...

[^1]: On fixe l'heure à minuit pour être sûr d'avoir le premier train, en supposant qu'une rando commence le plus tôt possible sauf si vous êtes bizarres
[^2]: Pas trouvé quoi que ce soit sur le scraping dans les [CGU](https://ignrando.fr/fr/cgu) donc on va faire genre ça passe

## Compilation
- Mac OS: ouais (avec clang)
- Linux: ouais (avec gcc)
- Windows: non (ouais avec WSL2)

J'utilise une seule translation unit donc il suffit de faire:
```
gcc rail2rando.c -o rail2rando
./rail2rando
```

## Objectifs
- Multithreader le scraping
- Exporter en `.html`

## Addendum
J'avais commencé à prototyper l'étape (1) dans le script shell `lio.sh` avec des requêtes `curl` sur le serveur [plan.lio-occitanie.fr](www.plan.lio-occitanie.fr) mais c'était lent et j'étais pas au courant pour le `GTFS`, on peut tout faire en local !
