//---------------------------------------------------------------------------

#pragma once

#ifndef VECTOR_H
#define VECTOR_H

//---------------------------------------------------------------------------

#include <math/Math.h>
#include <core/intrinsic/Intrinsic.h>
#include <core/String.h>
#include <core/memory/Storage.h>
#include <core/memory/allocator/AlignedAllocator.h>

//---------------------------------------------------------------------------

namespace Rapture
{
	template<typename T>
	struct Vector;

	template<typename T, size_t Mask>
	using VectorMaskAxis = IntrinsicMask<T, IntrinMax, Mask>;

	template<typename T, size_t Mask>
	using VectorSignAxis = IntrinsicMask<T, IntrinSignmask, Mask>;

	template<class T>
	struct VectorMath : Math<Vector<T>> {};

	template<class T>
	using VectorCfs = Cfs<Vector<T>>;

	template<class T>
	using VectorConstants = MathConstants<Vector<T>>;

	template<typename T>
	struct alignas(sizeof(T) * 4) Vector : AlignedAllocator
	{
		using Data = intrin_data<T, 4>;
		using IntrinType = typename Data::type;
		using Intrin = Intrinsic<T, 4>;

		union
		{
			struct
			{
				T x, y, z, w;
			};

			IntrinType intrinsic;

			array<T, 4> v;
			T elements[4];
		};

		member_cast(v, array<T, 4>);
		member_cast(intrinsic, IntrinType);

		Vector() : intrinsic(identity) {}
		Vector(const Vector & v) : intrinsic(v) {}
		Vector(const IntrinType & v) : intrinsic(v) {}

		template<class U, useif<!is_same<T, U>::value>>
		Vector(const Vector<U> & v) : intrinsic(intrin_cvt<IntrinType>(v)) {}

		template<class U, useif<can_cast<U, Vector>::value>>
		Vector(const U & v)
		{
			Cast<U, Vector>::cast(*this, v);
		}

		Vector(const T * v) : intrinsic(Intrin::load(v)) {}
		Vector(T value) : intrinsic(Intrin::fill(value)) {}
		Vector(T x, T y, T z, T w = 0) : intrinsic(Intrin::load(x, y, z, w)) {}

		Vector & operator = (const Vector & v)
		{
			intrinsic = v;
			return *this;
		}

		Vector & operator = (const IntrinType & v)
		{
			intrinsic = v;
			return *this;
		}

		template<class U, useif<can_cast<U, Vector>::value>>
		Vector & operator = (const U & v)
		{
			Cast<U, Vector>::cast(*this, v);
			return *this;
		}

		Vector & set(const T * v)
		{
			intrinsic = Intrin::load(v);
			return *this;
		}

		Vector & set(T x, T y, T z, T w)
		{
			intrinsic = Intrin::load(x, y, z, w);
			return *this;
		}

		Vector & fill(T value)
		{
			intrinsic = Intrin::fill(value);
			return *this;
		}

		Vector operator + () const
		{
			return intrinsic;
		}

		Vector operator - () const
		{
			return Intrin::negate(intrinsic);
		}

		Vector & abs()
		{
			Intrin::abs(intrinsic, intrinsic);
			return *this;
		}

		Vector & round()
		{
			Intrin::round(intrinsic, intrinsic);
			return *this;
		}

		Vector & invert()
		{
			Intrin::invert(intrinsic, intrinsic);
			return *this;
		}

		Vector rounded() const
		{
			return Intrin::round(intrinsic);
		}

		Vector inverse() const
		{
			return Intrin::invert(intrinsic);
		}

		void getRounded(Vector & vec) const
		{
			Intrin::round(intrinsic, vec.intrinsic);
		}

		void getInverse(Vector & vec) const
		{
			Intrin::invert(intrinsic, vec.intrinsic);
		}

		Vector & operator += (const Vector & v)
		{
			Intrin::add(intrinsic, v, intrinsic);
			return *this;
		}

