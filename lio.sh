#!/bin/bash

usage() {
    echo "Usage : $0 [options...] <origine> <destination>"
    echo
    echo "Arguments :"
    echo " <origine>                 Nom de la gare de départ (insensible à la casse)"
    echo " <destination>             Nom de la gare d'arrivée (insensible à la casse)"
    echo
    echo "Options :"
    echo " -b, --bus                 Ajoute les bus dans le calcul d'itinéraire (le calcul sera plus long mais plus précis)"
    echo " -d, --date <yyyy-mm-dd>   Précise la date de départ (défaut: aujourd'hui)"
    echo " -h, --heure <hh:mm>       Précise l'heure de départ (défaut: 00:00)"
    echo "     --help                Affiche ce texte"
    echo " -l, --lio                 Affiche le lien LIO du trajet"
    echo
    echo "Exemples:"
    echo " $0 Matabiau Figeac"
    echo " $0 -d 2025-09-25 cahors saint-gaudens"
    echo " $0 -h 09:45 -b cahors saint-agne"
    echo " $0 --bus --date 2025-04-16 --heure 09:45 Matabiau Figeac"
    exit 1
}

# Valeurs par défaut
DATE=$(date +%F)  # Aujourd'hui
TIME="00:00"     # Minuit
BUS_OPTION=""
LIOLINK=false

# Lecture des options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -d|--date)
            if [[ -n "$2" && "$2" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
                DATE="$2"
                shift 2  # Passe à l'argument suivant
            else
                echo -e "\e[31mErreur: Format de date invalide. Utilisez <yyyy-mm-dd>.\e[0m"
                exit 1
            fi ;;

        -h|--heure)
            if [[ -n "$2" && "$2" =~ ^[0-9]{2}:[0-9]{2}$ ]]; then
                TIME="$2"
                shift 2  # Passe à l'argument suivant
            else
                echo -e "\e[31mErreur: Format d'horaire invalide. Utilisez <hh:mm>.\e[0m"
                exit 1
            fi ;;

        -b|--bus)
            BUS="BUS"  # Active l'option bus
            shift ;;  # Passe à l'argument suivant

        -l|--lio)
            LIOLINK=true
            shift ;;

        --help)
            usage ;;

        --) shift; break ;;  # Fin des options

        -*)
            echo -e "\e[31mOption invalide : \e[33m$1\e[0m"
            exit 1 ;;

        *)
            break ;;  # Arrête dès qu'un argument non-optionnel est rencontré
    esac
done

