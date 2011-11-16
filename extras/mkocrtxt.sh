#!/bin/sh
# See COPYING file for copyright and license details.
#
# Makes a text file with text extracted by tesseract
#
# Note: Unfortunately tesseract works much better if one first
#       makes the image to be OCRed significantly larger. This
#       script therefore temporarily creates a larger file to
#       feed to tesseract.

for i in `ls *png`
do
	# create a much bigger version of the page image
	width=`identify "$i" | awk '{print $3}' | sed 's/x.*//'`
	bigwidth=`expr $width \* 4`
	convert "$i" -geometry ${bigwidth}x "$i.big.png"

	# scan the page image
	tesseract "$i.big.png" "$i" 2>&1 | sed '/Tesseract Open Source OCR Engine/d'

	# combine the page text with the rest of the book
	cat "$i.txt" >> book.txt

	# remove working files
	rm -f "$i.big.png" "$i.txt"
done
