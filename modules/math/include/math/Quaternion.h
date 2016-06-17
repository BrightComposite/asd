//---------------------------------------------------------------------------

#ifndef QUATERNION_H
#define QUATERNION_H

//---------------------------------------------------------------------------

#include <math/Math.h>
#include <core/intrinsic/Intrinsic.h>

#include "Matrix.h"

//---------------------------------------------------------------------------

namespace Rapture
{
	template<typename T>
	struct alignas(sizeof(T) * 4) Quaternion
	{
		using Data = intrin_data<T, 4>;
		using IntrinType = typename Data::type;
		using Intrin = Intrinsic<T, 4>;
		using VectorType = Vector<T>;

		union
		{
			struct
			{
				T x, y, z, w;
			};

			Data data;

			array<T, 4> q;
			T elements[4];
			VectorType v;
			IntrinType intrinsic;
		};

		member_cast(q, array<T, 4>);
		member_cast(data, Data);
		member_cast(intrinsic, IntrinType);

		Quaternion() : data {VectorType::default} {}
		Quaternion(const Quaternion & q) : data {q.data} {}
		Quaternion(T x, T y, T z, T w) : q {x, y, z, w} {}

		Quaternion(T x, T y, T z)
		{
			fromEuler({x, y, z});
		}

		Quaternion(const VectorType & v) : v {v} {}
		Quaternion(VectorType && v) : v {std::forward<VectorType>(v)} {}

		Quaternion(const Data & data) : data {data} {}
		Quaternion(Data && data) : data {std::forward<Data>(data)} {}

		template<class U, useif <!is_same<T, U>::value> endif>
		Quaternion(const Quaternion<U> & q) : v(q.v) {}

		template<class U, useif <can_cast<U, Quaternion>::value> endif>
		Quaternion(const U & v)
		{
			Cast<U, Quaternion>::cast(*this, v);
		}

		Quaternion(const VectorType & axis, T angle) : Quaternion(VectorMath<T>::trigon(angle * 0.5f).template shuffle<0, 0, 0, 1>() * axis.template blend<0, 0, 0, 1>(VectorType::default)) {}
		//																											[ s  s  s  c ]						[ x  y  z  1 ]

		Quaternion & operator = (const Quaternion & q)
		{
			data = q.data;
			return *this;
		}

		template<class U, useif <can_cast<U, Quaternion>::value> endif>
		Quaternion & operator = (const U & q)
		{
			Cast<U, Quaternion>::cast(*this, q);
			return *this;
		}

		bool operator == (const Quaternion & q) const
		{
			return v == q.v;
		}

		bool operator != (const Quaternion & q) const
		{
			return v != q.v;
		}

		Quaternion operator + () const
		{
			return *this;
		}

		Quaternion operator - () const
		{
			return Intrin::negate(data);
		}

		Quaternion & set(const T * v)
		{
			Intrin::load(v, data);
			return *this;
		}

		Quaternion & set(T x, T y, T z, T w)
		{
			Intrin::load(x, y, z, w, data);
			return *this;
		}

		Quaternion & negate()
		{
			Intrin::negate(data, data);
			return *this;
		}

		Quaternion negation() const
		{
			return Intrin::negate(data);
		}

		Quaternion & conjugate()
		{
			v = v.negate<1, 1, 1, 0>();
			return *this;
		}

		Quaternion conjugation() const
		{
			return v.negate<1, 1, 1, 0>();
		}

		Quaternion & invert()
		{
			return conjugate().downscale(norm());
		}

		Quaternion inverse() const
		{
			return conjugation().downscale(norm());
		}

		Quaternion & operator += (const Quaternion & q)
		{
			v += q.v;
			return *this;
		}

		Quaternion & operator -= (const Quaternion & q)
		{
			v -= q.v;
			return *this;
		}
		
		Quaternion & rotateBy(const Quaternion & q)
		{
			return *this = q * *this;
		}

		VectorType applyTo(const VectorType & d) const
		{
			const auto xyz = v.clearW();
			const auto t = 2 * cross(xyz, d);
			return d + w * t + cross(xyz, t);

			//return std::move((*this * Quaternion(d) * inverse()).v);
		}

		VectorType left() const
		{
			return applyTo(VectorType::left);
		}

		VectorType up() const
		{
			return applyTo(VectorType::up);
		}

		VectorType forward() const
		{
			return applyTo(VectorType::forward);
		}

		Quaternion & rotate(const VectorType & axis, T angle)
		{
			return rotateBy({axis, angle});
		}

		Quaternion & rotateX(T angle)
		{
			return rotate(VectorType::positiveX, angle);
		}

		Quaternion & rotateY(T angle)
		{
			return rotate(VectorType::positiveY, angle);
		}

		Quaternion & rotateZ(T angle)
		{
			return rotate(VectorType::positiveZ, angle);
		}

		Quaternion & scale(T s)
		{
			v *= s;
			return *this;
		}

		Quaternion & downscale(T s) &
		{
			v /= s;
			return *this;
		}

		Quaternion && downscale(T s) &&
		{
			v /= s;
			return std::forward<Quaternion>(*this);
		}

		T & operator [] (size_t index)
		{
			return q[index];
		}

		const T & operator [] (size_t index) const
		{
			return q[index];
		}

		T norm() const
		{
			return v.sqr().sum();  
		}

		T magnitude() const
		{
			return Math<T>::sqrt(norm());  
		}

