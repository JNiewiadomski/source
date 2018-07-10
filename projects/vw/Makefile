SOURCES = Main.cxx

BUILD_TAG ?= debug

ifeq "$(BUILD_TAG)" "debug"
    USING_OPTIMIZATION = 0
else ifeq "$(BUILD_TAG)" "release"
    USING_OPTIMIZATION = 1
else
    $(error Unknown BUILD_TAG: $(BUILD_TAG))
endif

PROJECT_DIR := .
OUTPUT_DIR := $(PROJECT_DIR)/output/$(BUILD_TAG)

#CXX_FLAGS = -std=c++14
DEPEND_FLAGS = -MT $@ -MMD -MP -MF $*.Td

COMPILE.cxx = $(CXX) $(DEPEND_FLAGS) $(CXX_FLAGS) -c
POSTCOMPILE = mv -f $*.Td $*.d && touch $@

vw: $(OUTPUT_DIR)/Main.o
	@echo LINK $@
	$(CXX) -o $@ $^

$(OUTPUT_DIR)/Main.o: $(SOURCES) | $(OUTPUT_DIR)
	@echo COMPILE $@
	$(COMPILE.cxx) -o $@ $^
	$(POSTCOMPILE)

$(OUTPUT_DIR)/%.d: ;
.PRECIOUS: $(OUTPUT_DIR)/%.d

include $(wildcard $(patsubst %,$(OUTPUT_DIR)/%.d,$(basename $(SOURCES))))

$(OUTPUT_DIR):
	@echo CREATE $@
	mkdir -p $@
