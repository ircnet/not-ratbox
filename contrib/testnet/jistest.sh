#!/bin/sh
# Try to create JIS-encoded channel containing ',' to see if JIS support works as expected
# $Id$

if (echo "USER sd 0 sd :sd"; echo "NICK sd--"; printf 'JOIN #\033\$Btest,test2\033(B\n'; echo QUIT) | nc localhost 6607 | grep "test,test"; then
	echo "JP test succeeded"
else
	echo "JP test failed"
fi

