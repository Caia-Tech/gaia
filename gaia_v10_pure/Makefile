# GAIA V10 Pure - True simplicity
CC = gcc
CFLAGS = -Wall -O2 -g
LDFLAGS = -lm

all: gaia_pure

gaia_pure: gaia_pure.c
	$(CC) $(CFLAGS) gaia_pure.c -o gaia_pure $(LDFLAGS)
	@echo "Built GAIA V10 Pure"
	@wc -l gaia_pure.c

clean:
	rm -f gaia_pure

run: gaia_pure
	./gaia_pure

.PHONY: all clean run