#pragma once

#include <type_traits> // std::enable_if, std::underlying_type

///////////////////////////////////////////////////////////////////////////////////////////////////
// Typesafe Bitmasks
//
// This header file provides template definitions for overloaded typesafe bitwise operators that
// simplifies the usage of scoped enumeration bitmasks.
//
// Consider the following example:
//
//      #include <Typesafe_Bitmasks.h>
//
//      // Forward declaration
//      namespace Util { namespace JSON { enum class Diff_Flags : unsigned char; } }
//
//      // Class template specializations must occur at global scope
//      template<> struct Enable_Typesafe_Bitmask_Operators<Util::JSON::Diff_Flags>
//      {
//          static bool const enable { true };
//      };
//
//      namespace Util { namespace JSON {
//          enum class Diff_Flags : unsigned char
//          {
//              Additions = 0x01,
//              Modifications = 0x02,
//              Deletions = 0x04,
//
//              All = 0xFF
//          };
//      } }
//
//      Util::JSON::Diff_Flags const diff_flags
//      {
//              Util::JSON::Diff_Flags::Additions | Util::JSON::Diff_Flags::Modifications
//      };
//
//      bool const additions_enabled
//      {
//          is_bitmask_set<Diff_Flags>(diff_flags, Diff_Flags::Additions)
//      };
//
//      bool const modifications_enabled
//      {
//          is_bitmask_set<Diff_Flags>(diff_flags, Diff_Flags::Modifications)
//      };
//
//      bool const deletions_enabled
//      {
//          is_bitmask_set<Diff_Flags>(diff_flags, Diff_Flags::Deletions)
//      };
//
//      if (additions_enabled || modifications_enabled)
//      {
//          // . . .
//      }
//
//      if (deletions_enabled)
//      {
//          // . . .
//      }
//
// Using enum classes as type-safe bitmasks
// http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
//
// Using Enum Classes as Bitfields
// https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
//
// SFINAE is a term coined by David Vandevoorde and Nicolai Josuttis in their book
// "C++ Templates: The Complete Guide". It stands for "Substitution Failure is Not an Error", and
// highlights a feature of expanding function templates during overload resolution: if substituting
// the template parameters into the function declaration fails to produce a valid declaration then
// the template is removed from the overload set without causing a compilation error.
//
// This is a key feature used to constrain templates, both within the C++ Standard Library, and in
// many other libraries and application code. It is such a key feature that the C++ Standard
// Library even provides a library facility to assist with its use: std::enable_if.
//
// We can therefore use it to constrain our template to just those scoped enumerations that we want
// to act as bitmasks.

/**
 * Unless a specialization is defined Enable_Typesafe_Bitmask_Operators is disabled.
 */
template<typename Enum>
struct Enable_Typesafe_Bitmask_Operators
{
    static bool const enable { false };
};

/**
 * Bitwise operator |
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs | rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type
operator | (Enum const & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

/**
 * Bitwise operator &
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs & rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type
operator & (Enum const & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

/**
 * Bitwise operator ^
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs ^ rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type
operator ^ (Enum const & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    return static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

/**
 * Bitwise operator ~
 *
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (~rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type
operator ~ (Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    return static_cast<Enum>(~ static_cast<T>(rhs));
}

/**
 * Bitwise operator |=
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs |= rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type &
operator |= (Enum & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    lhs = static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
    return lhs;
}

/**
 * Bitwise operator &=
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs &= rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type &
operator &= (Enum & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    lhs = static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
    return lhs;
}

/**
 * Bitwise operator ^=
 *
 * @param lhs Left-hand-side of the operator.
 * @param rhs Right-hand-side of the operator.
 *
 * @return The result of (lhs ^= rhs).
 */
template<typename Enum>
typename std::enable_if<Enable_Typesafe_Bitmask_Operators<Enum>::enable, Enum>::type &
operator ^= (Enum & lhs, Enum const & rhs)
{
    typedef typename std::underlying_type<Enum>::type T;
    lhs = static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
    return lhs;
}

/**
 * Tests whether the bitmask bits are set in flags.
 *
 * @param flags Flags to test.
 * @param bitmask Bits that are of interest.
 *
 * @return True if bitmask bits are set in flags.
 */
template<typename Enum>
bool is_bitmask_set(Enum const & flags, Enum const & bitmask)
{
    return (flags & bitmask) == bitmask;
}

