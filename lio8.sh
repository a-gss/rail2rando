curl 'https://plan.lio-occitanie.fr/fr/itineraire' --compressed -X POST \
-H 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0' \
-H 'Accept: text/html, */*; q=0.01' \
-H 'Accept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3' \
-H 'Accept-Encoding: gzip, deflate, br, zstd' \
-H 'Content-Type: application/x-www-form-urlencoded; charset=UTF-8' \
-H 'X-Requested-With: XMLHttpRequest' \
-H 'Origin: https://plan.lio-occitanie.fr' \
-H 'Connection: keep-alive' \
-H 'Referer: https://plan.lio-occitanie.fr/fr/itineraire?fi=STOPAREA|LIO:StopArea:OCE87611004&fv=Toulouse%20Matabiau,%20Toulouse&flat=43.61121&flon=1.45362&ti=STOPAREA|LIO:StopArea:OCE87613000&tv=Cahors,%20Cahors&tlat=44.44928&tlon=1.43333&dt=2025-01-25T06:00:00+01:00&ws=1&bs=&a=false&sl=false&ad=false&df=true&v=[]&c=FASTEST&m=train' \
-H 'Cookie: PHPSESSID=f9695003d70db9a21c2270b92e740d72' \
-H 'Sec-Fetch-Dest: empty' \
-H 'Sec-Fetch-Mode: cors' \
-H 'Sec-Fetch-Site: same-origin' \
-H 'Priority: u=0' \
-H 'TE: trailers' --data-raw 'from%5Bid%5D=STOPAREA%7CLIO%3AStopArea%3AOCE87611004&from%5Bvalue%5D=Toulouse+Matabiau%2C+Toulouse&from%5Blatitude%5D=43.61121&from%5Blongitude%5D=1.45362&to%5Bid%5D=STOPAREA%7CLIO%3AStopArea%3AOCE87613000&to%5Bvalue%5D=Cahors%2C+Cahors&to%5Blatitude%5D=44.44928&to%5Blongitude%5D=1.43333&departureDateTime=2025-01-25T06%3A00%3A00%2B01%3A00&arrivalDateTime=&modes%5B%5D=TRAIN&walk%5Bspeed%5D=1&bike%5Bspeed%5D=&displayFacilities=true&useScholarLines=false&accessible=false&avoidDisruptions=false&criterion=FASTEST&currentUrl=https%253A%252F%252Fplan.lio-occitanie.fr%252Ffr%252Fitineraire%253Ffi%253DSTOPAREA%257CLIO%253AStopArea%253AOCE87611004%2526fv%253DToulouse%252520Matabiau%252C%252520Toulouse%2526flat%253D43.61121%2526flon%253D1.45362%2526ti%253DSTOPAREA%257CLIO%253AStopArea%253AOCE87613000%2526tv%253DCahors%252C%252520Cahors%2526tlat%253D44.44928%2526tlon%253D1.43333%2526dt%253D2025-01-25T06%253A00%253A00%252B01%253A00%2526ws%253D1%2526bs%253D%2526a%253Dfalse%2526sl%253Dfalse%2526ad%253Dfalse%2526df%253Dtrue%2526v%253D%255B%255D%2526c%253DFASTEST%2526m%253Dtrain&widgetContext=false&layoutMode=TRANSPORT' \
-w "DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total}\n" \
-o lio.raw

sed -e 's/\\u0022/"/g' \
    -e 's/\\u0027/'"'"'/g' \
    -e 's/\\u0026/&/g' \
    -e 's/\\u0026#039;/'"'"'/g' \
    -e 's/\\u003C/</g' \
    -e 's/\\u003E/>/g' \
    -e 's/\\n/\n/g' \
    -e 's#\\/##g' \
    -e 's/\\u00e9/é/g' \
    -e 's/\\u00e8/è/g' \
    -e 's/\\u00ea/ê/g' \
    -e 's/\\u00e0/à/g' \
    -e 's/\\u00e2/â/g' \
    -e 's/\\u00f9/ù/g' \
    -e 's/\\u00e7/ç/g' \
    -e 's/\\u00ee/î/g' \
    -e 's/\\u00f4/ô/g' \
    -e 's/\\u00c9/É/g' \
    -e 's/&#039;/'"'"'/g' \
    lio.raw > lio.clean

#jq -r '.response.content' lio.clean > lio.html
