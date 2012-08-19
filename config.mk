# See COPYING file for copyright and license details.
VERSION = 1.0
RELDATE = 2012-08-19

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L \
         -DVERSION=\"$(VERSION)\"

W32TCLKIT = tclkit-8.5.9-win32.upx.exe

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
