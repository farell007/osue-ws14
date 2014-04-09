#!/bin/sh

color[0]=b
color[1]=d
color[2]=g
color[3]=o
color[4]=r
color[5]=s
color[6]=v
color[7]=w

client_output_file="test_client_output"
fails_file="test_fails"
avgs_file="test_avgs"
port=4242

rm "$client_output_file"
for i in {1..100}
do
	colorcombo=${color[$RANDOM % 8]}${color[$RANDOM % 8]}${color[$RANDOM % 8]}${color[$RANDOM % 8]}${color[$RANDOM % 8]}

	./server "$port" "$colorcombo" > /dev/null &
	serverpid="$!"

	sleep 0.2

	clientoutput="`./client localhost "$port"`"

	if [ "$?" -eq 3 ]
	then
		# client has failed this combination
		echo "$colorcombo" >> "$fails_file"
	else
		echo "$colorcombo: $clientoutput" >> "$client_output_file"
	fi

	# client has exit; kill server just in case
	kill "$serverpid" 2>/dev/null
done


# calculate average (thanks to (pi))
awk '{ SUM += $2 } END { print SUM/NR }' "$client_output_file"
