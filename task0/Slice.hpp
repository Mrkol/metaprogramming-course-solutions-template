#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>

inline constexpr std::ptrdiff_t dynamic_stride = -1;

template <typename T>
concept BasicContainer = requires (T t) { t.data(); t.size(); };

template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice {
public:
  template<class U>
  constexpr Slice(U& container)
      : data_(container.data()) {
    if constexpr (extent != std::dynamic_extent) {
      extent_ = extent;
    } else {
      extent_ = (container.size() - 1) / stride + 1;
    }
  }

  // template <std::contiguous_iterator It>
  // Slice(It first, std::size_t count, std::ptrdiff_t skip);

  // // Data, Size, Stride, begin, end, casts, etc...

  // Slice<T, std::dynamic_extent, stride>
  //   First(std::size_t count) const;

  // template <std::size_t count>
  // Slice<T, /*?*/, stride>
  //   First() const;

  // Slice<T, std::dynamic_extent, stride>
  //   Last(std::size_t count) const;

  // template <std::size_t count>
  // Slice<T, /*?*/, stride>
  //   Last() const;

  // Slice<T, std::dynamic_extent, stride>
  //   DropFirst(std::size_t count) const;

  // template <std::size_t count>
  // Slice<T, /*?*/, stride>
  //   DropFirst() const;

  // Slice<T, std::dynamic_extent, stride>
  //   DropLast(std::size_t count) const;

  // template <std::size_t count>
  // Slice<T, /*?*/, stride>
  //   DropLast() const;

  // Slice<T, /*?*/, /*?*/>
  //   Skip(std::ptrdiff_t skip) const;

  // template <std::ptrdiff_t skip>
  // Slice<T, /*?*/, /*?*/>
  //   Skip() const;

private:
  T* data_;
  std::size_t extent_;
  // std::ptrdiff_t stride_; ?
};
