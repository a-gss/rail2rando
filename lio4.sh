curl -v -w "DNS: %{time_namelookup} Connect: %{time_connect} PreTransfer: %{time_pretransfer} StartTransfer: %{time_starttransfer} Total: %{time_total}\n" \
 -k -X POST "https://35.195.12.125/fr/itineraire" \
-H "Host: plan.lio-occitanie.fr" \
-H "Content-Type: application/json" \
-H "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36" \
-H "Accept: application/json, text/javascript, */*; q=0.01" \
-H "Origin: https://plan.lio-occitanie.fr" \
-H "Referer: https://plan.lio-occitanie.fr/" \
-H "Connection: keep-alive" \
-d '{
  "from": {
    "id": "STOPAREA|LIO:StopArea:OCE87611004",
    "value": "Toulouse+Matabiau,+Toulouse",
    "latitude": "43.61121",
    "longitude": "1.45362"
  },
  "to": {
    "id": "STOPAREA|LIO:StopArea:OCE87613000",
    "value": "Cahors,+Cahors",
    "latitude": "44.44928",
    "longitude": "1.43333"
  },
  "departureDateTime": "2025-01-25T06:00:00+01:00",
  "arrivalDateTime": "",
  "modes": ["TRAIN", "METRO", "WALK"],
  "bike[speed]": "",
  "displayFacilities": true,
  "useSchoolarLines": false,
  "accessible": false,
  "avoidDisruptions": false,
  "criterion": "FASTEST",
  "widgetContext": false,
  "layoutMode": "TRANSPORT"
}'
