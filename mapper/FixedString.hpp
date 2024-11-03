#pragma once
#include<string_view>

using std::operator""sv;
using std::string_view;

template<int N>
struct FixedString
{
  char ptr[N];
  int len{0};

  constexpr FixedString(const char* s, int sz)
    : len(std::min(sz, N))
  {
    for (int i = 0; i < sz; ++i) {
      ptr[i] = s[i];
    }
    std::fill(&sz[ptr], &N[ptr], '\0');
  }

  constexpr operator string_view() const
  { return std::string_view(ptr, std::size_t(len)); }
};

constexpr FixedString<256> operator
""_cstr(const char* s, std::size_t n)
{ return FixedString<256>(s, n); }
