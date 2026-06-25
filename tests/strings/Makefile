CC = gcc
CFLAGS = -Wall -Wextra -Wno-override-init -ggdb -DCTL_IMPLEMENTATION -I../../ -lm

CTL_H = ../../ctl.h
BUILD = build

TARGETS = lifecycle \
          trims \
          appends \
          chars \
          views \
          equals \
          empty \
          chop \
          join

.PHONY: all clean $(TARGETS)
all: $(BUILD) $(addprefix $(BUILD)/, $(TARGETS))

$(TARGETS): %: $(BUILD)/%

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%: %.c $(CTL_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD)
