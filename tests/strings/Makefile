CC = gcc
CFLAGS = -Wall -Wextra -Wno-override-init -ggdb -DSTR_IMPLEMENTATION -DSV_IMPLEMENTATION -I../../src/

STR_H = ../../src/str.h
SV_H = ../../src/sv.h
BUILD = build

TARGETS = lifecycle \
          trims \
          appends \
          chars \
          views \
          equals \
          empty \
          chop 

.PHONY: all clean $(TARGETS)
all: $(BUILD) $(addprefix $(BUILD)/, $(TARGETS))

$(TARGETS): %: $(BUILD)/%

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%: %.c $(STR_H) $(SV_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD)
