#!/bin/bash

# FROM
FROM_NAME="Toulouse Matabiau, Toulouse"
FROM_LAT="43.61121"
FROM_LON="1.45362"
# TO
TO_ID="STOPAREA|LIO:StopArea:OCE87613000"
TO_NAME="Cahors, Cahors"
TO_LAT="44.44928"
TO_LON=""

DEPARTURE_TIME="2025-03-06T06:00:00+01:00"  # 6 March 2025, 06:00 AM

# Perform the request
curl 'https://plan.lio-occitanie.fr/fr/itineraire' --compressed -X POST \
--resolve plan.lio-occitanie.fr:443:35.195.12.125 \
-H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0' \
-H 'Accept: text/html, */*; q=0.01' \
-H 'Accept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3' \
-H 'Accept-Encoding: gzip, deflate, br, zstd' \
-H 'Content-Type: application/x-www-form-urlencoded; charset=UTF-8' \
-H 'X-Requested-With: XMLHttpRequest' \
-H 'Origin: https://plan.lio-occitanie.fr' \
-H 'Connection: keep-alive' \
-H 'Referer: https://plan.lio-occitanie.fr/fr/' \
-H 'Cookie: PHPSESSID=52323bce601b0ecb9fc1a2477f224d8e' \
-H 'Sec-Fetch-Dest: empty' \
-H 'Sec-Fetch-Mode: cors' \
-H 'Sec-Fetch-Site: same-origin' \
-H 'Priority: u=0' \
-H 'TE: trailers' \
--data-urlencode "from[id]=$FROM_ID" \
--data-urlencode "from[value]=$FROM_NAME" \
--data-urlencode "from[latitude]=$FROM_LAT" \
--data-urlencode "from[longitude]=$FROM_LON" \
--data-urlencode "to[id]=$TO_ID" \
--data-urlencode "to[value]=$TO_NAME" \
--data-urlencode "to[latitude]=$TO_LAT" \
--data-urlencode "to[longitude]=$TO_LON" \
--data-urlencode "departureDateTime=$DEPARTURE_TIME" \
--data-urlencode "arrivalDateTime=" \
--data-urlencode "modes[]=TRAIN" \
--data-urlencode "walk[speed]=1" \
--data-urlencode "bike[speed]=" \
--data-urlencode "displayFacilities=true" \
--data-urlencode "useScholarLines=false" \
--data-urlencode "accessible=false" \
--data-urlencode "avoidDisruptions=false" \
--data-urlencode "criterion=FASTEST" \
--data-urlencode "currentUrl=https://plan.lio-occitanie.fr/fr/itineraire?fi=$FROM_ID&fv=$FROM_NAME&flat=$FROM_LAT&flon=$FROM_LON&ti=$TO_ID&tv=$TO_NAME&tlat=$TO_LAT&tlon=$TO_LON&dt=$DEPARTURE_TIME&ws=1&bs=&a=false&sl=false&ad=false&df=true&v=[]&c=FASTEST&m=train" \
--data-urlencode "widgetContext=false" \
--data-urlencode "layoutMode=TRANSPORT" \
-w "DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total}\n" \
-o lio.raw

perl -ne '
    print "$1\n" if /"content":"(.*)"}/
' lio.raw > lio.rawhtml

perl -CSD -pe '
    s/\\u([0-9a-fA-F]{4})/chr(hex($1))/ge;
    s!\\/!/!g;
    s/\\n/\n/g
' lio.rawhtml > lio.almosthtml

perl -0777 -ne 'if (/(.*?)<span class="is-hide".*<\/span>(.*)/s) { print $1, $2 }' lio.almosthtml > lio.html

rm lio.raw
rm lio.rawhtml
rm lio.almosthtml

#open lio.html

echo
echo
echo

perl -0777 -ne 'print "[$1]->[$2]\n" if /Heure de départ : (\d{2}:\d{2})\./ and /prévue : (\d{2}:\d{2})\./' lio.html

echo
