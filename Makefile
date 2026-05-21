CC = clang
CFLAGS = -Wall -Wextra -ggdb

BUILD = build
TARGET = libcutils.so
HEADERS = \
	either.h \
	vector.h \
	sv.h \
	str.h \
	futil.h
SRC = build.c

TRIMMED = $(addprefix $(BUILD)/, $(HEADERS))

all: $(BUILD)/$(TARGET) $(BUILD) $(TRIMMED)
.PHONY: clean

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/$(TARGET): $(SRC) $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) $(SRC) -o $(BUILD)/$(TARGET) -shared -fPIC

define TRIM_RULE
$(BUILD)/$(1): $(1) | $(BUILD)
	awk ' \
		BEGIN { \
			print "/*"; \
			print "  This file was generated automatically"; \
			print "  It doesnt have implementation"; \
			print "  Compatible with >=C89 or >=C++98"; \
			print "*/"; \
			print ""; \
		} \
		/IMPLEMENTATION BEGIN/ {skip=1} \
		/IMPLEMENTATION END/ {skip=0; next} \
		!skip \
	' $$< > $$@
endef

$(foreach h,$(HEADERS),$(eval $(call TRIM_RULE,$(h))))

clean:
	rm -rf $(BUILD)
