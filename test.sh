#!/bin/bash

make loader
if [ ! -x "./loader" ]; then
	echo "FAIL: make";
	exit;
fi

./loader > /dev/null &
LOADER_PID=$!

OUTPUT=`perl ./blob.pl; cat blob.bin | nc localhost 12345`

if [ "$OUTPUT" == "Hell" ]; then
	echo "PASS";
else
	echo "FAIL";
fi

kill $LOADER_PID
