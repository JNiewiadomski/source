# Turn on C++14 functionality
CXX_GLOBAL_FLAGS += -std=c++14

# Use pipes rather than temporary files for communication between the various stages of
# compilation.
#
CXX_GLOBAL_FLAGS += -pipe

# Disable generation of information about every class with virtual functions for use by the C ++
# runtime type identification features (dynamic_cast and typeid).
#
CXX_GLOBAL_FLAGS += -fno-rtti

