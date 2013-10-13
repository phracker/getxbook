# See COPYING file for copyright and license details.
VERSION = 1.1
RELDATE = 2013-10-13

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L \
         -DVERSION=\"$(VERSION)\"

W32TCLKIT = tclkit-8.5.9-win32.upx.exe

# glibc dynamic
CC = cc
LDFLAGS = 

# musl static
#CC = musl-gcc
#LDFLAGS = -static #-s

# mingw
#CC = i686-w64-mingw32-gcc
#AR = i686-w64-mingw32-ar
#CFLAGS = -ansi -Wall -DVERSION=\"$(VERSION)\" -DWINVER=0x0501
#LDFLAGS = -lws2_32

LD = $(CC)
