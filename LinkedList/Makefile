TARGET=test_list
DEPS=list
DEPS:=$(addsuffix .o, $(DEPS))

CC=gcc
CFLAGS=-O2 -Wall -Wextra -Wpedantic

.PHONY: clean all

all:	$(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(addsuffix .c, $(TARGET)) $(DEPS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Tidying things up...
	-rm -f $(TARGET)
	-rm -f $(DEPS)
	-rm -f *.o $(TARGET)