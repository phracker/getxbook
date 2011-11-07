#!/bin/sh
# See COPYING file for copyright and license details.
#
# Makes a PDF with embedded text extracted by tesseract
# Requires imagemagick, pdftk, hocr2pdf and tesseract 3

echo 'tessedit_create_hocr 1' > hocr

for i in `ls *png`
do
	a=`basename $i .png`
	echo processing $a

	# unfortunately tesseract seems to work much better with a
	# resized larger image
	convert $i -geometry 1000x $a.big.png
	tesseract $a.big.png $a hocr 2>/dev/null

	# hocr2pdf has a habit of segfaulting, so fall back to convert
	hocr2pdf -i $a.big.png -o $a.pdf < $a.html || convert $a.big.png $a.pdf
	rm -f $a.html $a.big.png
done

pdftk *pdf cat output book.pdf
rm -f [0-9]*pdf hocr
