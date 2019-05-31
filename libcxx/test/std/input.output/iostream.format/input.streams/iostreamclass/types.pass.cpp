//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <istream>

// template <class charT, class traits = char_traits<charT> >
// class basic_iostream :
//     public basic_istream<charT,traits>,
//     public basic_ostream<charT,traits>
// {
// public:
//     // types:
//     typedef charT                          char_type;
//     typedef traits                         traits_type;
//     typedef typename traits_type::int_type int_type;
//     typedef typename traits_type::pos_type pos_type;
//     typedef typename traits_type::off_type off_type;

#include <istream>
#include <type_traits>

#include "test_macros.h"

int main(int, char**)
{
    static_assert((std::is_base_of<std::basic_istream<char>, std::basic_iostream<char> >::value), "");
    static_assert((std::is_base_of<std::basic_ostream<char>, std::basic_iostream<char> >::value), "");
    static_assert((std::is_same<std::basic_iostream<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::basic_iostream<char>::traits_type, std::char_traits<char> >::value), "");
    static_assert((std::is_same<std::basic_iostream<char>::int_type, std::char_traits<char>::int_type>::value), "");
    static_assert((std::is_same<std::basic_iostream<char>::pos_type, std::char_traits<char>::pos_type>::value), "");
    static_assert((std::is_same<std::basic_iostream<char>::off_type, std::char_traits<char>::off_type>::value), "");

  return 0;
}
