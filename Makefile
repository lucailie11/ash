CC      = gcc
CFLAGS  = -Wall -Wextra -MMD -MP

SRCS := $(shell find src -name '*.c')
OBJS := $(SRCS:src/%.c=obj/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean build run

all: build run

-include $(DEPS)

bin/ash: $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/ash

obj/%.o: src/%.c | obj
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bin obj:
	mkdir -p $@

clean:
	rm -rf bin obj

build: bin/ash

run:
	./bin/ash
