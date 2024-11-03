#pragma once
#include<string_view>

using std::operator""sv;
using std::string_view;

template<size_t N>
struct FixedString
{
  char ptr[N]{};
  size_t len{0};

  constexpr FixedString(const char* s, int sz)
    : ptr(), len(sz)
  { std::copy(s, &len[s], ptr); }

  constexpr operator string_view()
  { return operator""sv(&(ptr[0]), len); }
};

constexpr FixedString<256> operator
""_cstr(const char* s, std::size_t n)
{ return FixedString<256>(s, n); }
