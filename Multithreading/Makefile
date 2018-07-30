TARGET=pthread
DEPS=libs/list
LIBS=

CC=gcc
CFLAGS=-O2 -Wall -Wextra -Wpedantic #-Werror
LIBFLAGS:=-lm -pthread

ifneq ($(LIBS),)
LIBFLAGS+=$(shell pkg-config --cflags --libs $(LIBS) | sed -e 's/^[[:space:]]*//')
endif
DEPS:=$(addsuffix .o, $(DEPS))

.PHONY: help all clean

help:				## display this message
	@echo Available options:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

all: clean | $(TARGET)		## clean & build all

$(TARGET): $(DEPS)		## build target executable
	$(CC) $(CFLAGS) $(LIBFLAGS) $(addsuffix .c, $(TARGET)) -c
	$(CC) $(CFLAGS) $(LIBFLAGS) $(DEPS) $(addsuffix .o, $(TARGET)) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $<

clean:				## tidy build directory
	@echo Cleaning up...
	-rm -f $(DEPS) $(TARGET) $(addsuffix .o, $(TARGET))
