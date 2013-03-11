#!/bin/bash

make loader
if [ ! -x "./loader" ]; then
	echo "FAIL: make";
	exit;
fi

./loader > /dev/null &
LOADER_PID=$!

rm blob.bin 2> /dev/null
OUTPUT=`perl ./blob.pl; cat blob.bin | nc localhost 12345`

if [ "$OUTPUT" == "Hell" ]; then
	echo -n "PASS with: ";
	echo $OUTPUT;
else
	echo -n "FAIL with: ";
	echo $OUTPUT;
fi

kill $LOADER_PID
