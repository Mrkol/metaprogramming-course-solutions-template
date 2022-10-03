#include <bits/iterator_concepts.h>
#include <cstddef>
#include <memory>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>

// remove
#include <type_traits>
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

template < class T, std::size_t extent = std::dynamic_extent, std::ptrdiff_t stride = 1>
class SliceBase : public SliceDynamicData<extent, stride> {
public:
  using TBase = SliceDynamicData<extent, stride>;
  SliceBase(std::size_t ext, std::ptrdiff_t strd)
      : TBase(ext, strd) {};
  SliceBase& SetExtent(std::size_t ext) noexcept requires (extent == std::dynamic_extent) {
    this->extent = ext;
    return *this;
  }

  SliceBase& SetStride(std::ptrdiff_t strd) noexcept requires (stride == dynamic_stride) {
    this->stride = strd;
    return *this;
  }

  SliceBase& SetExtentIfDynamic(std::size_t ext) noexcept {
    if (extent == std::dynamic_extent) {
      this->extent = ext;
    }
    return *this;
  }

  SliceBase& SetStrideIfDynamic(std::ptrdiff_t strd) noexcept {
    if constexpr (stride == dynamic_stride) {
      this->stride = strd;
    }
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

} // namespace NSliceImpl

template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1
  >
class Slice : public NSliceImpl::SliceBase<T, extent, stride> {
public:
  using TBase = NSliceImpl::SliceBase<T, extent, stride>;
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;


  Slice& SetExtent(std::size_t ext) noexcept requires (extent == std::dynamic_extent) {
    TBase::SetExtent(ext);
    return *this;
  }

  Slice& SetStride(std::ptrdiff_t strd) noexcept requires (stride == dynamic_stride) {
    TBase::SetStride(strd);
    return *this;
  }

  Slice& SetExtentIfDynamic(std::size_t ext) noexcept {
    TBase::SetExtentIfDynamic(ext);
    return *this;
  }

  Slice& SetStrideIfDynamic(std::ptrdiff_t strd) noexcept {
    TBase::SetStrideIfDynamic(strd);
    return *this;
  }

  using TBase::GetExtent;
  using TBase::GetStride;

