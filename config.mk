# See COPYING file for copyright and license details.
VERSION = 0.5
RELDATE = 2011-09-29

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L \
         -DVERSION=\"$(VERSION)\"

W32TCLKIT = tclkit-8.5.9-win32.upx.exe
MACTCLKIT = tclkit-8.5b1-darwin-univ-aqua

# musl static
#CC = musl-gcc
#LDFLAGS = -static #-s

# glibc dynamic
CC = cc
LDFLAGS = 

# mingw
#CC = gcc
#CFLAGS = -ansi -Wall -DVERSION=\"$(VERSION)\" -DWINVER=0x0501
#LDFLAGS = -lws2_32

LD = $(CC)
