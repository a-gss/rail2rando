# rail2rando
Le but est d'automatiser la recherche de circuits de randonnées accessibles en train (région Occitanie pour l'instant) pour les gens qui n'ont pas de voiture (moi par exemple).

## Comment ça marche
1. On demande au serveur (plan.lio-occitanie.fr)[plan.lio-occitanie.fr] de calculer un itinéraire en train depuis notre gare de départ à un jour donné [^1]
2. On fait (1) mais pour toutes les gares de la région. On a alors les trajets les plus lents et les plus rapides.
3. Pour chaque gare, on va interroger (IGNrando)[https://ignrando.fr/] et on scrape les données des circuits obtenus [^2]
4. On filtre les résultats, si on se souvient bien, nous n'avons **pas** de voiture ; or, lla plupart des randonnées se trouvent à plusieurs km d'une ville ! Il faut donc trier celles qui sont par exemple à moins de 2km à pied de la gare.
5. On fait une requête météo pour chopper la météo parce que c'est sympa de savoir
6. On arrange le tout dans `stdout` (j'aimerais bien faire un export HTML aussi) avec les liens IGNrando pour aller chopper le topo, et des infos comme la distance, l'heure à laquelle la randonnée peut commencer, la météo etc...

[^1]: On fixe l'heure à 6h du matin pour être sur d'avoir le premier train, en supposant qu'une rando commence le plus tôt possible sauf si vous êtes bizarres
[^2]: Pas trouvé quoi que ce soit sur le scraping dans les CGU (https://ignrando.fr/fr/cgu) donc on va faire genre ça passe
