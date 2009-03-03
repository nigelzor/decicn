#!/bin/sh
echo converting $1 to $1.png
PMAP=`mktemp`
./decicn pmap "$1" > "$PMAP"
./decicn mask "$1" | convert - -negate - | composite -compose CopyOpacity - "$PMAP" "$1".png
rm $PMAP
