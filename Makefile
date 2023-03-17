CC=gcc
DEST=.build
EXENAME=main
SRC=$(shell find . -name "*.c")
OBJ=$(SRC:.c=.o)
LIBS=-lcurl -lm
CFLAGS=-Wall -Wextra -pedantic -g
LFLAGS=-fsanitize=address -fno-omit-frame-pointer

renew: clean main

main: createbuildfolder prog

createbuildfolder:
	@TESTBF=$(wildcard .build)
ifeq (,$(TESTBF))
	mkdir $(DEST)
endif

prog: $(OBJ)
	$(CC) -o $(EXENAME) $(addprefix $(DEST)/, $(notdir $^)) $(CFLAGS) $(LFLAGS) $(LIBS)

%.o: %.c
	$(CC) -o $(addprefix $(DEST)/, $(notdir $@)) -c $< $(CFLAGS) $(LFLAGS) $(LIBS)

clean:
	rm -rf $(EXENAME) $(DEST)

.PHONY: main
