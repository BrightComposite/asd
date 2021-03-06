//---------------------------------------------------------------------------

#pragma once

#ifndef META_H
#define META_H

//---------------------------------------------------------------------------

#include "adapt.h"
#include "cast.h"

//---------------------------------------------------------------------------

namespace asd
{
	namespace internals
	{
		template<int I, typename ... T>
		struct cut_t {};
		
		template<int I, typename H, typename ... T>
		struct cut_t<I, H, T...> : cut_t<I - 1, T...> {};
		
		template<typename H, typename ... T>
		struct cut_t<0, H, T...>
		{
			typedef H head;
			typedef tuple<H, T...> type;
		};
		
		template<int I>
		struct cut_t<I>
		{
			typedef empty head;
			typedef empty type;
		};
		
		template<int I, typename ... T>
		struct cut_t<I, types<T...>> : cut_t<I, T...> {};
		
		template<int I, typename ... T>
		struct cut_t<I, tuple<T...>> : cut_t<I, T...> {};
	}
	
	/**
	 *  @brief
	 *  Used by other meta-structures to
	 *  get type of generation I of some variadic-templated class
	 */
	template<int I, typename ... T>
	using cut_t = typename internals::cut_t<I, T...>::type;
	
	template<int I, typename ... T>
	using extract_t = typename internals::cut_t<I, T...>::head;
	
	template<typename ... T>
	using first_t = extract_t<0, T...>;
	
	template<typename ... T>
	using last_t = extract_t<sizeof...(T) - 1, T...>;
	
	namespace internals
	{
		template<int N, typename T, typename ... S>
		struct expand_t : expand_t<N - 1, T, decay_t<T>, S...> {};
		
		template<typename T, typename ... S>
		struct expand_t<0, T, S...>
		{
			typedef tuple<S...> type;
		};
	}
	
	/**
	 *  @brief
	 *  Returns tuple<S ...> with N elements of type T
	 */
	template<int I, typename H, typename ... S>
	using expand_t = typename internals::expand_t<I, H, S...>::type;
	
	namespace internals
	{
		template<int R, typename T, typename H, typename ... S>
		struct find_t : find_t<R + 1, T, S...> {};
		
		template<int R, typename T, typename ... S>
		struct find_t<R, T, T, S ...> : std::integral_constant<int, R> {};
		
		template<int R, typename T, typename H>
		struct find_t<R, T, H> : std::integral_constant<int, -1> {};
	}
	
	/**
	 *  @brief
	 *  Seeks for the type T in template-pack.
	 *  Returns pos of type in set if it was found, and -1 otherwise.
	 */
	template<typename T, typename H, typename ... S>
	struct find_t : internals::find_t<0, T, H, S...> {};
	
	/**
	 *  @brief
	 *  Seeks for the type T in template-pack.
	 *  Returns pos of type in set if it was found, and -1 otherwise.
	 */
	template<template<typename...> class Tpl, typename T, typename H, typename ... S>
	struct find_t<T, Tpl<H, S...>> : internals::find_t<0, T, H, S...> {};
	
	/**
	 *	@brief
	 *  Seeks for the type T in <A...> .
	 *  Returns true_type if it was found and false_type otherwise.
	 *	T - target type, A... - set of keys, H - first element of set.
	 */
	template<typename T, typename ... A>
	struct has_type {};
	
	/**
	 *	@brief
	 *  Seeks for the type T in <A...> .
	 *  Returns true_type if it was found and false_type otherwise.
	 *	T - target type, A... - set of keys, H - first element of set.
	 */
	template<typename T, typename H, typename ... A>
	struct has_type<T, H, A...> : has_type<T, A...> {};
	
	/**
	 *	@brief
	 *  Seeks for the type T in <A...> .
	 *  Returns true_type if it was found and false_type otherwise.
	 *	T - target type, A... - set of keys, H - first element of set.
	 */
	template<typename T, typename ... A>
	struct has_type<T, T, A...> : true_type {};
	
	/**
	 *	@brief
	 *  Seeks for the type T in <A...> .
	 *  Returns true_type if it was found and false_type otherwise.
	 *	T - target type, A... - set of keys, H - first element of set.
	 */
	template<typename T>
	struct has_type<T> : false_type {};

//---------------------------------------------------------------------------
	
	/**
	 *	@brief
	 *  Iterates through the tuple types with the Functor.
	 */
	template<class ...>
	struct foreach_t
	{
		template<typename Functor, bool Cond, class ... A>
		static inline bool iterate(A && ...) {
			return Cond;
		}
		
		template<typename Functor, class ... A>
		static inline void iterate(A && ...) {}
	};
	
	/**
	 *	@brief
	 *  Iterates through the tuple types with the Functor.
	 */
	template<class ... A>
	struct foreach_t<tuple<A...>> : foreach_t<A...> {};
	
	/**
	 *	@brief
	 *  Iterates through the tuple types with the Functor.
	 */
	template<class H, class ... T>
	struct foreach_t<H, T ...>
	{
		template<typename Functor, bool Cond, class ... A, useif<
			is_same<decltype(Functor::template iterate<H>(declval<A>()...)), bool>::value
		>
		>
		static inline bool iterate(A && ... args) {
			if(Functor::template iterate<H>(forward<A>(args)...) == !Cond) {
				return !Cond;
			}
			
			return foreach_t<T...>::template iterate<Functor, Cond>(forward<A>(args)...);
		}
		
		template<typename Functor, class ... A, typename = decltype(Functor::template iterate<H>(declval<A>()...))>
		static inline void iterate(A && ... args) {
			Functor::template iterate<H>(forward<A>(args)...);
			foreach_t<T...>::template iterate<Functor>(forward<A>(args)...);
		}
	};
	
	/**
	 *	@brief
	 *  Iterates through the tuple types with the Functor.
	 */
	template<>
	struct foreach_t<>
	{
		template<typename Functor, bool Cond, class ... A>
		static inline bool iterate(A && ...) {
			return Cond;
		}
		
		template<typename Functor, class ... A>
		static inline void iterate(A && ...) {}
	};

	template<template<class> class F, class T, T ... E>
	struct accum_t {};

	template<template<class> class F, class T, T Head, T ... Tail>
	struct accum_t<F, T, Head, Tail...>
	{
		static constexpr T value = F<T>()(Head, accum_t<F, T, Tail...>::value);
	};

	template<template<class> class F, class T>
	struct accum_t<F, T>
	{
		static constexpr T value = T(0);
	};

	template<class T, T ... E>
	using sum_t = accum_t<std::plus, T, E...>;

	template<class T, T ... E>
	using mul_t = accum_t<std::multiplies, T, E...>;

	template<char ... Ch>
	struct char_sequence
	{
		static constexpr const char value[sizeof...(Ch) + 1] = {Ch..., '\0'};
	};

	template<char ... Ch>
	constexpr const char char_sequence<Ch...>::value[sizeof...(Ch) + 1];

	namespace internals
	{
		template<class T, class...>
		struct concat
		{
			using type = T;
		};

		template<char... Dest, char ... Src, class... Ts>
		struct concat<char_sequence<Dest...>, char_sequence<Src...>, Ts...> : concat<char_sequence<Dest..., Src...>, Ts...> {};
	}

	template<class... Ts>
	using concat_t = typename internals::concat<Ts...>::type;
}

//---------------------------------------------------------------------------
#endif
