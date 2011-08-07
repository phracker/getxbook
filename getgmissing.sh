#!/bin/sh
# See COPYING file for copyright and license details.
#
# This gets any pages listed as available that have not been
# downloaded. Note that at present this is not too useful, as
# an IP block will be imposed after the first x pages each run,
# just for checking availaility.

test $# -ne 1 && echo "usage: $0 bookid" && exit

getgbook -p $1 2>/dev/null | while read i
do
	code=`echo $i|awk '{print $1}'`
	num=`echo $i|awk '{print $2}'`
	test -n "$num" && num=`printf '%04d' $num` || num=$code
	test -f $num.png || echo $code | getgbook $1
done
