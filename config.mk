# See COPYING file for copyright, license and warranty details.
VERSION = prealpha

# paths
PREFIX = /usr/local

# flags
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L -DVERSION=\"$(VERSION)\"
LDFLAGS = -static #-s

# compiler and linker
CC = musl-gcc
LD = ld
