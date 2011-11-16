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

echo 'tessedit_create_hocr 1' > hocr

for i in `ls *png`
do
	# create a much bigger version of the page image
	width=`identify "$i" | awk '{print $3}' | sed 's/x.*//'`
	bigwidth=`expr $width \* 4`
	convert "$i" -geometry ${bigwidth}x "$i.big.png"

	# scan the page image
	tesseract "$i.big.png" "$i.big.png" hocr 2>&1 | sed '/Tesseract Open Source OCR Engine/d'

	# this reduces all bbox information to match the original image size
	sedrule=`cat "$i.big.png.html" \
	         | sed -e 's/</\n/g' \
	         | sed -e '/bbox/!d' -e 's/.*bbox//g' -e 's/".*//g' -e "s/'.*//g" \
	         | awk '{ printf("s/bbox %d %d %d %d/bbox",$1,$2,$3,$4);
	                  for(i=1;i<5;i++) printf(" %d", $i/4);
	                  printf("/g\n")}'`
	sed -e 's/\.big\.png//g' -e "$sedrule" < "$i.big.png.html" > "$i.html"

	# combine the image and hocr into a pdf page
	# Note: hocr2pdf has a habit of segfaulting, so fall back to convert
	hocr2pdf -i "$i" -o "$i.pdf" < "$i.html" || convert "$i" "$i.pdf"

	# remove working files
	rm -f "$i.big.png" "$i.big.png.html" "$i.html"
done

# cat the pdf pages together
pdftk *pdf cat output book.pdf
rm -f [0-9]*pdf hocr
