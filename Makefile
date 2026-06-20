CC = clang
CFLAGS = -Wall -Wextra -Wno-override-init -Wno-initializer-overrides -fPIC

ifeq ($(d),1)
	CFLAGS += -ggdb -O0 -fsanitize=memory
else
	CFLAGS += -O2
endif

BUILD = build
SRC = src
LIB_NAME = libctl
VERSION = 1.0.0
BUILD_SCRIPT = build.c

OBJECT = $(BUILD)/lib/$(LIB_NAME).o
DYN_LIB = $(BUILD)/lib/$(LIB_NAME).so.$(VERSION)
STC_LIB = $(BUILD)/lib/$(LIB_NAME).a

HEADERS = \
	either.h \
	array.h \
	sv.h \
	str.h \
	futil.h \
	buic.h \
	json.h \
	basic.h
HEADERS_SRC = $(addprefix $(SRC)/, $(HEADERS))
TRIMMED = $(addprefix $(BUILD)/include/, $(HEADERS))

all: $(DYN_LIB) $(STC_LIB) $(BUILD) $(TRIMMED)
.PHONY: clean

$(BUILD):
	mkdir -p $(BUILD)
	mkdir -p $(BUILD)/include
	mkdir -p $(BUILD)/lib

$(OBJECT): $(BUILD_SCRIPT) $(HEADERS_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -I./src -c $< -o $@

$(STC_LIB): $(OBJECT) | $(BUILD)
	ar rcs $@ $<
	strip $@

$(DYN_LIB): $(OBJECT) | $(BUILD)
	$(CC) $(CFLAGS) -shared $< -o $@
	strip $@
	ln -s $(LIB_NAME).so.$(VERSION) $(BUILD)/lib/$(LIB_NAME).so || true

define TRIM_RULE
$(BUILD)/include/$(1): $(SRC)/$(1) | $(BUILD)/include
	awk ' \
		/IMPLEMENTATION BEGIN/ {skip=1} \
		/IMPLEMENTATION END/ {skip=0; next} \
		!skip \
	' $$< > $$@
endef

$(foreach h,$(HEADERS),$(eval $(call TRIM_RULE,$(h))))

clean:
	rm -rf $(BUILD)
