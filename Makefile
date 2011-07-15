SRC = getgbook.c
OBJ = $(SRC:.c=.o)

all: getgbook

.c.o:
	@echo CC $<
	@$(CC) -c -g $(CFLAGS) $<

$(OBJ): util.c

getgbook: $(OBJ)
	@echo LD $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f -- getgbook $(OBJ)

.PHONY: all clean
