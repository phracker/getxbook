#!/bin/sh
#
# Makes a PDF with embedded text extracted by tesseract
#
# Requires imagemagick, pdftk, hocr2pdf and tesseract
#
# Also requires this tesseract configuration:
# echo 'tessedit_create_hocr 1' > /usr/local/share/tessdata/configs/hocr

for i in `ls *png`
do
	a=`basename $i .png`
	echo processing $a

	tesseract $i $a hocr 2>/dev/null

	# hocr2pdf has a habit of segfaulting, so fall back to convert
	hocr2pdf -i $i -o $a.pdf < $a.html || convert $i $a.pdf
	rm -f $a.html
done

pdftk *pdf cat output book.pdf
