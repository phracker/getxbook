# See COPYING file for copyright and license details.
VERSION = 0.3

# paths
PREFIX = /usr/local

CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200112L \
         -DVERSION=\"$(VERSION)\"

# musl static
#CC = musl-gcc
#LDFLAGS = -static #-s

# glibc dynamic
CC = cc
LDFLAGS = 

LD = $(CC)
