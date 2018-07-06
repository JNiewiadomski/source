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

# Any commands we want to follow the verbosity rules should have $(VERBOSE) in front of them, with
# no space separation. @ will be appended if we're not in verbose mode.
#
ifneq "$(MAKE_VERBOSE)" "1"
VERBOSE=@
else
VERBOSE=
endif

###################################################################################################
# Directory structure

PROJECT_DIR = .
SOURCE_DIR = $(PROJECT_DIR)/source
INCLUDE_DIR = $(PROJECT_DIR)/include
OUTPUT_DIR = $(PROJECT_DIR)/output/$(BUILD_TAG)

###################################################################################################
# Definitions

CC = g++
CPPFLAGS = -I$(INCLUDE_DIR) -std=c++14

LDIR = lib

LIBS = -lm

_DEPS =
DEPS = $(patsubst %,$(INCLUDE_DIR)/%,$(_DEPS))

_OBJ = main.o
OBJS = $(patsubst %,$(OUTPUT_DIR)/%,$(_OBJ))

# Only use implicit rules we create
#
.SUFFIXES:

%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

$(OUTPUT_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(DEPS)
	@echo COMPILE $@
	$(VERBOSE)$(CC) -c -o $@ $< $(CPPFLAGS)

$(APPNAME): $(OBJS)
	@echo LINK $@
	$(VERBOSE)$(CC) -o $@ $^ $(CPPFLAGS)

all: $(APPNAME)

sources = $(SOURCE_DIR)/main.cpp

ifneq ($(MAKECMDGOALS),clean)
include $(sources:.cpp=.d)
endif

$(OBJS): | $(OUTPUT_DIR)

$(OUTPUT_DIR):
	@echo CREATE $@
	$(VERBOSE)mkdir -p $(OUTPUT_DIR)

.PHONY: clean

clean:
	@echo DELETING $(OUTPUT_DIR)
	$(VERBOSE)rm --force --recursive $(OUTPUT_DIR)/