		Vector & operator -= (const Vector & v)
		{
			Intrin::sub(intrinsic, v, intrinsic);
			return *this;
		}

		Vector & operator *= (const Vector & v)
		{
			Intrin::mul(intrinsic, v, intrinsic);
			return *this;
		}

		Vector & operator /= (const Vector & v)
		{
			Intrin::div(intrinsic, v, intrinsic);
			return *this;
		}

		Vector & operator += (T value)
		{
			Intrin::add(intrinsic, Intrin::fill(value), intrinsic);
			return *this;
		}

		Vector & operator -= (T value)
		{
			Intrin::sub(intrinsic, Intrin::fill(value), intrinsic);
			return *this;
		}

		Vector & operator *= (T k)
		{
			Intrin::mul(intrinsic, Intrin::fill(k), intrinsic);
			return *this;
		}

		Vector & operator /= (T k)
		{
			if(k == 0)
				return *this = VectorMath<T>::infinity;

			Intrin::div(intrinsic, Intrin::fill(k), intrinsic);
			return *this;
		}

		Vector operator & (const Vector & v) const
		{
			return Intrin::bit_and(intrinsic, v);
		}

		Vector operator | (const Vector & v) const
		{
			return Intrin::bit_or(intrinsic, v);
		}

		Vector operator ^ (const Vector & v) const
		{
			return Intrin::bit_xor(intrinsic, v);
		}

		Vector dot(const Vector & v) const
		{
			return Intrin::fillsum(Intrin::mul(intrinsic, v));
		}

		T dot1(const Vector & v) const
		{
			return Intrin::sum(Intrin::mul(intrinsic, v));
		}

		Vector cross(const Vector & v) const
		{
			return Intrin::sub(
				Intrin::mul(Intrin::template shuffle<1, 2, 0, 3>(intrinsic), Intrin::template shuffle<2, 0, 1, 3>(v)),
				Intrin::mul(Intrin::template shuffle<2, 0, 1, 3>(intrinsic), Intrin::template shuffle<1, 2, 0, 3>(v))
			);
		}

		bool operator == (const Vector & v) const
		{
			return Intrin::equal(intrinsic, v);
		}

		bool operator != (const Vector & v) const
		{
			return Intrin::notequal(intrinsic, v);
		}

		T & operator [] (size_t index)
		{
			return v[index];
		}

		const T & operator [] (size_t index) const
		{
			return v[index];
		}

		T & operator [] (int index)
		{
			return v[index];
		}

		const T & operator [] (int index) const
		{
			return v[index];
		}

		T magnitudeSq() const
		{
			return dot1(intrinsic);
		}

		Vector magnitudeSqVector() const
		{
			return dot(intrinsic);
		}

		T magnitude() const
		{
			return std::sqrt(magnitudeSq());
		}

		Vector magnitudeVector() const
		{
			return Intrin::sqrt(magnitudeSqVector());
		}

		Vector & normalize() &
		{
			intrinsic = Intrin::div(intrinsic, magnitudeVector());
			return *this;
		}

		Vector && normalize() &&
		{
			intrinsic = Intrin::div(intrinsic, magnitudeVector());
			return std::forward<Vector>(*this);
		}

		Vector normalized() const
		{
			return Intrin::div(intrinsic, Intrin::fill(magnitude()));
		}

		inline T max() const
		{
			Vector v = Intrin::abs(intrinsic);
			return std::max({v.x, v.y, v.z});
		}

		inline Vector abs() const
		{
			return Intrin::abs(intrinsic);
		}

		inline Vector sqr() const
		{
			return Intrin::sqr(intrinsic);
		}

		inline T sum() const
		{
			return Intrin::sum(intrinsic);
		}

		inline T distanceToSq(const Vector & p) const
		{
			return Intrin::sum(Intrin::sqr(Intrin::sub(intrinsic, p)));
		}

