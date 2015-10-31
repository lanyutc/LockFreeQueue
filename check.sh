#/bin/bash

./test > log

sleep 2

grep producer log > producer

sleep 2

sort -t ':' -n -k2 producer > producer_s

sleep 2

uniq -d producer_s

sleep 2

grep con log > consumer

sleep 2

sort -t ':' -n -k2 consumer > consumer_s

sleep 2

uniq -d consumer_s

sleep 2

rm -f producer producer_s consumer consumer_s
