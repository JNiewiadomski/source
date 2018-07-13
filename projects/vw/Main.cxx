#include <stdio.h>

#include "Display.h"
#include "Physical.h"

constexpr int const HEIGHT { 9 };
constexpr int const HALF_HEIGHT { 5 };
constexpr int const WIDTH { 31 };
constexpr int const HALF_WIDTH { 16 };

int main()
{
    Display::Rectangle r1(Display::Point(10, 15), Display::Point(20,75));
    Display::Rectangle r2();

    putchar(static_cast<int>(Display::Drawing_Symbols::UL));

    return 0;
}
