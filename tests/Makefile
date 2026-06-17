# This makefile automatically builds all the tests
CATEGORIES = \
	strings \
	arrays \
	either \
	futil \
	json \
	linking

all: $(CATEGORIES)

define RULE
$(1):
	make -C $(1) -B
endef

$(foreach category, $(CATEGORIES), $(eval $(call RULE, $(category)) ))