		inline auto distanceTo(const Vector & p) const
		{
			return std::sqrt(distanceToSq(p));
		}

		inline T distanceToAxis(const Vector & p, int axis) const
		{
			return std::abs(v[axis] - p[axis]);
		}

		inline int getMainAxis() const
		{
			return x >= y ? 0 : y >= z ? 1 : 2;
		}

		template<byte X, byte Y, byte Z, byte W, useif<(X < 2 && Y < 2 && Z < 2 && W < 2)>>
		Vector mask() const // select some components (e.g. if X == 1 then result.x = v.x else result.x = 0)
		{
			return Intrin::bit_and(VectorMaskAxis<T, mk_mask4(X, Y, Z, W)>::get(), intrinsic);
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector maskAxis() const // set all components of a array_list to zero except of one
		{
			return Intrin::bit_and(VectorMaskAxis<T, bitmask<Axis>::value>::get(), intrinsic);
		}

		Vector maskX() const
		{
			return maskAxis<0>();
		}

		Vector maskY() const
		{
			return maskAxis<1>();
		}

		Vector maskZ() const
		{
			return maskAxis<2>();
		}

		Vector maskW() const
		{
			return maskAxis<3>();
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector clearAxis() const // set a single component to zero
		{
			return Intrin::bit_and(VectorMaskAxis<T, 0xF ^ bitmask<Axis>::value>::get(), intrinsic);
		}

		Vector clearX() const
		{
			return clearAxis<0>();
		}

		Vector clearY() const
		{
			return clearAxis<1>();
		}

		Vector clearZ() const
		{
			return clearAxis<2>();
		}

		Vector clearW() const
		{
			return clearAxis<3>();
		}

		template<byte X, byte Y, byte Z, byte W, useif<(X < 2 && Y < 2 && Z < 2 && W < 2)>>
		Vector negate() const // negate some components (e.g. if X == 1 then result.x = -v.x else result.x = v.x)
		{
			return Intrin::bit_xor(VectorSignAxis<T, mk_mask4(X, Y, Z, W)>::get(), intrinsic);
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector negateAxis() const // negate one component
		{
			return Intrin::bit_xor(VectorSignAxis<T, bitmask<Axis>::value>::get(), intrinsic);
		}

		Vector negateX() const
		{
			return negateAxis<0>();
		}

		Vector negateY() const
		{
			return negateAxis<1>();
		}

		Vector negateZ() const
		{
			return negateAxis<2>();
		}

		Vector negateW() const
		{
			return negateAxis<3>();
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector spreadAxis() const // get a array_list filled with a single component of a src array_list
		{
			return shuffle<Axis, Axis, Axis, Axis>();
		}

		Vector spreadX() const
		{
			return spreadAxis<0>();
		}

		Vector spreadY() const
		{
			return spreadAxis<1>();
		}

		Vector spreadZ() const
		{
			return spreadAxis<2>();
		}

		Vector spreadW() const
		{
			return spreadAxis<3>();
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector & addAxis(const Vector & vec)
		{
			v += vec.maskAxis<Axis>();
			return *this;
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector & subtractAxis(const Vector & vec)
		{
			v -= vec.maskAxis<Axis>();
			return *this;
		}

		Vector & addAxis(const Vector & vec, int Axis)
		{
			v[Axis] += vec[Axis];
			return *this;
		}

		Vector & subtractAxis(const Vector & vec, int Axis)
		{
			v[Axis] -= vec[Axis];
			return *this;
		}

		Vector & addAxis(T val, int Axis)
		{
			v[Axis] += val;
			return *this;
		}

		Vector & subtractAxis(T val, int Axis)
		{
			v[Axis] -= val;
			return *this;
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector & addAxis(T val)
		{
			v[Axis] += val;
			return *this;
		}

		template<uint Axis, useif<(Axis < 4)>>
		Vector & subtractAxis(T val)
		{
			v[Axis] -= val;
			return *this;
		}

		Vector & clamp(T low, T high)
		{
			Math<T>::clamp(x, low, high);
			Math<T>::clamp(y, low, high);
			Math<T>::clamp(z, low, high);

			return *this;
		}

		Vector & clamp(const Vector & low, const Vector & high)
		{
			intrinsic = Intrin::min(Intrin::max(intrinsic, low), high);
			return *this;
		}

		template<byte A, byte B, byte C, byte D, useif<(A < 4 && B < 4 && C < 4 && D < 4)>>
		inline Vector shuffle() const
		{
			return Intrin::template shuffle<A, B, C, D>(intrinsic);
		}

		template<byte A, byte B, byte C, byte D, useif<(A < 4 && B < 4 && C < 4 && D < 4)>>
		inline Vector shuffle(const Vector & v) const
		{
			return Intrin::template shuffle2<A, B, C, D>(intrinsic, v);
		}

		template<byte A, byte B, byte C, byte D, useif<(A < 2 && B < 2 && C < 2 && D < 2)>>
		inline Vector blend(const Vector & v) const
		{
			return Intrin::template blend<A, B, C, D>(intrinsic, v);
		}

		static inline Vector minimum(const Vector & v1, const Vector & v2)
		{
			return Intrin::min(v1, v2);
		}

		static inline Vector maximum(const Vector & v1, const Vector & v2)
		{
			return Intrin::max(v1, v2);
		}

		static inline int compare(const Vector & v1, const Vector & v2)
		{
			return static_cast<int>(Intrin::sum(Intrin::sub(v1, v2)));
		}

		static api(math) const Vector zero;			// [  0,  0,  0,  0 ]
		static api(math) const Vector one;			// [  1,  1,  1,  1 ]
		static api(math) const Vector two;			// [  2,  2,  2,  2 ]
		static api(math) const Vector oneXYZ;		// [  1,  1,  1,  0 ]
		static api(math) const Vector twoXYZ;		// [  2,  2,  2,  0 ]
		static api(math) const Vector minusOne;		// [ -1, -1, -1, -1 ]
		static api(math) const Vector half;			// [ .5, .5, .5, .5 ]

		static api(math) const Vector positiveX;	// [  1,  0,  0,  0 ]
		static api(math) const Vector positiveY;	// [  0,  1,  0,  0 ]
		static api(math) const Vector positiveZ;	// [  0,  0,  1,  0 ]
		static api(math) const Vector positiveW;	// [  0,  0,  0,  1 ]
		static api(math) const Vector negativeX;	// [ -1,  0,  0,  0 ]
		static api(math) const Vector negativeY;	// [  0, -1,  0,  0 ]
		static api(math) const Vector negativeZ;	// [  0,  0, -1,  0 ]
		static api(math) const Vector negativeW;	// [  0,  0,  0, -1 ]

		static api(math) const Vector & right;
		static api(math) const Vector & up;
		static api(math) const Vector & forward;
		static api(math) const Vector & identity;
	};

	using ByteVector = Vector<byte>;
	using IntVector = Vector<int>;
	using FloatVector = Vector<float>;

	using bytev = ByteVector;
	using intv = IntVector;
	using floatv = FloatVector;

	template<class T>
	using svec = Storage<Vector<T>>;
	using bvec = Storage<ByteVector>;
	using ivec = Storage<IntVector>;
	using fvec = Storage<FloatVector>;

#ifdef USE_AVX
	using DoubleVector = Vector<double>;
	using dvec = DoubleVector;
#endif

	template<class T>
	struct BasicMath<Vector<T>, false>
	{
		typedef Intrinsic<T, 4> Intrin;

		static inline Vector<T> abs(const Vector<T> & x)
		{
			return Intrin::abs(x);
		}

		static inline Vector<T> sqr(const Vector<T> & x)
		{
			return Intrin::sqr(x);
		}

		static inline Vector<T> sqrt(const Vector<T> & x)
		{
			return Intrin::sqrt(x);
		}

		static inline Vector<T> avg(const Vector<T> & x, const Vector<T> & y)
		{
			return Vector<T>::half * (x + y);
		}

		static inline void clamp(Vector<T> & x, const Vector<T> & low, const Vector<T> & high)
		{
			x.clamp(low, high);
		}

		static inline Vector<T> invert(const Vector<T> & v)
		{
			return v.inverse();
		}

		static inline Vector<T> round(const Vector<T> & v)
		{
			return v.rounded();
		}
	};

	template<class T>
	struct SinCos<Vector<T>>
	{
		using Intrin = Intrinsic<T, 4>;

		static inline Vector<T> sin(const Vector<T> & angle)
		{
			Vector<T> s;
			sin(angle, s);
			return s;
		}

		static inline Vector<T> cos(const Vector<T> & angle)
		{
			Vector<T> c;
			cos(angle, c);
			return c;
		}

		static inline void sin(const Vector<T> & angle, Vector<T> & s)
		{
			auto x = VectorMath<T>::rmod(angle);
			auto sign = Intrin::sign(x);
			auto comp = Intrin::cmple(Intrin::bit_andnot(sign, x), VectorMath<T>::half_pi);

			s = Intrin::bit_or(Intrin::bit_and(comp, x), Intrin::bit_andnot(comp, Intrin::sub(Intrin::bit_or(VectorMath<T>::pi, sign), x)));
			x = Intrin::sqr(s);

			s *=
				Vector<T>::one + x * (
					VectorCfs<T>::sin[0] + x * (
						VectorCfs<T>::sin[1] + x * (
							VectorCfs<T>::sin[2] + x * (
								VectorCfs<T>::sin[3] + x * (
									VectorCfs<T>::sin[4])))));
		}

		static inline void cos(const Vector<T> & angle, Vector<T> & c)
		{
			auto x = VectorMath<T>::rmod(angle);
			auto sign = Intrin::sign(x);
			auto comp = Intrin::cmple(Intrin::bit_andnot(sign, x), VectorMath<T>::half_pi);

			x = Intrin::sqr(Intrin::bit_or(Intrin::bit_and(comp, x), Intrin::bit_andnot(comp, Intrin::sub(Intrin::bit_or(VectorMath<T>::pi, sign), x))));

			c = Intrin::bit_or(Intrin::bit_and(comp, Vector<T>::one), Intrin::bit_andnot(comp, Vector<T>::minusOne)) * (
				Vector<T>::one + x * (
					VectorCfs<T>::cos[0] + x * (
						VectorCfs<T>::cos[1] + x * (
							VectorCfs<T>::cos[2] + x * (
								VectorCfs<T>::cos[3] + x * (
									VectorCfs<T>::cos[4]))))));
		}

		static inline void sincos(const Vector<T> & angle, Vector<T> & s, Vector<T> & c)
		{
			auto x = VectorMath<T>::rmod(angle);
			auto sign = Intrin::sign(x);
			auto comp = Intrin::cmple(Intrin::bit_andnot(sign, x), VectorMath<T>::half_pi);

			s = Intrin::bit_or(Intrin::bit_and(comp, x), Intrin::bit_andnot(comp, Intrin::sub(Intrin::bit_or(VectorMath<T>::pi, sign), x)));
			x = Intrin::sqr(s);

			s *=
				Vector<T>::one + x * (
					VectorCfs<T>::sin[0] + x * (
						VectorCfs<T>::sin[1] + x * (
							VectorCfs<T>::sin[2] + x * (
								VectorCfs<T>::sin[3] + x * (
									VectorCfs<T>::sin[4])))));

			c = Intrin::bit_or(Intrin::bit_and(comp, Vector<T>::one), Intrin::bit_andnot(comp, Vector<T>::minusOne)) * (
				Vector<T>::one + x * (
					VectorCfs<T>::cos[0] + x * (
						VectorCfs<T>::cos[1] + x * (
							VectorCfs<T>::cos[2] + x * (
								VectorCfs<T>::cos[3] + x * (
									VectorCfs<T>::cos[4]))))));
		}

		// returns [ sin(angle), cos(angle), -sin(angle), 0 ]
		static inline Vector<T> trigon(T angle)
		{
			return sin(Vector<T>{angle, angle + Math<T>::half_pi, -angle, 0});
		}
	};

	using FloatVectorMath = VectorMath<float>;
	using FloatVectorCfs = VectorCfs<float>;
	using FloatVectorConstants = VectorConstants<float>;

#ifdef USE_AVX
	using DoubleVectorMath = VectorMath<double>;
	using DoubleVectorCfs = VectorCfs<double>;
	using DoubleVectorConstants = VectorConstants<double>;
#endif

	inline Vector<float> vec()
	{
		return Vector<float>::zero;
	}

	inline Vector<float> vec(float x, float y, float z)
	{
		return {x, y, z, 1};
	}

	inline Vector<float> vec(float x, float y, float z, float w)
	{
		return {x, y, z, w};
	}

#ifdef USE_AVX
	inline Vector<double> vecd()
	{
		return Vector<double>::zero;
	}

	inline Vector<double> vecd(double x, double y, double z)
	{
		return {x, y, z, 1};
	}

	inline Vector<double> vecd(double x, double y, double z, double w)
	{
		return {x, y, z, w};
	}
#endif

	template<typename T>
	inline Vector<T> operator + (const Vector<T> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::add(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator - (const Vector<T> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::sub(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator * (const Vector<T> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::mul(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator / (const Vector<T> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::div(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator + (const Vector<T> & v1, const intrin_t<T, 4> & v2)
	{
		return Intrinsic<T, 4>::add(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator - (const Vector<T> & v1, const intrin_t<T, 4> & v2)
	{
		return Intrinsic<T, 4>::sub(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator * (const Vector<T> & v1, const intrin_t<T, 4> & v2)
	{
		return Intrinsic<T, 4>::mul(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator / (const Vector<T> & v1, const intrin_t<T, 4> & v2)
	{
		return Intrinsic<T, 4>::div(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator + (const Vector<T> & v1, const IntrinData<T, 4> & v2)
	{
		return Intrinsic<T, 4>::add(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator - (const Vector<T> & v1, const IntrinData<T, 4> & v2)
	{
		return Intrinsic<T, 4>::sub(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator * (const Vector<T> & v1, const IntrinData<T, 4> & v2)
	{
		return Intrinsic<T, 4>::mul(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator / (const Vector<T> & v1, const IntrinData<T, 4> & v2)
	{
		return Intrinsic<T, 4>::div(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator + (const intrin_t<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::add(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator - (const intrin_t<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::sub(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator * (const intrin_t<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::mul(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator / (const intrin_t<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::div(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator + (const IntrinData<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::add(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator - (const IntrinData<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::sub(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator * (const IntrinData<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::mul(v1, v2);
	}

	template<typename T>
	inline Vector<T> operator / (const IntrinData<T, 4> & v1, const Vector<T> & v2)
	{
		return Intrinsic<T, 4>::div(v1, v2);
	}

	template<typename T, typename U, useif<std::is_pod<U>>>
	inline Vector<T> operator + (const Vector<T> & vec, U a)
	{
		return Intrinsic<T, 4>::add(vec, Intrinsic<T, 4>::fill(static_cast<T>(a)));
	}

	template<typename T, typename U, useif<std::is_pod<U>>>
	inline Vector<T> operator - (const Vector<T> & vec, U a)
	{
		return Intrinsic<T, 4>::sub(vec, Intrinsic<T, 4>::fill(static_cast<T>(a)));
	}

	template<typename T, typename U, useif<std::is_pod<U>>>
	inline Vector<T> operator * (const Vector<T> & vec, U a)
	{
		return Intrinsic<T, 4>::mul(vec, Intrinsic<T, 4>::fill(static_cast<T>(a)));
	}

	template<typename T, typename U, useif<std::is_pod<U>::value>>
	inline Vector<T> operator / (const Vector<T> & vec, U a)
	{
		return Intrinsic<T, 4>::div(vec, Intrinsic<T, 4>::fill(static_cast<T>(a)));
	}

	template<typename T, typename U, useif<std::is_pod<U>::value>>
	inline Vector<T> operator * (U a, const Vector<T> & vec)
	{
		return Intrinsic<T, 4>::mul(Intrinsic<T, 4>::fill(static_cast<T>(a)), vec.intrinsic);
	}

	template<typename T, typename U, useif<std::is_pod<U>::value>>
	inline Vector<T> operator / (U a, const Vector<T> & vec)
	{
		return Intrinsic<T, 4>::div(Intrinsic<T, 4>::fill(static_cast<T>(a)), vec.intrinsic);
	}

	template<typename T, typename U>
    inline auto lerp(const Vector<T> & a, const Vector<T> & b, U t)
    {
        return a + static_cast<T>(t) * (b - a);
    }

	template<typename T>
	inline T sum(const Vector<T> & v)
	{
		return v.sum();
	}

	template<typename T>
	inline T dot(const Vector<T> & v1, const Vector<T> & v2)
	{
		return v1.dot1(v2);
	}

	template<typename T>
	inline auto cross(const Vector<T> & v1, const Vector<T> & v2)
	{
		return v1.cross(v2);
	}

	template<typename T>
	inline auto mixedProduct(const Vector<T> & a, const Vector<T> & b, const Vector<T> & c)
	{
		return dot(a, cross(b, c));
	}

	template<typename T>
	inline bool areCollinear(const Vector<T> & a, const Vector<T> & b, const Vector<T> & c)
	{
		return cross(b - a, c - a).magnitudeSq() < Math<T>::eps2;
	}

	template<typename T>
	inline bool areComplanar(const Vector<T> & a, const Vector<T> & b, const Vector<T> & c, const Vector<T> & d)
	{
		return Math<T>::abs(mixedProduct(b - a, c - a, d - a)) < Math<T>::eps3;
	}

	inline FloatVector operator "" _v(long double v)
	{
		return {static_cast<float>(v)};
	}

#ifdef USE_AVX
	inline DoubleVector operator "" _vd(long double v)
	{
		return {static_cast<double>(v)};
	}
#endif

	template<class T>
	inline void print(String & s, const Vector<T> & v)
	{
		s << String::assemble("(", v.x, ", ", v.y, ", ", v.z, ", ", v.w, ")");
	}

	template<class T>
	inline void print(String & s, const Storage<Vector<T>> & v)
	{
		s << String::assemble("(", v->x, ", ", v->y, ", ", v->z, ", ", v->w, ")");
	}

	inline void print(String & s, const intrin_t<float, 4> & v)
	{
		s << String::assemble(
			"(",
			IntrinData<float, 4>::get<0>(v), ", ",
			IntrinData<float, 4>::get<1>(v), ", ",
			IntrinData<float, 4>::get<2>(v), ", ",
			IntrinData<float, 4>::get<3>(v), ")"
			);
	}

#ifdef USE_AVX
	inline void print(String & s, const intrin_t<double, 4> & v)
	{
		s << String::assemble(
			"(",
			IntrinData<double, 4>::get<0>(v), ", ",
			IntrinData<double, 4>::get<1>(v), ", ",
			IntrinData<double, 4>::get<2>(v), ", ",
			IntrinData<double, 4>::get<3>(v), ")"
			);
	}
#endif
}

//---------------------------------------------------------------------------
#endif
