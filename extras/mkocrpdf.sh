#!/bin/sh
# See COPYING file for copyright and license details.
#
# Makes a PDF with embedded text extracted by tesseract
# Requires imagemagick, pdftk, hocr2pdf and tesseract 3
#
# Note: Unfortunately tesseract works much better if one first
#       makes the image to be OCRed significantly larger. This
#       script does that, then reduces the results back down
#       to create a reasonable size PDF.

test $# -ne 1 && echo "Usage: $0 bookdir" && exit 1
cd "$1" || exit 1

echo 'tessedit_create_hocr 1' > hocr

for i in `ls`
do
	echo "$i"

	# create a much bigger version of the page image
	width=`identify "$i" | awk '{print $3}' | sed 's/x.*//'`
	bigwidth=`expr $width \* 4`
	convert "$i" -geometry ${bigwidth}x "$i.big.tif"

	# scan the page image
	tesseract "$i.big.tif" "$i.big.tif" hocr 2>&1 | sed '/Tesseract Open Source OCR Engine/d'

	# this reduces all bbox information to match the original image size
	sedrule=`cat "$i.big.tif.html" \
	         | sed -e 's/</\n/g' \
	         | sed -e '/bbox/!d' -e 's/.*bbox//g' -e 's/".*//g' -e "s/'.*//g" \
	         | awk '{ printf("s/bbox %d %d %d %d/bbox",$1,$2,$3,$4);
	                  for(a=1;a<5;a++) printf(" %d", $a/4);
	                  printf("/g\n")}'`
	sed -e 's/\.big\.tif//g' -e "$sedrule" < "$i.big.tif.html" > "$i.html"

	# combine the image and hocr into a pdf page
	# Note: hocr2pdf has a habit of segfaulting, so fall back to convert.
	#       also, it tends to complain about the quality of tesseract's
	#       hocr output, which it's best to silence here.
	hocr2pdf -i "$i" -o "$i.pdf" < "$i.html" >/dev/null 2>&1 || convert "$i" "$i.pdf"

	# remove working files
	rm -f "$i.big.tif" "$i.big.tif.html" "$i.html"
done

# cat the pdf pages together
pdftk *pdf cat output book.pdf
rm -f [0-9]*pdf hocr

echo "$1/book.pdf"
