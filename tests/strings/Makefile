CC = gcc
CFLAGS = -Wall -Wextra -Wno-override-init -ggdb -DSTR_IMPLEMENTATION -DSV_IMPLEMENTATION -I../../

STR_H = ../../str.h
SV_H = ../../sv.h
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