  template <typename U, bool Reversed = false>
  class SliceIterator
    : public NSliceImpl::SliceBase<T, extent, stride> {
  public:
    using TBase = NSliceImpl::SliceBase<T, extent, stride>;

    using TBase::GetStride;
    using TBase::GetExtent;
    using iterator_category = std::random_access_iterator_tag;

    friend class Slice;
    using value_type = std::remove_cv_t<U>;
    using pointer = U*;
    using reference = U&;
    using const_reference = std::remove_cv_t<U>&;
    using difference_type = int32_t;

    SliceIterator() : TBase(0, 1) {}

    SliceIterator(const SliceIterator& other)
        : TBase(other.GetExtent(), other.GetStride())
        , ptr_(other.ptr_), offset_(other.offset_)
    {
    }
    SliceIterator& operator=(const SliceIterator& other) {
      ptr_ = other.ptr_;
      offset_ = other.offset_;
      return *this;
    }
    reference operator*() const {
      return *(ptr_ + offset_);
    }
    pointer operator->() const {
      return ptr_ + offset_;
    }
    SliceIterator& operator++() {
      offset_ += Reversed ? -static_cast<int32_t>(GetStride()) : GetStride();
      return *this;
    }
    SliceIterator& operator--() {
      offset_ -= Reversed ? -static_cast<int32_t>(GetStride()) : GetStride();
      return *this;
    }
    SliceIterator operator++(int) {
      SliceIterator res(ptr_, offset_);
      offset_ += Reversed ? -static_cast<int32_t>(GetStride()) : GetStride();
      return res;
    }
    SliceIterator operator--(int) {
      SliceIterator res(ptr_, offset_);
      offset_ -= Reversed ? -static_cast<int32_t>(GetStride()) : GetStride();
      return res;
    }
    SliceIterator& operator+=(difference_type n) {
      offset_ += n * (Reversed ? -static_cast<int32_t>(GetStride()) : GetStride());
      return *this;
    }
    friend SliceIterator operator+(SliceIterator iter, difference_type n) {
      return SliceIterator(iter.ptr_, iter.offset_) += Reversed ? -n : n;
    }
    friend SliceIterator operator+(difference_type n, SliceIterator iter) {
      return SliceIterator(iter.ptr_, iter.offset_) -= Reversed ? -n : n;
    }
    friend SliceIterator operator-(SliceIterator iter, difference_type n) {
      return SliceIterator(iter.ptr_, iter.offset_) += Reversed ? -n : n;
    }
    friend SliceIterator operator-(difference_type n, SliceIterator iter) {
      return SliceIterator(iter.ptr_, iter.offset_) -= Reversed ? -n : n;
    }
    SliceIterator& operator-=(difference_type n) {
      offset_ -= n * (Reversed ? -static_cast<int32_t>(GetStride()) : GetStride());
    }
    reference operator[](difference_type n) {
      return *(ptr_ + offset_ + n * (Reversed ? -static_cast<int32_t>(GetStride()) : GetStride()));
    }
    const_reference operator[](difference_type n) const {
      return *(ptr_ + offset_ + n * (Reversed ? -static_cast<int32_t>(GetStride()) : GetStride()));
    }
    bool operator==(const SliceIterator& other) const {
        return offset_ == other.offset_;
    }
    bool operator!=(const SliceIterator& other) const {
        return offset_ != other.offset_;
    }
    bool operator<(const SliceIterator& other) const {
        return offset_ < other.offset_;
    }
    bool operator>(const SliceIterator& other) const {
        return offset_ > other.offset_;
    }
    bool operator<=(const SliceIterator& other) const {
        return offset_ <= other.offset_;
    }
    bool operator>=(const SliceIterator& other) const {
        return offset_ >= other.offset_;
    }
    difference_type operator+(const SliceIterator& other) const {
        return offset_ + other.offset_;
    }
    difference_type operator-(const SliceIterator& other) const {
        return offset_ - other.offset_;
    }

  private:
    pointer ptr_;
    difference_type offset_;
  private:
    SliceIterator(pointer ptr, difference_type offset)
        : TBase(GetExtent(), GetStride())
        , ptr_(ptr), offset_(offset)
    {
    }
  };

  using iterator = SliceIterator<T>;
  using const_iterator = SliceIterator<const T>;
  using reverse_iterator = SliceIterator<T, true>;
  using const_reverse_iterator = SliceIterator<const T, true>;

  iterator begin() {
    return iterator(data_, 0);
  }

  iterator end() {
    return iterator(data_, GetStride() * GetExtent());
  }

  const_iterator begin() const {
    return const_iterator(data_, 0);
  }

  const_iterator end() const {
    return const_iterator(data_, GetStride() * GetExtent());
  }

  reverse_iterator rbegin() {
    return reverse_iterator(data_, GetStride() * (GetExtent() - 1));
  }

  reverse_iterator rend() {
    return reverse_iterator(data_, -static_cast<int32_t>(GetStride()));
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(data_, GetStride() * (GetExtent() - 1));
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(data_, -static_cast<int32_t>(GetStride()));
  }

  const_iterator cbegin() const {
    return const_iterator(data_, 0);
  }

  const_iterator cend() const {
    return const_iterator(data_, GetStride() * GetExtent());
  }
  reference operator[](size_type idx) {
    return *(data_ + GetStride() * idx);
  }
  const_reference operator[](size_t idx) const {
    return *(data_ + GetStride() * idx);
  }
  pointer Data() {
    return GetData();
  }
  const_pointer Data() const {
    return GetData();
  }
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

  template <typename U, std::size_t other_extent, std::ptrdiff_t other_stride>
  requires ((extent == std::dynamic_extent ||
             extent == other_extent) && (
             stride == dynamic_stride ||
             stride == other_stride))
  Slice(Slice<U, other_extent, other_stride> other) {
    SetData(other.data_);
    SetExtentIfDynamic(other.GetExtent());
    SetStrideIfDynamic(other.GetStride());
  }

  // // Data, Size, Stride, begin, end, casts, etc...

  constexpr difference_type Stride() const noexcept {
    return GetStride();
  }

  constexpr size_type Size() const noexcept {
    if constexpr (extent == std::dynamic_extent) {
      return GetExtent();
    }
    return extent;
  }

