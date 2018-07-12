#pragma once

namespace Display
{
    enum class Drawing_Symbols
    {
        UL=218,     // upper left
        UR=191,     // upper right
        LL=192,     // lower left
        LR=217,     // lower right
        XD=194,     // T-intersection down
        XU=193,     // T-intersection up
        XR=195,     // T-intersection right
        XL=180,     // T-intersection left
        XX=197,     // 4-way intersection
        H=196,      // horizontal line
        V=179,      // vertical line
        ULD=210,    // upper left double
        URD=187,    // upper right double
        LLD=200,    // lower left double
        LRD=188,    // lower right double
        XDD=203,    // T-intersection down double
        XUD=202,    // T-intersection up double
        XRD=204,    // T-intersection right double
        XLD=185,    // T-intersection left double
        XXD=206,    // 4-way intersection double
        HD=205,     // horizontal line double
        VD=186,     // vertical line double
        HATCH=176,  // hatching
        SPACE=255,  // alternate space
    };

    enum class Character_Attribute
    {
        Normal=0x07,
        Inverse=0x70,
        Intense=0x0F,
        Blink=0x87,
        Underline=0x01
    };

    struct Cell
    {
        char m_character;
        Character_Attribute m_attribute;
    };

    enum class Direction { Up, Down, Left, Right };

    typedef short RowColumnType;

    class Point
    {
    public:
        Point(RowColumnType const row, RowColumnType const column)
            :
            m_row(row), m_column(column)
        {
        }

        RowColumnType get_row() const { return m_row; }
        RowColumnType get_column() const { return m_column; }

    private:
        Point();

        RowColumnType m_row;
        RowColumnType m_column;
    };

    class Rectangle
    {
    public:
        Rectangle(Point const & upper_left, Point const & lower_right)
            :
            m_upper_left(upper_left), m_lower_right(lower_right)
        {
        }

        RowColumnType get_width() const
        {
            return m_lower_right.get_column() - m_upper_left.get_column() + 1;
        }

        RowColumnType get_height() const
        {
            return m_lower_right.get_row() - m_upper_left.get_row() + 1;
        }

    private:
        Rectangle();

        Point m_upper_left;
        Point m_lower_right;
    };
}
