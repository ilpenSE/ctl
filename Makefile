CC = clang
CFLAGS = -Wall -Wextra -fPIC

ifeq ($(d),1)
	CFLAGS += -ggdb -O0 -fsanitize=memory
else
	CFLAGS += -O2
endif

BUILD = build
LIB_NAME = libcutils
VERSION = 1.0.0
OBJECT = $(BUILD)/lib/$(LIB_NAME).o
DYN_LIB = $(BUILD)/lib/$(LIB_NAME).so.$(VERSION)
STC_LIB = $(BUILD)/lib/$(LIB_NAME).a
HEADERS = \
	either.h \
	vector.h \
	sv.h \
	str.h \
	futil.h
SRC = build.c

TRIMMED = $(addprefix $(BUILD)/include/, $(HEADERS))

all: $(DYN_LIB) $(STC_LIB) $(BUILD) $(TRIMMED)
.PHONY: clean

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/include
	mkdir -p $(BUILD)/lib

$(OBJECT): $(SRC) $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(STC_LIB): $(OBJECT) | $(BUILD)
	ar rcs $@ $<
	strip $@

$(DYN_LIB): $(OBJECT) | $(BUILD)
	$(CC) $(CFLAGS) -shared $< -o $@
	strip $@
	ln -s $(LIB_NAME).so.$(VERSION) $(BUILD)/lib/$(LIB_NAME).so || true

define TRIM_RULE
$(BUILD)/include/$(1): $(1) | $(BUILD)/include
	awk ' \
		BEGIN { \
			print "/*"; \
			print "  This file was generated automatically"; \
			print "  It doesnt have implementation"; \
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