  constexpr bool operator==(const Slice& other) const noexcept {
    return GetExtent() == other.GetExtent() &&
           GetStride() == other.GetStride() &&
           GetData() == other.GetData();
  }

  constexpr bool operator!=(const Slice& other) const noexcept {
    return !(*this == other);
  }

  Slice<T, std::dynamic_extent, stride>
    First(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(GetData())
        .SetExtent(count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, count, stride>
    First() const {
    return Slice<T, count, stride>()
       .SetData(GetData())
       .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    Last(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(GetData() + (GetExtent() - count) * GetStride())
        .SetExtent(count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, count, stride>
    Last() const {
    return Slice<T, count, stride>()
        .SetData(GetData() + (GetExtent() - count) * GetStride())
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    DropFirst(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(GetData() + count * GetStride())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count> requires (extent != std::dynamic_extent)
  Slice<T, extent - count, stride>
    DropFirst() const {
    return Slice<T, extent - count, stride>()
        .SetData(GetData() + count * GetStride())
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, std::dynamic_extent, stride>
    DropFirst() const {
    return Slice<T, extent - count, stride>()
        .SetData(GetData() + count * GetStride())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, stride>
    DropLast(std::size_t count) const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(GetData())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count> requires (extent != std::dynamic_extent)
  Slice<T, extent - count, stride>
    DropLast() const {
    return Slice<T, extent - count, stride>()
        .SetData(GetData())
        .SetStrideIfDynamic(GetStride());
  }

  template <std::size_t count>
  Slice<T, std::dynamic_extent, stride>
    DropLast() const {
    return Slice<T, std::dynamic_extent, stride>()
        .SetData(GetData())
        .SetExtent(GetExtent() - count)
        .SetStrideIfDynamic(GetStride());
  }

  Slice<T, std::dynamic_extent, dynamic_stride>
    Skip(std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, dynamic_stride>()
        .SetData(GetData())
        .SetExtent((GetExtent() - 1) / skip + 1)
        .SetStride(GetStride() * skip);
  }

  template <std::ptrdiff_t skip> requires (extent != std::dynamic_extent && stride != dynamic_stride)
  Slice<T, (extent - 1) / skip + 1, stride * skip>
    Skip() const {
    return Slice<T, (extent - 1) / skip + 1, stride * skip>()
        .SetData(GetData());
  }

  template <std::ptrdiff_t skip> requires (extent == std::dynamic_extent && stride != dynamic_stride)
  Slice<T, std::dynamic_extent, stride * skip>
    Skip() const {
    return Slice<T, std::dynamic_extent, stride * skip>()
        .SetData(GetData())
        .SetExtent((GetExtent() - 1) / skip + 1);
  }

  template <std::ptrdiff_t skip> requires (extent == std::dynamic_extent && stride == dynamic_stride)
  Slice<T, std::dynamic_extent, stride * skip>
    Skip() const {
    return Slice<T, std::dynamic_extent, dynamic_stride>()
        .SetData(GetData())
        .SetExtent((GetExtent() - 1) / skip + 1)
        .SetStride(GetStride() * skip);
  }

private:
  pointer data_;

public:
  constexpr Slice() noexcept
      : TBase(GetExtent(), GetStride()) {};

  pointer GetData() const noexcept {
    return data_;
  }

  Slice& SetData(T* data) noexcept {
    data_ = data;
    return *this;
  }
};

static_assert();

// static_assert(std::random_access_iterator<Slice<int, 2, 3>::iterator>);

// int main() {
//   std::vector<int> a{};
//   for (size_t i = 0; i < 100; ++i) {
//     a.push_back(i);
//   }

//   Slice<int, 15, 2> slice(a);
//   for (auto&& item : slice) {
//     std::cout << item << " ";
//   }
//   std::cout << std::endl;

//   auto fst = slice.First<5>();
//   for (auto&& item : fst) {
//     std::cout << item << " ";
//   }
//   std::cout << std::endl;

//   auto fort = fst.Skip<2>();
//   for (auto&& item : fort) {
//     std::cout << item << " ";
//   }
//   std::cout << std::endl;

//   // slice.SetExtent(2);
//   return 0;
// }