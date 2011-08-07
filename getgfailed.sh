#!/bin/sh
# See COPYING file for copyright and license details.
#
# Tries to download each page listed in a fail log (from a
# previous run of getgbook -a bookid > faillog)

test $# -ne 2 && echo "usage: $0 bookid faillog" && exit

sort < $2 | sort | shuf | head -n 5 | while read i
do
	code=`echo $i|awk '{print $1}'`
	echo $code | getgbook $1
done
