# This makefile automatically builds all the tests
CATEGORIES = \
	strings \
	vectors \
	either \
	futil \
	linking

all: $(CATEGORIES)

define RULE
$(1):
	make -C $(1) -B
endef

$(foreach category, $(CATEGORIES), $(eval $(call RULE, $(category)) ))
