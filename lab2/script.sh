#!/bin/sh

directory="/shared"
lock_file="$directory/.lock"


mkdir -p "$directory"
touch "$lock_file"


ID=$(uuidgen)
seq=1

exec 200>"$lock_file"

echo "Container started with ID: $ID"

while true; do

    flock -x 200
    

    filename=""
    

    for i in $(seq -w 1 999); do
        if [ ! -e "$directory/$i" ]; then
            filename="$i"
            break
        fi
    done
    

    if [ -n "$filename" ]; then 
        echo "identifier: $ID" > "$directory/$filename"
        echo "sequence: $seq" >> "$directory/$filename"
        echo "$(date '+%Y-%m-%d %H:%M:%S'): Created file $filename with ID $ID and sequence $seq"
    fi
    

    flock -u 200
    

    
    if [ -n "$filename" ] && [ -f "$directory/$filename" ]; then

        sleep 1
        
        rm "$directory/$filename"
        echo "$(date '+%Y-%m-%d %H:%M:%S'): Deleted file $filename"

        seq=$(expr $seq + 1)
    else

        echo "$(date '+%Y-%m-%d %H:%M:%S'): No free slots available"
    fi
    

    sleep 1
done