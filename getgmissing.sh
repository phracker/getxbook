#!/bin/sh
# See COPYING file for copyright and license details.

test $# -ne 1 && echo "usage: $0 bookid" && exit

getgbook -p $1 2>/dev/null | while read i
do
	code=`echo $i|awk '{print $1}'`
	num=`echo $i|awk '{print $2}'`
	test -n "$num" && num=`printf '%04d' $num` || num=$code
	test -f $num.png || echo $code | getgbook $1
done
