#!/bin/sh
# See COPYING file for copyright and license details.
#
# Makes a DjVu
# Requires imagemagick and djvulibre

test $# -ne 1 && echo "Usage: $0 bookdir" && exit 1
cd "$1" || exit 1

for i in `ls`
do
	echo "$i"

	convert "$i" "$i.ppm"
	c44 "$i.ppm" "$i.djvu"

	rm -f "$i.ppm"
done

djvm -c book.djvu *.djvu

rm -f [0-9]*djvu

echo "$1/book.djvu"
