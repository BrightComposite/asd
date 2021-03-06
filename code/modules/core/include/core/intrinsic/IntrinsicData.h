//---------------------------------------------------------------------------

#pragma once

#ifndef INTRINSIC_DATA_H
#define INTRINSIC_DATA_H

//---------------------------------------------------------------------------

#include <meta/preprocessor.h>

//---------------------------------------------------------------------------

#define SIMD_SSE1 	0
#define SIMD_SSE2 	1
#define SIMD_SSE3 	2
#define SIMD_SSSE3 	3
#define SIMD_SSE4 	4
#define SIMD_SSE4A 	5
#define SIMD_SSE4_1 6
#define SIMD_SSE4_2 7

#define SIMD_AVX 	10
#define SIMD_AVX2 	11

#define SIMD_LEVEL pp_cat(SIMD_, _SIMD_LEVEL)

//---------------------------------------------------------------------------

#ifdef MSVC
#include <intrin.h>
#endif // MSVC

#if SIMD_LEVEL >= SIMD_SSE1
#include <xmmintrin.h>
#endif
#if SIMD_LEVEL >= SIMD_SSE2
#include <emmintrin.h>
#endif
#if SIMD_LEVEL >= SIMD_SSE3
#include <pmmintrin.h>
#endif
#if SIMD_LEVEL >= SIMD_SSE4
#include <tmmintrin.h>
#endif
#if SIMD_LEVEL >= SIMD_SSE4A
#include <ammintrin.h>
#endif
#if SIMD_LEVEL >= SIMD_SSE4_1
#include <smmintrin.h>
#endif

#if SIMD_LEVEL >= SIMD_AVX
#include <x86intrin.h>
#endif

#include <memory.h>

#include <meta/meta.h>

#undef min
#undef max

#if defined(ARCH_X86) && __cplusplus <= 201402L
#define UNALIGNED_VECTORS
#endif

//---------------------------------------------------------------------------

namespace asd
{
	template<class T, int N>
	struct IntrinType {};

#define declare_intrin_type(T, N, intrin)	\
	template<>								\
	struct IntrinType<T, N>					\
	{										\
		using type = intrin;				\
	}

	struct __m16
	{
		union
		{
			struct
			{
				byte x, y;
			};

			__int16 i;
			byte a[2];
		};
	};

	struct __m32
	{
		union
		{
			struct
			{
				byte x, y, z, w;
			};

			__int32 i;
			byte a[4];
		};
	};

	declare_intrin_type(byte,   2, __m16);
	declare_intrin_type(int,    2, __m64);
	declare_intrin_type(float,  2, __m64);
	declare_intrin_type(double, 2, __m128d);

	declare_intrin_type(byte,   4, __m32);
	declare_intrin_type(int,    4, __m128i);
	declare_intrin_type(float,  4, __m128);
	
#if SIMD_LEVEL >= SIMD_AVX
	declare_intrin_type(double, 4, __m256d);
#endif
	
	template<class T>
	struct is_intrin : false_type {};

	template<>
	struct is_intrin<__m32>		 : true_type {};
	template<>
	struct is_intrin<__m64>		 : true_type {};
	template<>
	struct is_intrin<__m128>	 : true_type {};
	template<>
	struct is_intrin<__m128i>	 : true_type {};
	template<>
	struct is_intrin<__m128d>	 : true_type {};
	template<>
	struct is_intrin<__m128d[2]> : true_type {};

#if SIMD_LEVEL >= SIMD_AVX
	template<>
	struct is_intrin<__m256>	 : true_type {};
	template<>
	struct is_intrin<__m256i>	 : true_type {};
	template<>
	struct is_intrin<__m256d>	 : true_type {};
#endif

	template<class T, int N>
	using intrin_t = typename IntrinType<T, N>::type;

	template<class T, int N>
	using intrin_base_t = remove_extent_t<intrin_t<T, N>>;

	template<typename T, int N, size_t sz = array_size<intrin_t<T, N>>::value>
	struct alignas(sizeof(intrin_t<T, N>)) IntrinData
	{
		typedef intrin_t<T, N> type;
		typedef array<T, N> inner;
		typedef IntrinData<T, N / sz> unit;

		static const size_t stride = sizeof(intrin_base_t<T, N>);

		union
		{
			inner data;
			type v;
		};

		member_cast(v, type);
		member_cast(data, inner);

		IntrinData() {}
		IntrinData(const type & v)
		{
			intrin_cvt(v, this->v);
		}

		template<class U, useif<is_intrin<T>::value>>
		IntrinData(const U & v)
		{
			intrin_cvt(v, this->v);
		}

		IntrinData & operator = (const type & v)
		{
			intrin_cvt(v, this->v);
			return *this;
		}

		unit & operator [] (size_t index)
		{
			return *reinterpret_cast<unit *>(&v[index]);
		}

		const unit & operator [] (size_t index) const
		{
			return *reinterpret_cast<const unit *>(&v[index]);
		}

