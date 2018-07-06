#!/usr/bin/env bash

OUTPUT_FILE='main'

SOURCE_FILES='main.cpp'

COMPILER_OPTIONS=

# Verbose output
#COMPILER_OPTIONS+=' -v'

# Use pipes rather than temporary files for communication between the various stages of
# compilation.
#
COMPILER_OPTIONS+=' -pipe'

# Turn on C++14 functionality
COMPILER_OPTIONS+=' -std=c++14'

# Disable generation of information about every class with virtual functions for use by the C ++
# runtime type identification features (dynamic_cast and typeid).
#
COMPILER_OPTIONS+=' -fno-rtti'

# Warn when a class seems unusable because all the constructors or destructors in that class are
# private, and it has neither friends nor public static member functions.
#
COMPILER_OPTIONS+=' -Wctor-dtor-privacy'

# Warn about violations of the following style guidelines from Scott Meyers' Effective C ++ book.
COMPILER_OPTIONS+=' -Weffc++'

# Warn also about the use of an uncasted "NULL" as sentinel.
COMPILER_OPTIONS+=' -Wstrict-null-sentinel'

# Warn if an old-style (C-style) cast to a non-void type is used within a C ++ program.
COMPILER_OPTIONS+=' -Wold-style-cast'

# Warn when a function declaration hides virtual functions from a base class.
COMPILER_OPTIONS+=' -Woverloaded-virtual'

# Warn when overload resolution chooses a promotion from unsigned or enumerated type to a signed
# type, over a conversion to an unsigned type of the same size.
#
COMPILER_OPTIONS+=' -Wsign-promo'

# Make all warnings into errors.
COMPILER_OPTIONS+=' -Werror'

# Issue all the warnings demanded by strict ISO C and ISO C ++ ; reject all programs that use
# forbidden extensions, and some other programs that do not follow ISO C and ISO C ++.
#
COMPILER_OPTIONS+=' -pedantic'

# This enables all the warnings about constructions that some users consider questionable, and that
# are easy to avoid (or modify to prevent the warning), even in conjunction with macros.
#
COMPILER_OPTIONS+=' -Wall'

# This enables some extra warning flags that are not enabled by -Wall.
COMPILER_OPTIONS+=' -Wextra'

# Enable -Wformat plus format checks not included in -Wformat. Currently equivalent to -Wformat
# -Wformat-nonliteral -Wformat-security -Wformat-y2k.
#
COMPILER_OPTIONS+=' -Wformat=2'

# Warn about uninitialized variables which are initialized with themselves. Note this option can
# only be used with the -Wuninitialized option.
#
COMPILER_OPTIONS+=' -Winit-self'

# Warn if a user-supplied include directory does not exist.
COMPILER_OPTIONS+=' -Wmissing-include-dirs'

# Warn whenever a "switch" statement has an index of enumerated type and lacks a "case" for one or
# more of the named codes of that enumeration.
#
COMPILER_OPTIONS+=' -Wswitch-enum'

# All the above -Wunused options combined.
COMPILER_OPTIONS+=' -Wunused'

# Warn when a #pragma directive is encountered which is not understood by GCC.
COMPILER_OPTIONS+=' -Wunknown-pragmas'

# Warn if floating point values are used in equality comparisons.
COMPILER_OPTIONS+=' -Wfloat-equal'

# Warn if an undefined identifier is evaluated in an #if directive.
COMPILER_OPTIONS+=' -Wundef'

# Warn whenever a local variable shadows another local variable, parameter or global variable or
# whenever a built-in function is shadowed.
#
COMPILER_OPTIONS+=' -Wshadow'

# Warn whenever a pointer is cast so as to remove a type qualifier from the target type.
COMPILER_OPTIONS+=' -Wcast-qual'

# Warn whenever a pointer is cast such that the required alignment of the target is increased.
COMPILER_OPTIONS+=' -Wcast-align'

# Warn for implicit conversions that may alter a value.
COMPILER_OPTIONS+=' -Wconversion'

# Warn for implicit conversions that may change the sign of an integer value, like assigning a
# signed integer expression to an unsigned integer variable.
#
COMPILER_OPTIONS+=' -Wsign-conversion'

# Warn about suspicious uses of logical operators in expressions.
COMPILER_OPTIONS+=' -Wlogical-op'

# Warn if any functions that return structures or unions are defined or called.
COMPILER_OPTIONS+=' -Waggregate-return'

# Warn if a global function is defined without a previous declaration.
COMPILER_OPTIONS+=' -Wmissing-declarations'

# Warn if a structure is given the packed attribute, but the packed attribute has no effect on the
# layout or size of the structure.
#
COMPILER_OPTIONS+=' -Wpacked'

# Warn if padding is included in a structure, either to align an element of the structure or to
# align the whole structure.
#
COMPILER_OPTIONS+=' -Wpadded'

# Warn if anything is declared more than once in the same scope, even in cases where multiple
# declaration is valid and changes nothing.
#
COMPILER_OPTIONS+=' -Wredundant-decls'

# Warn if a function can not be inlined and it was declared as inline.
COMPILER_OPTIONS+=' -Winline'

# Warn if variable length array is used in the code.
COMPILER_OPTIONS+=' -Wvla'

# Warn if a requested optimization pass is disabled.
COMPILER_OPTIONS+=' -Wdisabled-optimization'

# Produce debugging information in the operating system's native format (stabs, COFF , XCOFF , or
# DWARF 2).
#
COMPILER_OPTIONS+=' -g'

# Produce debugging information for use by GDB.
COMPILER_OPTIONS+=' -ggdb'

# Generate extra code to write profile information suitable for the analysis program prof.
COMPILER_OPTIONS+=' -p'

# Add code so that program flow arcs are instrumented.
COMPILER_OPTIONS+=' -fprofile-arcs'

# This option is used to compile and link code instrumented for coverage analysis.
COMPILER_OPTIONS+=' --coverage'

#Produce a notes file that the gcov code-coverage utility can use to show program coverage.
COMPILER_OPTIONS+=' -ftest-coverage'

# Optimize yet more.
COMPILER_OPTIONS+=' -O3'

g++ ${COMPILER_OPTIONS} -o ${OUTPUT_FILE} ${SOURCE_FILES}
