# Warn when a class seems unusable because all the constructors or destructors in that class are
# private, and it has neither friends nor public static member functions.
#
CXX_WARNING_FLAGS += -Wctor-dtor-privacy

# Warn about violations of the following style guidelines from Scott MeyersEffective C ++ book.
CXX_WARNING_FLAGS += -Weffc++

# Warn also about the use of an uncasted "NULLas sentinel.
CXX_WARNING_FLAGS += -Wstrict-null-sentinel

# Warn if an old-style (C-style) cast to a non-void type is used within a C ++ program.
CXX_WARNING_FLAGS += -Wold-style-cast

# Warn when a function declaration hides virtual functions from a base class.
CXX_WARNING_FLAGS += -Woverloaded-virtual

# Warn when overload resolution chooses a promotion from unsigned or enumerated type to a signed
# type, over a conversion to an unsigned type of the same size.
#
CXX_WARNING_FLAGS += -Wsign-promo

# Make all warnings into errors.
CXX_WARNING_FLAGS += -Werror

# Issue all the warnings demanded by strict ISO C and ISO C ++ ; reject all programs that use
# forbidden extensions, and some other programs that do not follow ISO C and ISO C ++.
#
CXX_WARNING_FLAGS += -pedantic

# This enables all the warnings about constructions that some users consider questionable, and that
# are easy to avoid (or modify to prevent the warning), even in conjunction with macros.
#
CXX_WARNING_FLAGS += -Wall

# This enables some extra warning flags that are not enabled by -Wall.
CXX_WARNING_FLAGS += -Wextra

# Enable -Wformat plus format checks not included in -Wformat. Currently equivalent to -Wformat
# -Wformat-nonliteral -Wformat-security -Wformat-y2k.
#
CXX_WARNING_FLAGS += -Wformat=2

# Warn about uninitialized variables which are initialized with themselves. Note this option can
# only be used with the -Wuninitialized option.
#
CXX_WARNING_FLAGS += -Winit-self

# Warn if a user-supplied include directory does not exist.
CXX_WARNING_FLAGS += -Wmissing-include-dirs

# Warn whenever a "switchstatement has an index of enumerated type and lacks a "casefor one or
# more of the named codes of that enumeration.
#
CXX_WARNING_FLAGS += -Wswitch-enum

# All the above -Wunused options combined.
CXX_WARNING_FLAGS += -Wunused

# Warn when a #pragma directive is encountered which is not understood by GCC.
CXX_WARNING_FLAGS += -Wunknown-pragmas

# Warn if floating point values are used in equality comparisons.
CXX_WARNING_FLAGS += -Wfloat-equal

# Warn if an undefined identifier is evaluated in an #if directive.
CXX_WARNING_FLAGS += -Wundef

# Warn whenever a local variable shadows another local variable, parameter or global variable or
# whenever a built-in function is shadowed.
#
CXX_WARNING_FLAGS += -Wshadow

# Warn whenever a pointer is cast so as to remove a type qualifier from the target type.
CXX_WARNING_FLAGS += -Wcast-qual

# Warn whenever a pointer is cast such that the required alignment of the target is increased.
CXX_WARNING_FLAGS += -Wcast-align

# Warn for implicit conversions that may alter a value.
CXX_WARNING_FLAGS += -Wconversion

# Warn for implicit conversions that may change the sign of an integer value, like assigning a
# signed integer expression to an unsigned integer variable.
#
CXX_WARNING_FLAGS += -Wsign-conversion

# Warn about suspicious uses of logical operators in expressions.
CXX_WARNING_FLAGS += -Wlogical-op

# Warn if a global function is defined without a previous declaration.
CXX_WARNING_FLAGS += -Wmissing-declarations

# Warn if a structure is given the packed attribute, but the packed attribute has no effect on the
# layout or size of the structure.
#
CXX_WARNING_FLAGS += -Wpacked

# Warn if padding is included in a structure, either to align an element of the structure or to
# align the whole structure.
#
CXX_WARNING_FLAGS += -Wpadded

# Warn if anything is declared more than once in the same scope, even in cases where multiple
# declaration is valid and changes nothing.
#
CXX_WARNING_FLAGS += -Wredundant-decls

# Warn if a function can not be inlined and it was declared as inline.
CXX_WARNING_FLAGS += -Winline

# Warn if variable length array is used in the code.
CXX_WARNING_FLAGS += -Wvla

# Warn if a requested optimization pass is disabled.
CXX_WARNING_FLAGS += -Wdisabled-optimization

