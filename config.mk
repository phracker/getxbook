# See COPYING file for copyright and license details.
VERSION = 0.9
RELDATE = 2012-04-27

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L \
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
