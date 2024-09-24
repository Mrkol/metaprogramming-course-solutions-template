#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <span>
#include <type_traits>
#include <vector>


template
	< size_t extent
	>
class _span_base {
protected:
	_span_base(size_t ext)
	{
		if (extent != ext) {
			throw ::mpc::detail::AssertException();
		}
	}

	static consteval size_t Size()
	{
		return extent;
	}
};

template <>
class _span_base
	< std::dynamic_extent
	> {
protected:
	_span_base(): _ext(0) {}

	_span_base(size_t ext)
		: _ext(ext)
	{
		if (ext != Size()) {
			throw ::mpc::detail::AssertException();
		}
	}

	size_t Size() const { return _ext; }

private:
	size_t _ext;
};

template
  < class _Tp
  , size_t extent = std::dynamic_extent>
class Span : private _span_base<extent> {
public:
  // Like in c++lib
	using element_type     = _Tp;
	using value_type       = std::remove_cv_t<_Tp>;
	using size_type        = size_t;
	using difference_type  = ptrdiff_t;
	using pointer          = _Tp*;
	using const_pointer    = const _Tp*;
	using reference        = element_type&;
	using const_reference  = const element_type&;
	using iterator         = pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	
	using _span_base<extent>::Size;

	Span(_Tp *ptr, size_t ext)
		: _span_base<extent>(ext)
		, _ptr(ptr)
	{}

	template <std::contiguous_iterator Iter>
	Span(Iter it, size_t ext)
		: _span_base<extent>(ext)
		, _ptr(std::to_address(it))
	{}

	template<typename Range>
	Span(Range&& r) requires (std::ranges::range<Range>)
		: _span_base<extent>(std::ranges::size(r))
		, _ptr(std::ranges::data(r))
	{}

	template <class Container>
	Span(Container& c)
		: _span_base<extent>(c.size())
		, _ptr(std::to_address(c.begin()))
	{}

  ~Span() = default;

	_Tp& operator[](const size_t idx)
	{
		if (_ptr == nullptr || idx > Size()) {
			throw ::mpc::detail::AssertException();
		}

		return *(_ptr + idx);
	}

	_Tp& operator[](const size_t idx) const
	{
		if (_ptr == nullptr || idx > Size()) {
			throw ::mpc::detail::AssertException();
		}

		return *(_ptr + idx);
	}

	_Tp& Front() {
		if (begin() == nullptr) {
			throw ::mpc::detail::AssertException();
		}
		
		return this->operator[](0);
	}

	_Tp& Front() const{
		if (begin() == nullptr) {
			throw ::mpc::detail::AssertException();
		}

		return this->operator[](0);
	}

	_Tp& Back() {
		if (end() == nullptr) {
			throw ::mpc::detail::AssertException();
		}

		return (this)->operator[](Size() - 1);
	}

	_Tp& Back() const{
		if (end() == nullptr) {
			throw ::mpc::detail::AssertException();
		}
		
		return (this)->operator[](Size() - 1);
	}

	iterator          begin() const { return _ptr; }
	iterator          end()   const { return _ptr + Size(); }
	reverse_iterator rend()   const { return reverse_iterator(_ptr); }
	reverse_iterator rbegin() const { return reverse_iterator(_ptr + Size()); }


	template<size_t Sz>
	Span<_Tp, Sz> First ()
	{
		if (Size() < Sz) {
			throw ::mpc::detail::AssertException();
		}

		return Span<_Tp, Sz>(_ptr, Sz);
	}

	Span<_Tp> First (size_t sz)
	{
		if (Size() < sz) {
			throw ::mpc::detail::AssertException();
		}
		
		return Span<_Tp>(_ptr, sz);
	}

	template<size_t Sz>
	Span<_Tp, Sz> Last ()
	{
		if (Size() < Sz) {
			throw ::mpc::detail::AssertException();
		}
		
		return Span<_Tp, Sz>(_ptr + Size() - Sz, Sz);
	}

	Span<_Tp> Last (size_t sz)
	{
		if (Size() < sz) {
			throw ::mpc::detail::AssertException();
		}

		return Span<_Tp>(_ptr + Size() - sz, sz);
	}

	_Tp* Data() noexcept { return _ptr; }

private:
	_Tp *_ptr;
};


template <typename _Tp>
Span(std::vector<_Tp>&) -> Span<std::remove_reference_t<_Tp>>;

template <typename _Tp, size_t extent>
Span(std::array<_Tp, extent>&) -> Span<std::remove_reference_t<_Tp>, extent>;

template <std::contiguous_iterator Iterator, typename extent>
Span(Iterator, extent) -> Span<std::remove_reference_t<typename Iterator::value_type>>;