# Vérifie qu'il reste bien deux arguments : origine et destination
if [ $# -ne 2 ]; then
    echo -e "\e[31mErreur: Vous devez spécifier une origine et une destination.\e[0m"
    usage
fi

# Récupère les arguments
FROM_NAME="$1"
TO_NAME="$2"

# Télécharge la liste des gares d'occitanie
if [ ! -f gares.csv ]; then
    echo -ne "\e[33mTéléchargement de la liste des gares d'occitanie dans gares.csv...\e[0m"
    curl -s -o gares.csv "https://data.laregion.fr/api/explore/v2.1/catalog/datasets/localisation-des-gares-et-haltes-ferroviaires-doccitanie/exports/csv?lang=fr&timezone=Europe%2FBerlin&use_labels=true&delimiter=%3B" && echo "OK"
fi

# Récupere les coordonnées long/lat de la gare de départ
FROM_DATA=$(cat gares.csv | grep -io "${FROM_NAME}".*)
if [[ -z "$FROM_DATA" ]]; then
    echo -e "\e[31mErreur: Aucune gare "${FROM_NAME}" n'a été trouvée.\e[0m"
    exit 1
fi
FROM_LON=$(echo "$FROM_DATA" | perl -ne 'print "$1\n" if /\[(\d+\.\d+), (\d+\.\d+)\]/')
FROM_LAT=$(echo "$FROM_DATA" | perl -ne 'print "$2\n" if /\[(\d+\.\d+), (\d+\.\d+)\]/')

# Récupere l'UIC de la gare d'arrivée
UIC=$(perl -ne "if (/\Q${TO_NAME}\E;.*?;(\d*);/i) { print \$1 }" gares.csv) # /i case insensitive
TO_ID="STOPAREA|LIO:StopArea:OCE$UIC"
currentURL="https://plan.lio-occitanie.fr/fr/itineraire?fi=$FROM_ID&fv=$FROM_NAME&flat=$FROM_LAT&flon=$FROM_LON&ti=$TO_ID&tv=$TO_NAME&tlat=&tlon=&dt=${DATE}T$TIME&ws=1&bs=&a=false&sl=false&ad=false&df=true&v=&c=FASTEST&m=train&m=walk&m=${BUS}"

if [[ -z "$UIC" ]]; then
    echo -e "\e[31mErreur: Aucune gare "${TO_NAME}" n'a été trouvée.\e[0m"
    exit 1
fi

# j'aime trop curl c'est trop fort
curl 'https://plan.lio-occitanie.fr/fr/itineraire' -X POST \
--resolve plan.lio-occitanie.fr:443:35.195.12.125 \
-H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0' \
-H 'X-Requested-With: XMLHttpRequest' \
-H 'Connection: keep-alive' \
-H 'TE: trailers' \
--data-urlencode "from[id]=$FROM_ID" \
--data-urlencode "from[value]=$FROM_NAME" \
--data-urlencode "from[latitude]=$FROM_LAT" \
--data-urlencode "from[longitude]=$FROM_LON" \
--data-urlencode "to[id]=$TO_ID" \
--data-urlencode "to[value]=$TO_NAME" \
--data-urlencode "departureDateTime=${DATE}T$TIME" \
--data-urlencode "modes[]=TRAIN" \
--data-urlencode "modes[]=WALK" \
--data-urlencode "modes[]=${BUS}" \
--data-urlencode "walk[speed]=1" \
--data-urlencode "bike[speed]=" \
--data-urlencode "displayFacilities=true" \
--data-urlencode "useScholarLines=false" \
--data-urlencode "accessible=false" \
--data-urlencode "avoidDisruptions=false" \
--data-urlencode "criterion=FASTEST" \
--data-urlencode "currentUrl=$currentURL" \
--data-urlencode "widgetContext=false" \
--data-urlencode "layoutMode=TRANSPORT" \
-o lio.raw \
-s #-i -w "DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total}\n"

perl -ne 'print "$1\n" if /"content":"(.*)"}/' lio.raw > lio.rawhtml # on recupere que la partie "content" de la réponse
perl -CSD -pe 's/\\u([0-9a-fA-F]{4})/chr(hex($1))/ge; s!\\/!/!g; s/\\n//g' lio.rawhtml > lio.html # conversion unicode \u0261 en caratères spéciaux

ERROR_MSG=$(grep -oE "(Le calcul d'itinéraire n'a pas trouvé de résultat pour votre demande|Aucun itinéraire disponible pour le moment)" lio.html)
if [ -n "$ERROR_MSG" ]; then
    echo -e "\e[31m$ERROR_MSG\e[0m"
else
    read START END < <(perl -0777 -ne 'print "$1 $2\n" if /(\d{2}:\d{2})\..*?(\d{2}:\d{2})/' lio.html)
    START_SEC=$(date -d "$START" +%s)
    END_SEC=$(date -d "$END" +%s)
    DIFF_SEC=$((END_SEC - START_SEC)) # on calcule la difference en secondes
    HOURS=$((DIFF_SEC / 3600)) # on reconverti en heure/minute
    MINUTES=$(((DIFF_SEC % 3600) / 60))
    if [ $HOURS == 0 ]; then
        echo "${FROM_NAME[@]^}->${TO_NAME[@]^} [$START]->[$END] (${MINUTES}min)"
    else
        echo "${FROM_NAME[@]^}->${TO_NAME[@]^} [$START]->[$END] (${HOURS}h${MINUTES}min)"
    fi
fi


if [ "$LIOLINK" = true ]; then
    echo
    echo -e "\e[36mLIO: $currentURL\e[0m"
fi

rm lio.raw
rm lio.rawhtml
rm lio.html

# Affichage des valeurs pour debug
# echo "Date : $DATE"
# echo "Heure : $TIME"
# echo "Origine : $FROM_NAME"
# echo "Destination : $TO_NAME"
