#pragma once

#include <type_traits>
#include <cstdint>
#include <concepts>
#include <array>
#include <string_view>
#include <algorithm>

const size_t OFFSET = 5;

template< auto T >
constexpr auto __f__()
{ return __PRETTY_FUNCTION__; }

namespace my
{ template<typename... tuple_T>
  struct tuple_tp {};

  template<typename T>
  concept tuple_concept = requires(T arg)
  { []<typename... tuple_T>(tuple_tp<tuple_T...>){}(arg); };

  template<auto _Vp>
  struct _tag
  { static constexpr auto tag_value = _Vp; };

  template<class _Tp, _Tp... Args>
  using VTuple = tuple_tp<_tag<Args>...>;

  template<tuple_concept T, tuple_concept U>
  struct Concat;

  template<typename... T, typename... U>
  struct Concat< tuple_tp<T...>, tuple_tp<U...>>
  { using _rng = tuple_tp<T..., U...>; };

  template< int _f, int _t>
  struct range
  { using _rng = typename Concat
    <typename range< _f, ((_f + _t) - int((_f + _t) % 2 != 0)) / 2>::_rng
      , typename range<((_f + _t) - int((_f + _t)%2 != 0)) / 2 + 1, _t>::_rng>::_rng;
    static constexpr int _sz = _t - _f + 1;
  };

  template<int K>
  struct range<K, K>
  { using _rng = tuple_tp<_tag<K>>;
    static constexpr int _sz = 1;
  };
}

constexpr bool has_val(const char* cnt)
{ size_t idx = 0;
  for (;cnt[idx] != '['; ++idx) {}
  return cnt[idx + OFFSET] != '(';
}


template<typename Enum, int MAXN = 512>
struct EnumeratorTraits
{
  static constexpr int _f()
  {
    return (std::same_as<std::underlying_type_t<Enum>, signed char>)? -std::min(128, MAXN)
      : (std::same_as<std::underlying_type_t<Enum>, int>)? -MAXN
      : 0;
  }

  static constexpr int _t()
  {
    return (std::same_as<std::underlying_type_t<Enum>, signed char>)? std::min(127, MAXN)
      : (std::same_as<std::underlying_type_t<Enum>, int>)? MAXN
      : (!std::same_as<std::underlying_type_t<Enum>, unsigned char>)? MAXN
      : std::min(255, MAXN);
  }

  static constexpr size_t size()
  { size_t ret_val = 0;
    constexpr auto& cnt = get_cnt<_f(), _t()>();
    for(size_t s_idx = 0; s_idx < cnt.size(); ++s_idx)
    { if(has_val( cnt[s_idx])) { ++ret_val; } }
    return ret_val;
  }

  static constexpr std::string_view nameAt(size_t index)
  { const char* name = nullptr;
    size_t at = 0;
    constexpr auto& cnt = get_cnt<_f(), _t()>();
    for (size_t i = 0; i < cnt.size(); ++i) {
      if (has_val( cnt[i]))
      { ++at; }

      if(index + 1 == at)
      { name = cnt[i];
        break;
      }
    }
    
    size_t start_idx = 0;
    size_t end_idx = 0;
    while (name[start_idx] != '[')
    { ++start_idx; }
    
    start_idx += 5;
    for (size_t i = 1; name[start_idx + i] != ']'; ++i) {
      if(name[start_idx + i] == ':')
      { start_idx += (i + 2);
        break;
      }
    }
    while (name[start_idx + end_idx] != ']')
    { ++end_idx; }

    return std::string_view(name + start_idx, end_idx);
  }

  static constexpr Enum at(size_t I)
  { size_t ret_val = 0;
    constexpr auto& cnt = get_cnt<_f(), _t()>();
    for (size_t i = 0; i < cnt.size(); ++i) {
      if(has_val( cnt[i]))
      { ++ret_val; }
      if (I + 1 == ret_val) {
        if (std::same_as<std::underlying_type_t<Enum>, signed char>) {
          return static_cast<Enum>((signed char)(i) - (signed char)(_f()));
        } else if (std::same_as<std::underlying_type_t<Enum>, int>) {
          return static_cast<Enum>((int)(i) - (int)(MAXN));
        } else if (!std::same_as<std::underlying_type_t<Enum>, unsigned char>) {
          return static_cast<Enum>(i);
        } else {
          return static_cast<Enum>(i);
        }
      }
    }

    return static_cast<Enum>(ret_val);
  }

private:
  template<my::tuple_concept T, int _Cnt>
  struct content;

  template<typename ...Args, int _Cnt>
  struct content<my::tuple_tp<Args...>, _Cnt>
  {
    static constexpr std::array<const char*, _Cnt> typed_content
    { __f__<(Enum)((std::underlying_type_t<Enum>)Args::tag_value)>()... };
  };

  template <int from, int to>
  static constexpr std::array<const char*, to - from + 1> const& get_cnt()
  { return content<typename my::range<from, to>::_rng, to - from + 1>::typed_content; }
};
