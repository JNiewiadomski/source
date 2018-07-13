###################################################################################################
# Reference articles
#
# GNU make
# https://www.gnu.org/software/make/manual/make.html
#
# Practical Makefiles, by example
# http://nuclear.mutantstargoat.com/articles/make/
#
# Auto-Dependency Generation
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
# Managing Recipe Echoing
# http://make.mad-scientist.net/managing-recipe-echoing/

###################################################################################################
# Makefile parameters

BUILD_TAG ?= debug

ifeq "$(BUILD_TAG)" "debug"
    USING_OPTIMIZATION := 0
else ifeq "$(BUILD_TAG)" "release"
    USING_OPTIMIZATION := 1
else
    $(error Unknown BUILD_TAG: $(BUILD_TAG))
endif

# Most of the time we want our build to not echo the compilation commands, but sometimes we do.
# The user may specify MAKE_VERBOSE=1 on the command line to turn on command echoing.
#
MAKE_VERBOSE ?= 0

ifneq "$(MAKE_VERBOSE)" "0"
VERBOSE := 1
else
VERBOSE :=
endif

###################################################################################################
# Directory structure

PROJECT_DIR := .
OUTPUT_DIR := $(PROJECT_DIR)/output/$(BUILD_TAG)

###################################################################################################
# Defines

SOURCES := $(wildcard *.cxx)
OBJECTS := $(addprefix $(OUTPUT_DIR)/,$(SOURCES:.cxx=.o))
DEPENDS := $(OBJECTS:.o=.d)
OUTPUTS := $(sort $(dir $(OBJECTS)))

###################################################################################################
# Flags

CXX_FLAGS = -std=c++14 -Wall -Wextra
DEPEND_FLAGS = -MT $@ -MMD -MP -MF $(OUTPUT_DIR)/$*.Td

###################################################################################################
# Commands

COMPILE.cxx = $(CXX) $(DEPEND_FLAGS) $(CXX_FLAGS) -c
POSTCOMPILE = mv --force $(OUTPUT_DIR)/$*.Td $(OUTPUT_DIR)/$*.d && touch $@

vw: $(OBJECTS)
	@echo LINK $@
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(OUTPUT_DIR)/%.o: %.cxx | $(OUTPUT_DIR)
	@echo COMPILE $@
	@echo OBJECTS=$(OBJECTS)
	@echo DEPENDS=$(DEPENDS)
	@echo OUTPUTS=$(OUTPUTS)
	$(COMPILE.cxx) -o $@ $<
	$(POSTCOMPILE)

$(OUTPUT_DIR)/%.d: ;
.PRECIOUS: $(OUTPUT_DIR)/%.d

ifneq ($(MAKECMDGOALS),clean)
include $(wildcard $(patsubst %,$(OUTPUT_DIR)/%.d,$(basename $(SOURCES))))
endif

$(OUTPUT_DIR):
	@echo CREATE $@
	mkdir --parents $@

.PHONY: clean
clean:
	@echo DELETING $(OUTPUT_DIR)
	rm --force --recursive $(OUTPUT_DIR)/

$(VERBOSE).SILENT:
