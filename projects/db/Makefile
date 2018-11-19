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

OBJECTS_DIR := o/$(BUILD_TAG)

###################################################################################################
# Include

-include mk/*.mk

###################################################################################################
# Defines

C.SOURCES := $(wildcard *.c)
CXX.SOURCES := $(wildcard *.cxx)
OBJECTS := $(addprefix $(OBJECTS_DIR)/,$(C.SOURCES:.c=.o) $(CXX.SOURCES:.cxx=.o))
DEPENDS := $(OBJECTS:.o=.d)

###################################################################################################
# Flags

CXX_FLAGS := $(CXX_GLOBAL_FLAGS) $(CXX_WARNING_FLAGS)
DEPEND_FLAGS = -MT $@ -MMD -MP -MF $(OBJECTS_DIR)/$*.Td

ifeq "$(USING_OPTIMIZATION)" "1"
	CXX_FLAGS += $(CXX_OPTIMIZATION_FLAGS)
else
	CXX_FLAGS += $(CXX_DEBUG_FLAGS) $(CXX_PROFILE_FLAGS)
endif

###################################################################################################
# Rules

db : $(OBJECTS)
	@echo LINK $@
	$(CXX) $(CXX_FLAGS) -o $@ $^ -lpthread -ldl

$(OBJECTS_DIR)/%.o : %.c
	@echo COMPILE $@
	$(CC) $(C_FLAGS) $(DEPEND_FLAGS) -o $@ $< -c
	mv --force $(OBJECTS_DIR)/$*.Td $(OBJECTS_DIR)/$*.d && touch $@

$(OBJECTS_DIR)/%.o : %.cxx
	@echo COMPILE $@
	$(CXX) $(CXX_FLAGS) $(DEPEND_FLAGS) -o $@ $< -c
	mv --force $(OBJECTS_DIR)/$*.Td $(OBJECTS_DIR)/$*.d && touch $@

$(OBJECTS) : | $(OBJECTS_DIR)

$(OBJECTS_DIR):
	@echo CREATE $@
	mkdir --parents $@

$(OBJECTS_DIR)/%.d: ;
.PRECIOUS: $(OBJECTS_DIR)/%.d

ifneq ($(MAKECMDGOALS),clean)
include $(wildcard $(patsubst %,$(OBJECTS_DIR)/%.d,$(basename $(SOURCES))))
endif

.PHONY : clean
clean :
	@echo DELETING edit $(OBJECTS_DIR)
	rm --force --recursive edit $(OBJECTS_DIR)

$(VERBOSE).SILENT:

