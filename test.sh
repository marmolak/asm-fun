#!/bin/bash

FILES_STATE=`find ./`;

rm -f shellcode-test.bin 2> /dev/null
make shellcode-test
if [ ! -s "./shellcode-test.bin" ]; then
	echo "FAIL: make shellcode-test";
	exit;
fi

rm -f loader 2> /dev/null
make loader 2> /dev/null
if [ ! -x "./loader" ]; then
	echo "FAIL: make loader";
	exit;
fi

./loader > /dev/null &
sleep 1;
LOADER_PID=$!
kill -0 $LOADER_PID 2> /dev/null; 
RET=$?
if [ $RET == 1 ]; then
	echo "FAIL run loader";
	exit;
fi

rm -f blob.bin 2> /dev/null
OUTPUT=`cat shellcode-test.bin | nc localhost 12345`

if [ "$OUTPUT" == "Hell" ]; then
	echo -n "PASS with: ";
else
	echo -n "FAIL with: ";
fi
echo $OUTPUT;

# make clean is better way
rm -f shellcode-test.bin 2> /dev/null
rm -f loader 2> /dev/null
kill $LOADER_PID

FILES_STATE_NEW=`find ./`;

diff <(echo $FILES_STATE) <(echo $FILES_STATE_NEW)
