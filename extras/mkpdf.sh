#!/bin/sh
# See COPYING file for copyright and license details.
#
# Requires imagemagick

test $# -ne 1 && echo "Usage: $0 bookdir" && exit 1

cd "$1" && convert * book.pdf
echo "$1/book.pdf"