		unit & operator [] (int index)
		{
			return *reinterpret_cast<unit *>(&v[index]);
		}

		const unit & operator [] (int index) const
		{
			return *reinterpret_cast<const unit *>(&v[index]);
		}

		template<int I, useif<(I < N)>>
		static inline T __vectorcall get(const IntrinData & in)
		{
			return in.data[I];
		}

		template<int I, useif<(I < N)>>
		static inline void __vectorcall set(type & out, T value)
		{
			reinterpret_cast<IntrinData *>(out)->data[I] = value;
		}
	};

	template<typename T>
	struct alignas(sizeof(intrin_t<T, 2>)) IntrinData<T, 2, 0>
	{
		typedef intrin_t<T, 2> type;
		typedef array<T, 2> inner;

		union
		{
			inner data;
			type v;

			struct
			{
				T x, y;
			};
		};

		member_cast(v, type);
		member_cast(data, inner);

		IntrinData() {}
		IntrinData(const type & v)
		{
			intrin_cvt(v, this->v);
		}

		template<class U, useif<is_intrin<U>::value>>
		IntrinData(const U & v)
		{
			intrin_cvt(v, this->v);
		}

		IntrinData & operator = (const type & v)
		{
			intrin_cvt(v, this->v);
			return *this;
		}

		template<int I, useif<(I < 2)>>
		static inline T get(const IntrinData & in)
		{
			return in.data[I];
		}

		template<int I, useif<(I < 2)>>
		static inline void set(type & out, T value)
		{
			reinterpret_cast<IntrinData *>(out)->data[I] = value;
		}
	};

	template<typename T, size_t sz>
	struct alignas(sizeof(intrin_t<T, 4>)) IntrinData<T, 4, sz>
	{
		typedef intrin_t<T, 4> type;
		typedef array<T, 4> inner;
		typedef IntrinData<T, 4 / sz> unit;

		static const size_t stride = sizeof(intrin_base_t<T, 4>);

		union
		{
			inner data;
			type v;

			struct
			{
				T x, y, z, w;
			};
		};

		member_cast(v, type);
		member_cast(data, inner);

		IntrinData() {}

		IntrinData(const type & v)
		{
			intrin_cvt(v, this->v);
		}

		template<class U, useif<is_intrin<T>::value>>
		IntrinData(const U & v)
		{
			intrin_cvt(v, this->v);
		}

		IntrinData & operator = (const type & v)
		{
			intrin_cvt(v, this->v);
			return *this;
		}

		unit & operator [] (size_t index)
		{
			return *reinterpret_cast<unit *>(&v[index]);
		}

		const unit & operator [] (size_t index) const
		{
			return *reinterpret_cast<const unit *>(&v[index]);
		}

		unit & operator [] (int index)
		{
			return *reinterpret_cast<unit *>(&v[index]);
		}

		const unit & operator [] (int index) const
		{
			return *reinterpret_cast<const unit *>(&v[index]);
		}

		template<int I, useif<(I < 4)>>
		static inline T get(const IntrinData & in)
		{
			return in.data[I];
		}

		template<int I, useif<(I < 4)>>
		static inline void set(type & out, T value)
		{
			reinterpret_cast<IntrinData *>(out)->data[I] = value;
		}
	};

	template<typename T>
	struct alignas(sizeof(intrin_t<T, 4>)) IntrinData<T, 4, 0>
	{
		typedef intrin_t<T, 4> type;
		typedef array<T, 4> inner;

		union
		{
			inner data;
			
			struct
			{
				T x, y, z, w;
			};
		};

		member_cast(data, inner);

		IntrinData() {}
		IntrinData(const type & v) : data(cast(v)) {}
		IntrinData(const inner & data) : data(data) {}

		void * operator new (size_t size)
		{
			return _mm_malloc(size, sizeof(T) * 4);
		}

		IntrinData & operator = (const type & v)
		{
			this->data = cast(v);
			return *this;
		}

		T & operator [] (size_t index)
		{
			return data[index];
		}

		const T & operator [] (size_t index) const
		{
			return data[index];
		}

		T & operator [] (int index)
		{
			return data[index];
		}

		const T & operator [] (int index) const
		{
			return data[index];
		}
		
		operator type () const
		{
			return *reinterpret_cast<const type *>(&data);
		}
		
		static inline const array<T, 4> & cast(const type & in)
		{
			return *reinterpret_cast<const array<T, 4> *>(&in);
		}

		template<int I, useif<(I < 4)>>
		static inline T get(const IntrinData & in)
		{
			return in.data[I];
		}

		template<int I, useif<(I < 4)>>
		static inline void set(type & out, T value)
		{
			reinterpret_cast<IntrinData *>(&out)->data[I] = value;
		}
	};

	template<class T, size_t N>
	using intrin_data = IntrinData<T, N>;
}

//---------------------------------------------------------------------------
#endif
