#! /bin/sh

if [ "x$1" = "x" -o "x$2" = "x" ]; then
 echo " Format: $0 <old-dir> <new-dir>"
fi

if [ ! -d "$1" ]; then
 echo " Error: $1 isn't a directory"
fi

if [ ! -d "$2" ]; then
 echo " Error: $2 isn't a directory"
fi

rm -f talker.patch

diff -ruN $1 $2 > talker.patch
