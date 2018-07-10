#include <stdio.h>

#include "Display.h"

constexpr int const HEIGHT { 9 };
constexpr int const HALF_HEIGHT { 5 };
constexpr int const WIDTH { 31 };
constexpr int const HALF_WIDTH { 16 };

int main()
{
    putchar(static_cast<int>(Display::Drawing_Symbols::UL));

    return 0;
}
