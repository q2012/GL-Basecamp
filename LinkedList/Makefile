TARGET=test_list
DEPS=list
DEPS:=$(addsuffix .o, $(DEPS))

CC=gcc
CFLAGS=-O2 -Wall -Wextra -Wpedantic

.PHONY: help clean all

all:	clean | $(TARGET)	## clean & build all

help:				## display this message
	@echo Available options:
	@echo Run with CFLAGS+=DDEBUG to do assert check for every operation tested
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

$(TARGET): $(DEPS)		## build target exec
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:	## tidy build directory
	@echo Tidying things up...
	-rm -f $(TARGET)
	-rm -f $(DEPS)
	-rm -f *.o $(TARGET)