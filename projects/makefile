###################################################################################################
# Reference articles
#
# Auto-Dependency Generation
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
# Managing Recipe Echoing
# http://make.mad-scientist.net/managing-recipe-echoing/

###################################################################################################
# Defines

APPNAME ?= main

###################################################################################################
# Makefile parameters

BUILD_TAG ?= debug

ifeq "$(BUILD_TAG)" "debug"
    USING_OPTIMIZATION = 0
else ifeq "$(BUILD_TAG)" "release"
    USING_OPTIMIZATION = 1
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
VERBOSE =
endif

###################################################################################################
# Directory structure

PROJECT_DIR := .
SOURCE_DIR := $(PROJECT_DIR)/source
INCLUDE_DIR := $(PROJECT_DIR)/include
OUTPUT_DIR := $(PROJECT_DIR)/output/$(BUILD_TAG)
DEPEND_DIR := $(OUTPUT_DIR)/depend

###################################################################################################
# Tools

CPP := g++

###################################################################################################
# Flags

CPPFLAGS := -I$(INCLUDE_DIR) -std=c++14
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPEND_DIR)/$*.Td

###################################################################################################
# Commands

COMPILE.cpp = $(CPP) $(DEPFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPEND_DIR)/$*.Td $(DEPEND_DIR)/$*.d && touch $@

###################################################################################################
# Rules

#_DEPS =
#DEPS = $(patsubst %,$(INCLUDE_DIR)/%,$(_DEPS))

#_OBJ = main.o
#OBJS = $(patsubst %,$(OUTPUT_DIR)/%,$(_OBJ))

# Only use implicit rules we create
.SUFFIXES:

%.o : %.cpp
%.o : %.cpp $(DEPEND_DIR)/%.d
	@echo COMPILE $@
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPEND_DIR)/%.d: ;
.PRECIOUS: $(DEPEND_DIR)/%.d

#SRCS = $(SOURCE_DIR)/main.cpp

#include $(wildcard $(patsubst %,$(DEPEND_DIR)/%.d,$(basename $(SRCS))))

#%.d: %.c
#	@set -e; rm -f $@; \
#		$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
#		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#		rm -f $@.$$$$
#
#$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(DEPS)
#	@echo COMPILE $@
#	$(CC) -c -o $@ $< $(CPPFLAGS)
#
$(APPNAME): $(OBJS)
	@echo LINK $@
	$(CC) -o $@ $^ $(CPPFLAGS)

all: $(APPNAME)

sources = $(SOURCE_DIR)/main.cpp

ifneq ($(MAKECMDGOALS),clean)
#include $(sources:.cpp=.d)
include $(wildcard $(patsubst %,$(DEPEND_DIR)/%.d,$(basename $(sources))))
endif

$(OBJS): | $(OUTPUT_DIR) $(DEPEND_DIR)

$(OUTPUT_DIR) $(DEPEND_DIR):
	@echo CREATE $@
	mkdir -p $@

.PHONY: clean

clean:
	@echo DELETING $(OUTPUT_DIR)
	rm --force --recursive $(OUTPUT_DIR)/

$(VERBOSE).SILENT:
