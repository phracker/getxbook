#!/bin/sh
#
# Makes a pdf with embedded text as extracted by cuneiform
#
# Requires imagemagick, pdftk, hocr2pdf and cuneiform

for i in `ls *png`
do
	a=`basename $i .png`
	echo processing $a

	convert $i $a.bmp
	cuneiform -f hocr -o $a.html $a.bmp
	rm -f $a.bmp

	# hocr2pdf has a habit of segfaulting, so fall back to convert
	hocr2pdf -i $i -o $a.pdf < $a.html || convert $i $a.pdf
	rm -f $a.html
done

pdftk *pdf cat output book.pdf
