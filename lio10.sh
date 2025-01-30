#!/bin/bash

# TODO
# Gestion des erreurs :
#    - si la date est dans le passé (wtf)
#    - si on met pas d'heure, default à 00:01 pour avoir le premier train

if [ $# -ne 3 ]; then
    echo "Usage: $0 <yyyy-mm-dd> <hh:mm> <destination>"
    exit 1
fi

# Télécharge la liste des gares d'occitanie
if [ ! -f gares.csv ]; then
    echo -ne "\e[33mTéléchargement de la liste des gares d'occitanie dans gares.csv...\e[0m"
    curl -s -o gares.csv "https://data.laregion.fr/api/explore/v2.1/catalog/datasets/localisation-des-gares-et-haltes-ferroviaires-doccitanie/exports/csv?lang=fr&timezone=Europe%2FBerlin&use_labels=true&delimiter=%3B" && echo "OK"
fi

DATE_REGEX="^[0-9]{4}-[0-9]{2}-[0-9]{2}$"
TIME_REGEX="^[0-9]{2}:[0-9]{2}$"

DATE=$1
TIME=$2

# Check if the date matches the format yyyy-mm-dd
if [[ ! $DATE =~ $DATE_REGEX ]]; then
    echo "Error: Invalid date format. Please use yyyy-mm-dd."
    exit 1
fi

# Check if the time matches the format hh:mm
if [[ ! $TIME =~ $TIME_REGEX ]]; then
    echo "Error: Invalid time format. Please use hh:mm."
    exit 1
fi

# quelques variables
FROM_NAME="Toulouse"
FROM_LAT="43.61121"
FROM_LON="1.45362"

TO_NAME="$3"
UIC=$(perl -ne "if (/\Q$TO_NAME\E;.*?;(\d*);/i) { print \$1 }" gares.csv)
TO_ID="STOPAREA|LIO:StopArea:OCE$UIC"

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
--data-urlencode "to[latitude]=" \
--data-urlencode "to[longitude]=" \
--data-urlencode "departureDateTime=${DATE}T$TIME" \
--data-urlencode "arrivalDateTime=" \
--data-urlencode "modes[]=TRAIN" \
--data-urlencode "walk[speed]=1" \
--data-urlencode "bike[speed]=" \
--data-urlencode "displayFacilities=true" \
--data-urlencode "useScholarLines=false" \
--data-urlencode "accessible=false" \
--data-urlencode "avoidDisruptions=false" \
--data-urlencode "criterion=FASTEST" \
--data-urlencode "currentUrl=https://plan.lio-occitanie.fr/fr/itineraire?fi=$FROM_ID&fv=$FROM_NAME&flat=$FROM_LAT&flon=$FROM_LON&ti=$TO_ID&tv=$TO_NAME&tlat=&tlon=&dt=${DATE}T$TIME&ws=1&bs=&a=false&sl=false&ad=false&df=true&v=[]&c=FASTEST&m=train" \
--data-urlencode "widgetContext=false" \
--data-urlencode "layoutMode=TRANSPORT" \
-o lio.raw \
-s # -w "DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total}\n"

perl -ne '
    print "$1\n" if /"content":"(.*)"}/
' lio.raw > lio.rawhtml

perl -CSD -pe '
    s/\\u([0-9a-fA-F]{4})/chr(hex($1))/ge;
    s!\\/!/!g;
    s/\\n//g
' lio.rawhtml > lio.html

ERROR_MSG=$(grep -oP "(Le calcul d'itinéraire n'a pas trouvé de résultat pour votre demande|Aucun itinéraire disponible pour le moment)" lio.html)
if [ -n "$ERROR_MSG" ]; then
    echo "$ERROR_MSG."
else
    read START END HOURS MINUTES < <(perl -0777 -ne 'print "$1 $2 $3 $4\n" if /(\d{2}:\d{2})\..*?(\d{2}:\d{2}).*?(\d{1}).*?(\d{2})/' lio.html)
    echo "$FROM_NAME->$TO_NAME [$START]->[$END] (${HOURS}h${MINUTES})"
fi


# Aucun itinéraire disponible pour le moment

rm lio.raw
#rm lio.rawhtml
#rm lio.html
