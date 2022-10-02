#include <cstddef>
#include <memory>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>

// remove
#include <vector>
#include <iostream>
// ----

inline constexpr std::ptrdiff_t dynamic_stride = -1;

template <typename T>
concept BasicContainer = requires (T t) { t.data(); t.size(); };

namespace NSliceImpl {

template <size_t extent, std::ptrdiff_t stride>
struct SliceDynamicData {
  SliceDynamicData() = default;
  SliceDynamicData(std::size_t, std::ptrdiff_t) {}
};

template <std::ptrdiff_t stride>
struct SliceDynamicData<std::dynamic_extent, stride> {
  SliceDynamicData() = default;
  SliceDynamicData(std::size_t extent, std::ptrdiff_t)
      : extent(extent) {}
  std::size_t extent;
};

template <std::size_t extent>
struct SliceDynamicData<extent, dynamic_stride> {
  SliceDynamicData() = default;
  SliceDynamicData(std::size_t, std::ptrdiff_t stride)
      : stride(stride) {}
  std::ptrdiff_t stride;
};

template <>
struct SliceDynamicData<std::dynamic_extent, dynamic_stride> {
  SliceDynamicData() = default;
  SliceDynamicData(std::size_t extent, std::ptrdiff_t stride)
      : extent(extent), stride(stride) {}
  std::size_t extent;
  std::ptrdiff_t stride;
};

// template <typename T, std::size_t extent, std::ptrdiff_t stride>
// class SliceContainer : public SliceDynamicData<extent, stride> {
// public:
//   using TBase = SliceDynamicData<extent, stride>;
//   constexpr T* GetData() const noexcept {
//     return data_;
//   }
//   TBase& MutableDynamicData() noexcept {
//     return *this;
//   }

// private:
//   T* data_;
// protected:

// };

} // namespace NSliceImpl

template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice : protected NSliceImpl::SliceDynamicData<extent, stride> {
public:
  using TBase = NSliceImpl::SliceDynamicData<extent, stride>;
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  template<BasicContainer U> requires (stride != dynamic_stride)
  constexpr Slice(U& container)
      : data_(container.data())
      , TBase((container.size() - 1) / stride + 1, stride)
  {
  }

  template <std::contiguous_iterator It> requires (stride != dynamic_stride)
  Slice(It first, std::size_t count, std::ptrdiff_t skip)
      : data_(std::to_address(first))
      , TBase(count, stride)
  {
  }

  // // Data, Size, Stride, begin, end, casts, etc...

  constexpr std::ptrdiff_t Stride() const noexcept {
    return GetStride();
  }

  Slice<T, std::dynamic_extent, stride>
    First(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(data_)
        .SetExtent(count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, count, stride>
    First() const {
    return Slice<T, count, stride>()
        .SetData(data_)
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    Last(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(data_ + (GetExtent() - count) * GetStride())
        .SetExtent(count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, count, stride>
    Last() const {
    return Slice<T, count, stride>()
        .SetData(data_ + (GetExtent() - count) * GetStride())
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    DropFirst(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(data_ + count * GetStride())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count> requires (extent != std::dynamic_extent)
  Slice<T, extent - count, stride>
    DropFirst() const {
    return Slice<T, extent - count, stride>()
        .SetData(data_ + count * GetStride())
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, std::dynamic_extent, stride>
    DropFirst() const {
    return Slice<T, extent - count, stride>()
        .SetData(data_ + count * GetStride())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    DropLast(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(data_)
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count> requires (extent != std::dynamic_extent)
  Slice<T, extent - count, stride>
    DropLast() const {
    return Slice<T, extent - count, stride>()
        .SetData(data_)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, std::dynamic_extent, stride>
    DropLast() const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(data_)
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, dynamic_stride>
    Skip(std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, dynamic_stride>()
        .SetData(data_)
        .SetExtent((GetExtent() - 1) / skip + 1)
        .SetStride(GetStride() * skip);
  }

  template <std::ptrdiff_t skip> requires (extent != std::dynamic_extent && stride != dynamic_stride)
  Slice<T, (extent - 1) / skip + 1, stride * skip>
    Skip() const {
    return Slice<T, (extent - 1) / skip + 1, stride * skip>()
        .SetData(data_);
  }

  template <std::ptrdiff_t skip> requires (extent == std::dynamic_extent && stride != dynamic_stride)
  Slice<T, std::dynamic_extent, stride * skip>
    Skip() const {
    return Slice<T, std::dynamic_extent, stride * skip>()
        .SetData(data_)
        .SetExtent((GetExtent() - 1) / skip + 1);
  }

  template <std::ptrdiff_t skip> requires (extent == std::dynamic_extent && stride == dynamic_stride)
  Slice<T, std::dynamic_extent, stride * skip>
    Skip() const {
    return Slice<T, std::dynamic_extent, dynamic_stride>()
        .SetData(data_)
        .SetExtent((GetExtent() - 1) / skip + 1)
        .SetStride(GetStride() * skip);
  }

private:
  T* data_;
  // NSliceImpl::SliceDynamicData<extent, stride> this->;
private:
  constexpr Slice() : data_(nullptr) {}
  Slice& SetExtent(std::size_t ext) noexcept requires (extent == std::dynamic_extent) {
    this->extent = ext;
    return *this;
  }

  Slice& SetStride(std::ptrdiff_t strd) noexcept requires (stride == dynamic_stride) {
    this->stride = strd;
    return *this;
  }

  Slice& SetExtentIfDynamic(std::size_t ext) noexcept {
    if (extent == std::dynamic_extent) {
      this->extent = ext;
    }
    return *this;
  }

  Slice& SetStrideIfDynamic(std::ptrdiff_t strd) noexcept {
    if constexpr (stride == dynamic_stride) {
      this->stride = strd;
    }
    return *this;
  }

  Slice& SetData(pointer data) noexcept {
    data_ = data;
    return *this;
  }

  constexpr std::size_t GetExtent() const noexcept {
    if constexpr (extent == std::dynamic_extent) {
      return this->extent;
    } else {
      return extent;
    }
  }

  constexpr std::ptrdiff_t GetStride() const noexcept {
    if constexpr (stride == dynamic_stride) {
      return this->stride;
    } else {
      return stride;
    }
  }
};

// int main() {
//   std::vector<int> a(100, 2);
//   Slice<int, 5> slice(a);
//   auto fst = slice.First<5>();
//   auto fort = fst.Skip<2>();
//   // slice.SetExtent(2);
//   return 0;
// }