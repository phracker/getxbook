# See COPYING file for copyright, license and warranty details.
VERSION = prealpha

# paths
PREFIX = /usr/local

CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"

# musl static
CC = musl-gcc
LDFLAGS = -static #-s

# glibc dynamic
#CC = cc
#LDFLAGS = 

LD = ld