		Quaternion & fromEuler(const VectorType & angles)
		{
			auto halfs = angles * VectorType::half;

			VectorType sine, cosine;
			VectorMath<T>::sincos(halfs, sine, cosine);

			auto r = sine.template shuffle<0, 0, 0, 0>(cosine);
			auto p = sine.template shuffle<1, 1, 1, 1>(cosine);
			auto y = sine.template shuffle<2, 2, 2, 2>(cosine);

			v = r.template shuffle<0, 2, 2, 2>() * p.template shuffle<2, 0, 2, 2>() * y.template shuffle<2, 2, 0, 2>() + r.template shuffle<2, 0, 0, 0>().template negate<0, 1, 0, 1>() * p.template shuffle<0, 2, 0, 0>() * y.template shuffle<0, 0, 2, 0>();
			return *this;
		}

		Matrix<T> toMatrix() const
		{
			VectorType s = -v.sqr();
			s = s.template shuffle<1, 2, 0, 3>() + s.template shuffle<2, 0, 1, 3>();
			VectorType a = v.template shuffle<1, 2, 0, 3>() * v.template shuffle<2, 0, 1, 3>();
			VectorType b = v * v.spreadW().template negate<1, 0, 0, 0>();

			return {
				VectorType::positiveX + VectorType::twoXYZ * s.template blend<0, 1, 1, 0>((a + b.template negate<0, 0, 1, 0>()).template shuffle<2, 2, 1, 1>()),	// 1.0f - 2.0f * (y * y + z * z),	2.0f * (x * y - w * z),			2.0f * (x * z + w * y),			0.0f
				VectorType::positiveY + VectorType::twoXYZ * s.template blend<1, 0, 1, 0>((a + b).template shuffle<2, 2, 0, 0>()),									// 2.0f * (x * y + w * z),			1.0f - 2.0f * (x * x + z * z),	2.0f * (y * z - w * x),			0.0f
				VectorType::positiveZ + VectorType::twoXYZ * (a - b).template shuffle<1, 0, 2, 3>(s),																// 2.0f * (x * z - w * y),			2.0f * (y * z + w * x),			1.0f - 2.0f * (x * x + y * y),	0.0f
				VectorType::positiveW
			};
		}

		void toMatrix(Matrix<T> & m) const
		{
			VectorType s = -v.sqr();
			s = s.template shuffle<1, 2, 0, 3>() + s.template shuffle<2, 0, 1, 3>();
			VectorType a = v.template shuffle<1, 2, 0, 3>() * v.template shuffle<2, 0, 1, 3>();
			VectorType b = v * v.spreadW().template negate<1, 0, 0, 0>();

			m[0] = VectorType::positiveX + VectorType::twoXYZ * s.template blend<0, 1, 1, 0>((a + b.template negate<0, 0, 1, 0>()).template shuffle<2, 2, 1, 1>()); // 1.0f - 2.0f * (y * y + z * z),	2.0f * (x * y - w * z),			2.0f * (x * z + w * y),			0.0f
			m[1] = VectorType::positiveY + VectorType::twoXYZ * s.template blend<1, 0, 1, 0>((a + b).template shuffle<2, 2, 0, 0>());								// 2.0f * (x * y + w * z),			1.0f - 2.0f * (x * x + z * z),	2.0f * (y * z - w * x),			0.0f
			m[2] = VectorType::positiveZ + VectorType::twoXYZ * (a - b).template shuffle<1, 0, 2, 3>(s);															// 2.0f * (x * z - w * y),			2.0f * (y * z + w * x),			1.0f - 2.0f * (x * x + y * y),	0.0f
			m[3] = VectorType::positiveW;
		}

		static const Quaternion api(math) default;
	};

	using FloatQuaternion = Quaternion<float>;
	using DoubleQuaternion = Quaternion<double>;
	using fquat = Quaternion<float>;
	using dquat = Quaternion<double>;

	template<class T>
	using Rotation = Quaternion<T>;

	template<class T>
	Quaternion<T> operator + (const Quaternion<T> & q1, const Quaternion<T> & q2)
	{
		return q1.v + q2.v;
	}

	template<class T>
	Quaternion<T> operator - (const Quaternion<T> & q1, const Quaternion<T> & q2)
	{
		return q1.v - q2.v;
	}

	template<class T>
	Quaternion<T> operator * (const Quaternion<T> & q2, const Quaternion<T> & q1)
	{ 
		//q2.w*q1.x + q2.x*q1.w + q2.y*q1.z - q2.z*q1.y                        
		//q2.w*q1.y + q2.y*q1.w + q2.z*q1.x - q2.x*q1.z
		//q2.w*q1.z + q2.z*q1.w + q2.x*q1.y - q2.y*q1.x
		//q2.w*q1.w - q2.x*q1.x - q2.y*q1.y - q2.z*q1.z

		const Vector<T> & v1 = q1.v;
		const Vector<T> & v2 = q2.v;

		return {
			v2.spreadW() * v1 +
			v2.template shuffle<0, 1, 2, 0>() * v1.template shuffle<3, 3, 3, 0>().negateW() +
			v2.template shuffle<1, 2, 0, 1>() * v1.template shuffle<2, 0, 1, 1>().negateW() -
			v2.template shuffle<2, 0, 1, 2>() * v1.template shuffle<1, 2, 0, 2>()
		};
	}
}

//---------------------------------------------------------------------------
#endif
