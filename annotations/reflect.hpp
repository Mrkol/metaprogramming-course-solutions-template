#pragma once

#include <bits/utility.h>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

template <typename Field_T, typename... Anns>
struct descriptor;

template<class... Anns>
class Annotate {};

using std::tuple;


// Object characteristics concept

// Compiles when Struct is non-union class
//   triv - continuous storage
template<class Object>
concept good = std::is_class_v<Object>;

// struct/class T is constructioble with args Args
template<class Object, typename... Tps>
concept is_constructible = requires(Tps&&... vs) { Object{vs...}; };

namespace reflect
{

  // Fields implementation
  // I do it via friend injection (somewhere in last lectures)

  // Tag of the field
	template<size_t I, class Object>
	struct field_tag
	{
	// 	// Structure (field_tag) links connected
	// 	//   field function that will return default_init same of the field
	// 	// template<class U>
		friend auto field(field_tag<I, Object>);
	};

	template< size_t I, class Object, class U>
	struct set_field_function : public field_tag<I,Object>
	{
		// structure set_field_function
		//   create friend function that can be called 
		friend auto field(field_tag<I, Object>)
	  { return U(); };
	};

	template <size_t I, good Object>
	struct field_func_initializer
	{
		template <class U>
		constexpr operator U()
		{
			// create connection between field_tag<I, Object> and it's field function
			// Can't init without calling
			set_field_function<I, Object, U>();
			// Use the function
			return U();
		}
	};







  // End of recursion
	template <good Object, size_t N, typename... Tps>
	struct object_description
	{ using _Tp = tuple<tuple<>, Tps...>; };

  // The idea is that
	//   first time we call object_description
	// 
	//   It will return
	//   Type=object_description<Object, 1st_field_func()>::Type
	// 
	//     which will return
	//     Type = object_description<Object, 1st_field_func(), 2nd_field_func()>::Type
	// ...
	// So each recursive call of (object_description) replace Tps={T, tail} -> {func, Tail}
	// But we should ensure that our object is trivial
	template <good Object, size_t N, typename... Fields>
	requires(is_constructible<Object, Fields..., field_func_initializer<N, Object>>)
	struct object_description<Object, N, Fields...>
	{
		// Replace (i)-th type with it's field_func and and do it
		//   with next arguments
		using _Tp = object_description<Object, N+1, Fields..., decltype(field(field_tag<N, Object>()))>::_Tp;
	};







  // Describe algorithm
	template<class fields, class ds >
	struct _descr_iml
	{ using _Tp = ds; };

	// push back file description with
	//   descriptor<first field,annotations to the filed>
	template
	  < class    H
		, class... Tl
		, class... anns
		, class... ds
		>
	struct _descr_iml
	  // Was passed field to the first place
	  <tuple<tuple<anns...>, H, Tl...>
	  , tuple<ds...>
		>
	{
		// Pass new file_description
		using _Tp = _descr_iml
		  < tuple<tuple<>, Tl...>
			, tuple<ds..., descriptor<H, anns...> >
			>::_Tp;
	};

  // push back H to annotatiable list
	template
	  < class ...H
		, class ...Tl
		, class ...anns
		, class ...ds
		>
	struct _descr_iml
	  < tuple<tuple<anns...>, Annotate<H...>, Tl...>
		, tuple<ds...>
		>
	{
		// push arguments to the tuple
		using _Tp = _descr_iml
		  < tuple<tuple<anns..., H...>, Tl...>
			, tuple<ds...>
			>::_Tp;
	};
}


template
  < template <typename... Args>
	  typename Tml, typename T >
struct is_same_template
{ static constexpr bool same = false; };

template
  < template <typename ...Args>
	  typename Tml, typename ...Ts>
struct is_same_template<Tml, Tml<Ts...>>
{ static constexpr bool same = true; };

// Iterated whole templates
// and we didn't find the instance
template
  < template <typename ...Args>
	  typename Tml, typename ...Ts >
struct find_first
{ using _tml = void; };

// Pass next element in tais
template
  < template <typename... Args>
    typename Tml, typename H, typename... Tail>
struct find_first<Tml, H, Tail...>
{ using _tml = find_first<Tml, Tail...>::_tml; };

// If found template
template
  < template < typename... Args>
	  typename Tml, typename H, typename... Tail>
requires(is_same_template<Tml, H>::same)
struct find_first<Tml, H, Tail...>
{ using _tml = H;};

// Checks if the field has any annotations
// template
// 	< template < class ...Args>
// 	  typename Tml >
// static constexpr bool has_annotation_template = (is_same_template<Tml, Anns>::same || ...);
template <template<class...> typename TargetTemplate, typename ...Instances>
struct find_template_instance;

template
	< template<class...> typename TargetTemplate
	, typename T
	, typename ...Instances >
struct find_template_instance
	< TargetTemplate
	, T
	, Instances... >
{ static constexpr bool value = find_template_instance<TargetTemplate, Instances...>::value; };

template
	< template<class...> typename TargetTemplate
	, typename... TArgs
	, typename ...Instances >
struct find_template_instance
	< TargetTemplate
	, TargetTemplate<TArgs...>
	, Instances...>
{ static constexpr bool value = true; };

template
	< template<class...> typename TargetTemplate >
struct find_template_instance
	< TargetTemplate >
{ static constexpr bool value = false; };





// Field_T is field descriptor of the structure (T)
// Anns is all annotations
// Anns is inner characteristics of Annotate< Anns , ... >
// Anns connected with field
template <typename Field_T, typename... Anns>
struct descriptor {
	// Type of the field
	using Type = Field_T;
	// All Annotate<Ann1, Ann2, ...> connected with field_T
	using Annotations = Annotate<Anns...>;

	template
		< template<class...> typename Tml>
	static constexpr bool has_template = find_template_instance<Tml, Anns...>::value;

	template
		< template<class...> typename Tml>
	static constexpr bool has_annotation_template = has_template<Tml>;
	
	// Check is same class have Annotation and one element of aanotation
	template<typename A>
	static constexpr bool has_annotation_class = (std::is_same_v<A, Anns> || ...);

	template
		< template < typename...> typename Tml>
	requires has_annotation_template<Tml>
	using FindAnnotation = find_first<Tml, Anns...>::_tml;
};







// Struct T
template <class T>
struct Describe {
	static constexpr size_t num_fields = std::tuple_size_v<typename reflect::_descr_iml
	  // Build description for the object
	  < typename reflect::object_description<T, 0>::_Tp
		// and try to build for descriptors
		, tuple<>
	  >::_Tp>;
	
	template <size_t I>
	using Field = std::tuple_element_t<I, typename reflect::_descr_iml
	  // Build description for the object
	  < typename reflect::object_description<T, 0>::_Tp
		// and try to build for descriptors
		, tuple<>
	  // Last annotations
		>::_Tp>;
};
