#!/bin/sh
# See COPYING file for copyright and license details.
#
# Makes a DjVu with embedded text extracted by tesseract
# Requires imagemagick, djvulibre and tesseract 3
#
# Note that this doesn't use bounding box info, so that text
# reflows much better.

test $# -ne 1 && echo "Usage: $0 bookdir" && exit 1
cd "$1" || exit 1

for i in `ls`
do
	echo "$i"

	# create djvu compressed version
	convert "$i" "$i.ppm"
	c44 "$i.ppm" "$i.djvu"

	# create a much bigger version of the page image, for better
	# tesseract accuracy
	width=`identify "$i" |awk '{print $3}'|awk -F x '{print $1}'`
	height=`identify "$i" |awk '{print $3}'|awk -F x '{print $2}'`
	bigwidth=`expr $width \* 4`
	convert "$i" -geometry ${bigwidth}x "$i.tif"

	tesseract "$i.tif" "$i" 2>&1 | sed '/Tesseract Open Source OCR Engine/d'

	# convert tesseract output into djvused input
	(printf "(page 0 0 $width $height \"";sed 's/"/\\"/g;'"s/\'/\\\'/g" < "$i.txt";printf \"")\n") > "$i.djvutxt"
	djvused "$i.djvu" -e "select 1; set-txt $i.djvutxt" -s

	rm -f "$i.ppm" "$i.tif" "$i.txt" "$i.djvutxt"
done

djvm -c book.djvu *.djvu

rm -f [0-9]*djvu

echo "$1/book.djvu"
