#!/bin/bash

OUTPUT=`make CFLAGS='-Wfatal-errors' iso 2>&1 | grep ': [a-zA-Z ]*error:' | head -n 1`
FILENAME=`echo "$OUTPUT" | grep '^[a-zA-Z/_0-9]*\.[ch]:[0-9]*' -o`
echo $OUTPUT
if [ -n "$OUTPUT" ]; then
	read -p "Open '$FILENAME' ? [y]" value
	case $value in
		[Nn]* ) exit;;
	esac
	geany $FILENAME
fi
