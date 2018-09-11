# Generate extra code to write profile information suitable for the analysis program prof.
CXX_PROFILE_FLAGS += -p

# Add code so that program flow arcs are instrumented.
CXX_PROFILE_FLAGS += -fprofile-arcs

# This option is used to compile and link code instrumented for coverage analysis.
CXX_PROFILE_FLAGS += --coverage

#Produce a notes file that the gcov code-coverage utility can use to show program coverage.
CXX_PROFILE_FLAGS += -ftest-coverage

