/*
								+----------------------------------+
								|                                  |
								|       ***  Vector math  ***      |
								|                                  |
								|   Copyright © -tHE SWINe- 2005   |
								|                                  |
								|             Vector.h             |
								|                                  |
								+----------------------------------+
*/

/*
 *	passed code revision at 2006-05-16
 *
 *	optimized some parts of code
 *	all integers storing just true of false were replaced with bool
 *
 *	passed code revision at 2006-07-02
 *
 *	added swizzle operators to vector classes
 *	added redlect / refract / fresnel functions to Vector3<T> class
 *	renamed Vector3<T>::Len to Vector3<T>::f_Length, Vector3<T>::Len2 to Vector3<T>::f_Length2
 *	added some Matrix3f methods, renamed Matrix3f::Init() to Matrix3f::Identity()
 *	renamed Planef to Plane3f
 *	renamed Pi to f_pi
 *	general cleanup of code
 *
 *	2006-07-17
 *
 *	renamed CPolygon::r_Vertex(int) to CPolygon::r_t_Vertex(int)
 *	renamed CPolygon::r_Normal() to CPolygon::r_t_Normal()
 *
 *	2006-07-20
 *
 *	added CPolygon::b_MT_RayHit_CullFrontfaces and CPolygon::b_MT_RayHit_CullBackFaces
 *
 *	2006-07-24
 *
 *	renamed TVertexStruct to TVertex3f
 *	added class CVectorMath with ray distances and bounding box / line (ray) intersection tests
 *
 *	2007-01-23
 *
 *	fixed std::vector handling in template CPolygon<TVertex3f> (not-enough-memory checks)
 *	remade TVertex3f base class and CPolygon<TVertex3f> a bit (added + and * operators,
 *	renamed Lerp() to t_Lerp())
 *
 *	2007-03-07
 *
 *	added complete swizzling functions set to vectors
 *
 *	2007-06-04
 *
 *	added missing const keyword to input swizzle functions
 *
 *	2007-08-03
 *
 *	added quaternion class with basic quaternion operations
 *
 *	2007-09-17
 *
 *	fixed swizzle*_in macros
 *
 *	2007-10-27
 *
 *	added convenience vector / constant operators and component-wise vector * vector
 *	and vector / vector operators to all vector templates
 *	added Vector4f Matrix4f::operator *(const Vector4f &) function for natural matrix-vertex
 *	multiplication as alternative to v_Transform_Pos() and v_Transform_Dir()
 *
 *	2007-11-26
 *
 *	added Matrix4f::f_Subdet, Matrix4f::f_Determinant, Matrix4f::FullInvert and
 *	Matrix4f::t_FullInverse
 *	added convenience vector + constant and vector - constant operators to all vector templates
 *
 *	2008-05-19
 *
 *	fixed typos in the word 'environment'
 *
 *	2008-08-21
 *
 *	fixed Matrix4f::Scale(), added non-Vector3f variants of Matrix4f::Scale(),
 *	Matrix4f::Translate() and Matrix4f::Rotate()
 *
 *	2008-08-23
 *
 *	documented Matrix4f class, added functions for generating transformation
 *	matrices (not applying transformation on the matrix), namely Matrix4f::Translation(),
 *	Matrix4f::Scaling(), Matrix4f::RotationX(), Matrix4f::RotationY(), Matrix4f::RotationZ(),
 *	and Matrix4f::Rotation(). added component-wise matrix by scalar multiplication (overloads
 *	of Matrix4f::operator *() and Matrix4f::operator *=()) and finaly added transposition
 *	function Matrix4f::Transposition()
 *
 *	2009-05-04
 *
 *	fixed mixed windows / linux line endings
 *
 *	2009-05-12
 *
 *	fixed out-of-bounds index in TObjectOrientedBox::b_Intersect_Ray()
 *
 *	2009-05-23
 *
 *	removed all instances of std::vector::reserve and replaced them by stl_ut::Reserve_*
 *
 *	2009-12-18
 *
 *	removed Plane3f::v_Intersect_Ray(), Plane3f::f_Intersect_Ray_t() in favor of
 *	Plane3f::Intersect_Ray(), Plane3f::Intersect_Ray_t(), the new functions do not throw
 *	exceptions, which is considered better solution here.
 *
 *	added Matrix4f::ProductOf(), Matrix4f::FastInverseTo(), Matrix4f::FullInverseTo()
 *	and Matrix4f::TransposeTo(), which are intended to reduce unnecessary matrix copying
 *	when using overloaded operators (overloaded operators remains unchanged)
 *
 *	renamed Matrix4f::Invert(), Matrix4f::t_Inverse() to Matrix4f::FastInvert(),
 *	Matrix4f::t_FastInverse() respectively
 *
 *	renamed Quat4f::q_*() to Quat4f::t_*() to satisfy naming convention (t_ for struct)
 *
 *	added connecting constructors for vectors, meaning it's possible to write
 *	<tt>Vector3f(Vector2f(1, 2), float(3))</tt> with the same effect, as <tt>Vector3f(1, 2, 3)</tt>.
 *	all combinations are possible,
 *
 *	@date 2010-09-22
 *
 *	cannibalized useful parts of Vector.h from ÜberLame
 *
 */

#ifndef __VECTOR2_INCLUDED
#define __VECTOR2_INCLUDED

#include <math.h>
#include <vector>

#if defined(_MSC_VER) && !defined(__MWERKS__) && !defined(for)
#define for if(0) {} else for
#endif
// msvc 'for' hack

extern const float f_pi; // 3.1415926535897932384626433832795028841971691075f


/*
 *								=== Vector2 ===
 */

template <class T> struct Vector2 {
	T x, y;

	inline Vector2() { }
	inline Vector2(T t_x, T t_y) :x(t_x), y(t_y) { }

	inline T f_Length() const { return (T)sqrt(x * x + y * y); }
	inline T f_Length2() const { return x * x + y * y; }

	inline T operator [](int n_index) const
	{
		if(&x + 1 == &y) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else
				return y;
		}
	}

	inline T &operator [](int n_index)
	{
		if(&x + 1 == &y) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else
				return y;
		}
	}

	inline void Normalize()
	{
		T t = f_Length();
		
		if(t != 0) {
			x /= t;
			y /= t;
		}
	}

	inline void Normalize(T t_len)
	{
		T t = f_Length();

		if(t != 0) {
			t /= t_len;
			x /= t;
			y /= t;
		}
	}

	inline bool operator ==(const Vector2<T> &r_v_vec) const
	{
		return x == r_v_vec.x && y == r_v_vec.y;
	}

	inline Vector2<T> operator +(const Vector2<T> &r_v_vec) const
	{
		return Vector2<T>(x + r_v_vec.x, y + r_v_vec.y);
	}

	inline Vector2<T> operator -(const Vector2<T> &r_v_vec) const
	{
		return Vector2<T>(x - r_v_vec.x, y - r_v_vec.y);
	}

	inline Vector2<T> operator *(const Vector2<T> &r_v_vec) const
	{
		return Vector2<T>(x * r_v_vec.x, y * r_v_vec.y);
	}

	inline Vector2<T> operator /(const Vector2<T> &r_v_vec) const
	{
		return Vector2<T>(x / r_v_vec.x, y / r_v_vec.y);
	}

	inline Vector2<T> operator -() const
	{
		return Vector2<T>(-x, -y);
	}

	inline Vector2<T> operator *(T t) const
	{
		return Vector2<T>(x * t, y * t);
	}

	inline Vector2<T> operator /(T t) const
	{
		return Vector2<T>(x / t, y / t);
	}

	inline Vector2<T> operator +(T t) const
	{
		return Vector2<T>(x + t, y + t);
	}

	inline Vector2<T> operator -(T t) const
	{
		return Vector2<T>(x - t, y - t);
	}

	inline Vector2<T> operator +=(const Vector2<T> &r_v_vec)
	{
		x += r_v_vec.x;
		y += r_v_vec.y;
		return *this;
	}

	inline Vector2<T> operator -=(const Vector2<T> &r_v_vec)
	{
		x -= r_v_vec.x;
		y -= r_v_vec.y;
		return *this;
	}

	inline Vector2<T> operator *=(const Vector2<T> &r_v_vec)
	{
		x *= r_v_vec.x;
		y *= r_v_vec.y;
		return *this;
	}

	inline Vector2<T> operator /=(const Vector2<T> &r_v_vec)
	{
		x /= r_v_vec.x;
		y /= r_v_vec.y;
		return *this;
	}

	inline Vector2<T> operator *=(T t)
	{
		x *= t;
		y *= t;
		return *this;
	}

	inline Vector2<T> operator /=(T t)
	{
		x /= t;
		y /= t;
		return *this;
	}

	inline Vector2<T> operator +=(T t)
	{
		x += t;
		y += t;
		return *this;
	}

	inline Vector2<T> operator -=(T t)
	{
		x -= t;
		y -= t;
		return *this;
	}

	inline T f_Dot(const Vector2<T> &r_v_vec) const
	{
		return x * r_v_vec.x + y * r_v_vec.y;
	}

	inline Vector2<T> v_Orthogonal() const
	{
		return Vector2<T>(-y, x);
	}
};

typedef Vector2<float> Vector2f;
typedef Vector2<int> Vector2i;

/*
 *								=== ~Vector2 ===
 */

/*
 *								=== Vector3 ===
 */

template <class T> struct Vector3 {
	T x, y, z;

	inline Vector3()
	{}

	inline Vector3(Vector2<T> &r_v_vec, T t_z)
		:x(r_v_vec.x), y(r_v_vec.y), z(t_z)
	{}

	inline Vector3(T t_x, Vector2<T> &r_v_vec)
		:x(t_x), y(r_v_vec.x), z(r_v_vec.y)
	{}

	inline Vector3(T t_x, T t_y, T t_z)
		:x(t_x), y(t_y), z(t_z)
	{}

	inline T f_Length() const
	{
		return (T)sqrt(x * x + y * y + z * z);
	}

	inline T f_Length2() const
	{
		return x * x + y * y + z * z;
	}

	inline T operator [](int n_index) const
	{
		if(&x + 1 == &y && &x + 2 == &z) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else if(n_index == 1)
				return y;
			else
				return z;
		}
	}

	inline T &operator [](int n_index)
	{
		if(&x + 1 == &y && &x + 2 == &z) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else if(n_index == 1)
				return y;
			else
				return z;
		}
	}

	inline void Normalize()
	{
		T t = f_Length();
		
		if(t != 0) {
			t = 1 / t;
			x *= t;
			y *= t;
			z *= t;
		}
	}

	inline void Normalize(T t_len)
	{
		T t = f_Length();

		if(t != 0) {
			t = t_len / t;
			x *= t;
			y *= t;
			z *= t;
		}
	}

	inline bool operator ==(const Vector3<T> &r_v_vec) const
	{
		return x == r_v_vec.x && y == r_v_vec.y && z == r_v_vec.z;
	}

	inline Vector3<T> operator +(const Vector3<T> &r_v_vec) const
	{
		return Vector3<T>(x + r_v_vec.x, y + r_v_vec.y, z + r_v_vec.z);
	}

	inline Vector3<T> operator -(const Vector3<T> &r_v_vec) const
	{
		return Vector3<T>(x - r_v_vec.x, y - r_v_vec.y, z - r_v_vec.z);
	}

	inline Vector3<T> operator *(const Vector3<T> &r_v_vec) const
	{
		return Vector3<T>(x * r_v_vec.x, y * r_v_vec.y, z * r_v_vec.z);
	}

	inline Vector3<T> operator /(const Vector3<T> &r_v_vec) const
	{
		return Vector3<T>(x / r_v_vec.x, y / r_v_vec.y, z / r_v_vec.z);
	}

	inline Vector3<T> operator -() const
	{
		return Vector3<T>(-x, -y, -z);
	}

	inline Vector3<T> operator *(T t) const
	{
		return Vector3<T>(x * t, y * t, z * t);
	}

	inline Vector3<T> operator /(T t) const
	{
		t = 1 / t;
		return Vector3<T>(x * t, y * t, z * t);
	}

	inline Vector3<T> operator +(T t) const
	{
		return Vector3<T>(x + t, y + t, z + t);
	}

	inline Vector3<T> operator -(T t) const
	{
		return Vector3<T>(x - t, y - t, z - t);
	}

	inline Vector3<T> operator +=(const Vector3<T> &r_v_vec)
	{
		x += r_v_vec.x;
		y += r_v_vec.y;
		z += r_v_vec.z;
		return *this;
	}

	inline Vector3<T> operator -=(const Vector3<T> &r_v_vec)
	{
		x -= r_v_vec.x;
		y -= r_v_vec.y;
		z -= r_v_vec.z;
		return *this;
	}

	inline Vector3<T> operator *=(const Vector3<T> &r_v_vec)
	{
		x *= r_v_vec.x;
		y *= r_v_vec.y;
		z *= r_v_vec.z;
		return *this;
	}

	inline Vector3<T> operator /=(const Vector3<T> &r_v_vec)
	{
		x /= r_v_vec.x;
		y /= r_v_vec.y;
		z /= r_v_vec.z;
		return *this;
	}

	inline Vector3<T> operator *=(T t)
	{
		x *= t;
		y *= t;
		z *= t;
		return *this;
	}

	inline Vector3<T> operator /=(T t)
	{
		t = 1 / t;
		x *= t;
		y *= t;
		z *= t;
		return *this;
	}

	inline Vector3<T> operator +=(T t)
	{
		x += t;
		y += t;
		z += t;
		return *this;
	}

	inline Vector3<T> operator -=(T t)
	{
		x -= t;
		y -= t;
		z -= t;
		return *this;
	}

	inline T f_Dot(const Vector3<T> &r_v_vec) const // dot product
	{
		return x * r_v_vec.x + y * r_v_vec.y + z * r_v_vec.z;
	}

	inline Vector3<T> v_Cross(const Vector3<T> &r_v_vec) const // cross product
	{
		return Vector3<T>(r_v_vec.y * z - r_v_vec.z * y, r_v_vec.z * x - r_v_vec.x * z, r_v_vec.x * y - r_v_vec.y * x);
	}

	inline Vector3<T> Cross(const Vector3<T> &r_v_vec) // cross product
	{
		T tx, ty;

		tx = r_v_vec.y * z - r_v_vec.z * y;
		ty = r_v_vec.z * x - r_v_vec.x * z;
		z = r_v_vec.x * y - r_v_vec.y * x;
		x = tx;
		y = ty;
		return *this;
	}

	/*
	 *	inline Vector3<T> v_Refraction(const Vector3<T> &r_v_normal, float f_eta)
	 *		- calcualte refraction of vector coming trough surface with normal r_v_normal
	 *		- f_eta = index of refraction of first environment to index of refraction of second
	 *		- n(vacuum) = 1, n(air) = 1.00029, n(water) = 1.33,
	 *		  n(glass) = 1.52 - 1.62, n(diamond) = 2.417, n(plastic) = 1.46 - 1.55
	 */
	inline Vector3<T> v_Refraction(const Vector3<T> &r_v_normal, float f_eta) const
	{
		Vector3<T> v_tangent = v_Orthogonal(r_v_normal);
		float f_sin2_psi2 = f_eta * f_eta * (1.0f -
			r_v_normal.f_Dot(*this) * r_v_normal.f_Dot(*this));
		return (r_v_normal * -(float)sqrt(1.0f - f_sin2_psi2)) +
			   (v_tangent * (float)sqrt(f_sin2_psi2));
		// v_transmitted = cos(psi2) * (-r_v_normal) + sin(psi2) * v_tangent
	}

	inline Vector3<T> Refract(const Vector3<T> &r_v_normal, float f_eta)
	{
		return (*this = v_Refraction(r_v_normal, f_eta));
	}

	inline Vector3<T> v_Reflection(const Vector3<T> &r_v_normal) const
	{
		return (*this) - ((r_v_normal * f_Dot(r_v_normal)) * 2.0f);
	}

	inline Vector3<T> Reflect(const Vector3<T> &r_v_normal)
	{
		return ((*this) -= ((r_v_normal * f_Dot(r_v_normal)) * 2.0f));
	}

	inline Vector3<T> v_Orthogonal(const Vector3<T> &r_v_normal) const
	{
		Vector3<T> v_tangent;
		v_tangent = *this - r_v_normal * f_Dot(r_v_normal);
		v_tangent.Normalize();
		return v_tangent;
	}

	inline Vector3<T> Orthogonalize(const Vector3<T> &r_v_normal)
	{
		*this -= r_v_normal * f_Dot(r_v_normal);
		Normalize();
		return *this;
	}
};

typedef Vector3<float> Vector3f;
typedef Vector3<int> Vector3i;

/*
 *								=== ~Vector3 ===
 */


/*
 *								=== Vector4 ===
 */

template <class T> struct Vector4 {
	T x, y, z, w;

	inline Vector4()
	{}

	inline Vector4(T t_x, T t_y, T t_z, T t_w)
		:x(t_x), y(t_y), z(t_z), w(t_w)
	{}

	inline Vector4(Vector2<T> &r_t_vec, T t_z, T t_w)
		:x(r_t_vec.x), y(r_t_vec.y), z(t_z), w(t_w)
	{}

	inline Vector4(T t_x, Vector2<T> &r_t_vec, T t_w)
		:x(t_x), y(r_t_vec.x), z(r_t_vec.y), w(t_w)
	{}

	inline Vector4(T t_x, T t_y, Vector2<T> &r_t_vec)
		:x(t_x), y(t_y), z(r_t_vec.x), w(r_t_vec.y)
	{}

	inline Vector4(Vector2<T> &r_t_vec, Vector2<T> &r_t_vec2)
		:x(r_t_vec.x), y(r_t_vec.y), z(r_t_vec2.x), w(r_t_vec2.y)
	{}

	inline Vector4(Vector3<T> &r_t_vec, T t_w)
		:x(r_t_vec.x), y(r_t_vec.y), z(r_t_vec.z), w(t_w)
	{}

	inline Vector4(T t_x, Vector3<T> &r_t_vec)
		:x(t_x), y(r_t_vec.x), z(r_t_vec.y), w(r_t_vec.z)
	{}

	inline T f_Length() const
	{
		return (T)sqrt(x * x + y * y + z * z + w * w);
	}

	inline T f_Length2() const
	{
		return x * x + y * y + z * z + w * w;
	}

	inline T operator [](int n_index) const
	{
		if(&x + 1 == &y && &x + 2 == &z && &x + 3 == &w) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else if(n_index == 1)
				return y;
			else if(n_index == 2)
				return z;
			else
				return w;
		}
	}

	inline T &operator [](int n_index)
	{
		if(&x + 1 == &y && &x + 2 == &z && &x + 3 == &w) // should optimize away in release mode
			return *(&x + n_index);
		else {
			if(n_index == 0)
				return x;
			else if(n_index == 1)
				return y;
			else if(n_index == 2)
				return z;
			else
				return w;
		}
	}

	inline void Normalize()
	{
		T t = f_Length();
		
		if(t != 0) {
			t = 1 / t;
			x *= t;
			y *= t;
			z *= t;
			w *= t;
		}
	}

	inline void Normalize(T t_len)
	{
		T t = f_Length();

		if(t != 0) {
			t = t_len / t;
			x *= t;
			y *= t;
			z *= t;
			y *= t;
		}
	}

	inline bool operator ==(const Vector4<T> &r_v_vec) const
	{
		return x == r_v_vec.x && y == r_v_vec.y && z == r_v_vec.z && w == r_v_vec.w;
	}

	inline Vector4<T> operator +(const Vector4<T> &r_v_vec) const
	{
		return Vector4<T>(x + r_v_vec.x, y + r_v_vec.y, z + r_v_vec.z, w + r_v_vec.w);
	}

	inline Vector4<T> operator -(const Vector4<T> &r_v_vec) const
	{
		return Vector4<T>(x - r_v_vec.x, y - r_v_vec.y, z - r_v_vec.z, w - r_v_vec.w);
	}

	inline Vector4<T> operator *(const Vector4<T> &r_v_vec) const
	{
		return Vector4<T>(x * r_v_vec.x, y * r_v_vec.y, z * r_v_vec.z, w * r_v_vec.w);
	}

	inline Vector4<T> operator /(const Vector4<T> &r_v_vec) const
	{
		return Vector4<T>(x / r_v_vec.x, y / r_v_vec.y, z / r_v_vec.z, w / r_v_vec.w);
	}

	inline Vector4<T> operator -() const
	{
		return Vector4<T>(-x, -y, -z, -w);
	}

	inline Vector4<T> operator *(T t) const
	{
		return Vector4<T>(x * t, y * t, z * t, w * t);
	}

	inline Vector4<T> operator /(T t) const
	{
		t = 1 / t;
		return Vector4<T>(x * t, y * t, z * t, w * t);
	}

	inline Vector4<T> operator +(T t) const
	{
		return Vector4<T>(x + t, y + t, z + t, w + t);
	}

	inline Vector4<T> operator -(T t) const
	{
		return Vector4<T>(x - t, y - t, z - t, w - t);
	}

	inline Vector4<T> operator +=(const Vector4<T> &r_v_vec)
	{
		x += r_v_vec.x;
		y += r_v_vec.y;
		z += r_v_vec.z;
		w += r_v_vec.w;
		return *this;
	}

	inline Vector4<T> operator -=(const Vector4<T> &r_v_vec)
	{
		x -= r_v_vec.x;
		y -= r_v_vec.y;
		z -= r_v_vec.z;
		w -= r_v_vec.w;
		return *this;
	}

	inline Vector4<T> operator *=(const Vector4<T> &r_v_vec)
	{
		x *= r_v_vec.x;
		y *= r_v_vec.y;
		z *= r_v_vec.z;
		w *= r_v_vec.w;
		return *this;
	}

	inline Vector4<T> operator /=(const Vector4<T> &r_v_vec)
	{
		x /= r_v_vec.x;
		y /= r_v_vec.y;
		z /= r_v_vec.z;
		w /= r_v_vec.w;
		return *this;
	}

	inline Vector4<T> operator *=(T t)
	{
		x *= t;
		y *= t;
		z *= t;
		w *= t;
		return *this;
	}

	inline Vector4<T> operator /=(T t)
	{
		t = 1 / t;
		x *= t;
		y *= t;
		z *= t;
		w *= t;
		return *this;
	}

	inline Vector4<T> operator +=(T t)
	{
		x += t;
		y += t;
		z += t;
		w += t;
		return *this;
	}

	inline Vector4<T> operator -=(T t)
	{
		x -= t;
		y -= t;
		z -= t;
		w -= t;
		return *this;
	}

	inline T f_Dot(const Vector4<T> &r_v_vec) const
	{
		return x * r_v_vec.x + y * r_v_vec.y + z * r_v_vec.z + w * r_v_vec.w;
	}
};

typedef Vector4<int> Vector4i;
typedef Vector4<float> Vector4f;
typedef Vector4<float> Color4f;

/*
 *								=== Vector4 ===
 */

/*
 *								=== Plane3 ===
 */

template <class T>
struct Plane3 {
	Vector3<T> v_normal;
	T f_dist;

	inline Plane3()
	{}

	inline Plane3(const Vector3<T> &r_v_norm, T _f_dist)
		:v_normal(r_v_norm), f_dist(_f_dist)
	{}

	inline Plane3(const Vector3<T> &r_v_pos, const Vector3<T> &r_v_norm)
		:v_normal(r_v_norm), f_dist(-r_v_norm.f_Dot(r_v_pos))
	{}

	inline Plane3(const Vector3<T> &r_v_u, const Vector3<T> &r_v_v, const Vector3<T> &r_v_pos)
	{
		v_normal = r_v_u.v_Cross(r_v_v);
		v_normal.Normalize();
		f_dist = -v_normal.f_Dot(r_v_pos);
	}
};

typedef Plane3<float> Plane3f;
typedef Plane3<double> Plane3d;

/*
 *								=== Matrix4f ===
 */

/*
 *	struct Matrix4f
 *		- 4x4 column-major order matrix class (suitable for use with OpenGL)
 */
struct Matrix4f {
protected:
	float f[4][4];

public:
	/*
	 *	void Matrix4f::Identity()
	 *		- creates unit matrix (identity transformation)
	 *		- note this is not done automatically by constructor (there's none)
	 */
	void Identity();

	/*
	 *	void Matrix4f::Translation(const Vector3f &r_v_translate)
	 *		- creates translation matrix; r_v_translate is translation vector
	 */
	void Translation(const Vector3f &r_v_translate);

	/*
	 *	void Matrix4f::Translation(float f_translate_x, float f_translate_y, float f_translate_z)
	 *		- creates translation matrix;
	 *		  (f_translate_x, f_translate_y, f_translate_z) is translation vector
	 */
	void Translation(float f_translate_x, float f_translate_y, float f_translate_z);

	/*
	 *	void Matrix4f::Scaling(float f_scale)
	 *		- creates scaling matrix; f_scale is scaling factor (same for x, y and z)
	 */
	void Scaling(float f_scale);

	/*
	 *	void Matrix4f::Scaling(float f_scale_x, float f_scale_y, float f_scale_z)
	 *		- creates scaling matrix; f_scale_x, f_scale_y and f_scale_z are
	 *		  scaling factors for x, y and z, respectively
	 */
	void Scaling(float f_scale_x, float f_scale_y, float f_scale_z);

	/*
	 *	void Matrix4f::Scaling(const Vector3f &r_v_scale)
	 *		- creates scaling matrix; r_v_scale contains scaling factors for x, y and z
	 */
	void Scaling(const Vector3f &r_v_scale);

	/*
	 *	void Matrix4f::RotationX(float f_angle)
	 *		- creates matrix for rotation arround x-axis; f_angle is angle in radians
	 */
	void RotationX(float f_angle);

	/*
	 *	void Matrix4f::RotationY(float f_angle)
	 *		- creates matrix for rotation arround y-axis; f_angle is angle in radians
	 */
	void RotationY(float f_angle);

	/*
	 *	void Matrix4f::RotationZ(float f_angle)
	 *		- creates matrix for rotation arround z-axis; f_angle is angle in radians
	 */
	void RotationZ(float f_angle);

	/*
	 *	void Matrix4f::Rotation(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
	 *		- creates matrix for rotation arround axis given
	 *		  by (f_axis_x, f_axis_y, f_axis_z), f_angle is angle in radians
	 */
	void Rotation(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z);

	/*
	 *	void Matrix4f::Rotation(float f_angle, const Vector3f &r_v_axis)
	 *		- creates matrix for rotation arround axis given by r_v_axis,
	 *		  f_angle is angle in radians
	 */
	void Rotation(float f_angle, const Vector3f &r_v_axis);

	/*
	 *	void Matrix4f::Translate(float f_translate_x, float f_translate_y, float f_translate_z)
	 *		- applies translation on this matrix; translation vector
	 *		  is given by (f_translate_x, f_translate_y, f_translate_z)
	 */
	void Translate(float f_translate_x, float f_translate_y, float f_translate_z);

	/*
	 *	void Matrix4f::Translate(const Vector3f &r_v_translate)
	 *		- applies translation on this matrix; r_v_translate is translation vector
	 */
	void Translate(const Vector3f &r_v_translate);

	/*
	 *	void Matrix4f::Scale(float f_scale)
	 *		- applies scaling on this matrix; f_scale is scaling factor (for all x, y and z)
	 */
	void Scale(float f_scale);

	/*
	 *	void Matrix4f::Scale(float f_scale_x, float f_scale_y, float f_scale_z)
	 *		- applies scaling on this matrix; f_scale_x, f_scale_y, f_scale_z
	 *		  are scaling factors for x, y and z, respectively
	 */
	void Scale(float f_scale_x, float f_scale_y, float f_scale_z);

	/*
	 *	void Matrix4f::Scale(const Vector3f &r_v_scale)
	 *		- applies scaling on this matrix; r_v_scale contains scaling factors for x, y and z
	 */
	void Scale(const Vector3f &r_v_scale);

	/*
	 *	void Matrix4f::RotateX(float f_angle)
	 *		- applies rotation f_angle radians arround x-axis to this matrix
	 */
	void RotateX(float f_angle);

	/*
	 *	void Matrix4f::RotateY(float f_angle)
	 *		- applies rotation f_angle radians arround y-axis to this matrix
	 */
	void RotateY(float f_angle);

	/*
	 *	void Matrix4f::RotateZ(float f_angle)
	 *		- applies rotation f_angle radians arround z-axis to this matrix
	 */
	void RotateZ(float f_angle);

	/*
	 *	void Matrix4f::Rotate(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z)
	 *		- applies rotation f_angle radians arround axis given
	 *		  by (f_axis_x, f_axis_y, f_axis_z) to this matrix
	 */
	void Rotate(float f_angle, float f_axis_x, float f_axis_y, float f_axis_z);

	/*
	 *	void Matrix4f::Rotate(float f_angle, const Vector3f &r_v_axis)
	 *		- applies rotation f_angle radians arround axis given by r_v_axis to this matrix
	 */
	void Rotate(float f_angle, const Vector3f &r_v_axis);

	void ProductOf(const Matrix4f &r_t_mat1, float f_factor);
	void ProductOf(const Matrix4f &r_t_mat1, const Matrix4f &r_t_mat2);

	/*
	 *	Matrix4f Matrix4f::operator *(float f_factor) const
	 *		- returns element-wise multiplication of matrix and f_factor
	 */
	inline Matrix4f operator *(float f_factor) const
	{
		Matrix4f t_mult;
		t_mult.ProductOf(*this, f_factor);
		return t_mult;
	}

	/*
	 *	Matrix4f Matrix4f::operator *=(float f_factor) const
	 *		- element-wise multiplies this matrix by
	 *		  f_factor and returns reference to this
	 *		- note this is faster, than <tt>*this = *this * f_factor;</tt>.
	 */
	Matrix4f &operator *=(float f_factor);

	/*
	 *	Matrix4f Matrix4f::operator *(const Matrix4f &r_t_mat) const
	 *		- returns multiplication of this matrix and r_t_mat
	 */
	inline Matrix4f operator *(const Matrix4f &r_t_mat) const
	{
		Matrix4f t_mult;
		t_mult.ProductOf(*this, r_t_mat);
		return t_mult;
	}

	/*
	 *	Vector4f Matrix4f::operator *(const Vector4f &r_v_vec) const
	 *		- vector-matrix multiplication
	 *		- returns this * r_v_vec
	 */
	Vector4f operator *(const Vector4f &r_v_vec) const;

	/*
	 *	Matrix4f Matrix4f::&operator *=(const Matrix4f &r_t_mat)
	 *		- multiplies this matrix by r_t_mat and returns reference to this
	 *		- note this is better optimized, than just <tt>*this = *this * r_t_mat;</tt>.
	 */
	Matrix4f &operator *=(const Matrix4f &r_t_mat);

	/*
	 *	Vector3f Matrix4f::v_Transform_Pos(const Vector3f &r_v_vec) const
	 *		- transforms position r_v_vec by this matrix
	 *		- equivalent to multiplying this matrix by Vector4f(r_v_vec, 1)
	 */
	Vector3f v_Transform_Pos(const Vector3f &r_v_vec) const;

	/*
	 *	Vector3f Matrix4f::v_Transform_Dir(const Vector3f &r_v_vec) const
	 *		- transforms direction r_v_vec by this matrix
	 *		- equivalent to multiplying this matrix by Vector4f(r_v_vec, 0)
	 */
	Vector3f v_Transform_Dir(const Vector3f &r_v_vec) const;

	/*
	 *	float Matrix4f::f_Subdet(int n_col, int n_row) const
	 *		- returns determinant of this matrix with column n_col and row
	 *		  n_row left out (so it calculates 3x3 matrix determinant)
	 *		- note the result is not multiplied by (-1)^(n_col + n_row)
	 */
	float f_Subdet(int n_col, int n_row) const;

	/*
	 *	float Matrix4f::f_Determinant() const
	 *		- returns determinant of this matrix
	 *		- note it uses subdeterminants, it is optimized for matrices
	 *		  having zeros in the last row (common transformation matrices)
	 */
	float f_Determinant() const;

	/*
	 *	void Matrix4f::FastInvert()
	 *		- inverts this matrix (uses adjunged matrix method)
	 *		- note this is optimized for matrices with bottom row equal to 0 0 0 1
	 *		  (common transformation matrices), this will give faulty output for
	 *		  other matrices; use FullInvert() instead
	 */
	inline void FastInvert()
	{
		Matrix4f t_inverse;
		FastInverseTo(t_inverse);
		*this = t_inverse;
	}

	void FastInverseTo(Matrix4f &r_dest) const;

	inline void FastInverseOf(const Matrix4f &r_src)
	{
		r_src.FastInverseTo(*this);
	}

	/*
	 *	Matrix4f Matrix4f::t_FastInverse() const
	 *		- returns inverse of this matrix (uses adjunged matrix method)
	 *		- note this is optimized for matrices with bottom row equal to 0 0 0 1
	 *		  (common transformation matrices), this will give faulty output for
	 *		  other matrices; use t_FullInverse() instead
	 */
	inline Matrix4f t_FastInverse() const
	{
		Matrix4f t_inverse;
		FastInverseTo(t_inverse);
		return t_inverse;
	}

	/*
	 *	void Matrix4f::FullInvert()
	 *		- inverts this matrix (uses adjunged matrix method)
	 *		- note full here means unoptimized, Invert() can be used to invert
	 *		  matrices with bottom row equal to 0 0 0 1 (common transformation
	 *		  matrices) more optimally
	 */
	inline void FullInvert()
	{
		Matrix4f t_inverse;
		FullInverseTo(t_inverse);
		*this = t_inverse;
	}

	inline void FullInvertNoTranspose()
	{
		Matrix4f t_inverse;
		FullInverseNoTransposeTo(t_inverse);
		*this = t_inverse;
	}

	/*
	 *	Matrix4f Matrix4f::t_FullInverse() const
	 *		- inverts this matrix (uses adjunged matrix method)
	 *		- note full here means unoptimized, t_FastInverse() can be used to invert
	 *		  matrices with bottom row equal to 0 0 0 1 (common transformation
	 *		  matrices) more optimally
	 */
	inline Matrix4f t_FullInverse() const
	{
		Matrix4f t_inverse;
		FullInverseTo(t_inverse);
		return t_inverse;
	}

	inline Matrix4f t_FullInverseNoTranspose() const
	{
		Matrix4f t_inverse;
		FullInverseNoTransposeTo(t_inverse);
		return t_inverse;
	}

	void FullInverseTo(Matrix4f &r_dest) const;
	void FullInverseNoTransposeTo(Matrix4f &r_dest) const;

	inline void FullInverseOf(const Matrix4f &r_src)
	{
		r_src.FullInverseTo(*this);
	}

	inline void FullInverseNoTransposeOf(const Matrix4f &r_src)
	{
		r_src.FullInverseNoTransposeTo(*this);
	}

	/*
	 *	void Matrix4f::Transpose()
	 *		- transposes this matrix
	 *		- note this is better optimized, than just <tt>*this = t_Transpose();</tt>.
	 */
	void Transpose();

	/*
	 *	void Matrix4f::Transpose()
	 *		- transposes this matrix
	 */
	void TransposeTo(Matrix4f &r_dest) const;

	/*
	 *	void Matrix4f::Transpose()
	 *		- transposes this matrix
	 */
	inline void TransposeOf(const Matrix4f &r_src)
	{
		r_src.TransposeTo(*this);
	}

	/*
	 *	Matrix4f Matrix4f::t_Transpose() const
	 *		- returns transposition of this matrix
	 */
	inline Matrix4f t_Transpose() const
	{
		Matrix4f t_transpose;
		TransposeTo(t_transpose);
		return t_transpose;
	}

	inline float *p_Data()
	{
		return &f[0][0];
	}

	inline const float *p_Data() const
	{
		return &f[0][0];
	}

	/*
	 *	inline float *Matrix4f::operator [](int n_index)
	 *		- returns pointer to n_index-th column of this matrix (math notation)
	 */
	inline float *operator [](int n_index) { return f[n_index]; }

	/*
	 *	inline const float *Matrix4f::operator [](int n_index)
	 *		- returns const pointer to n_index-th column of this matrix (math notation)
	 */
	inline const float *operator [](int n_index) const { return f[n_index]; }

	/*
	 *	inline Vector3f Matrix4f::v_Right() const
	 *		- returns local x-axis vector
	 */
	inline Vector3f v_Right() const { return Vector3f(f[0][0], f[0][1], f[0][2]); }

	/*
	 *	inline Vector3f Matrix4f::v_Up() const
	 *		- returns local y-axis vector
	 */
	inline Vector3f v_Up() const { return Vector3f(f[1][0], f[1][1], f[1][2]); }

	/*
	 *	inline Vector3f Matrix4f::v_Dir() const
	 *		- returns local z-axis vector
	 */
	inline Vector3f v_Dir() const { return Vector3f(f[2][0], f[2][1], f[2][2]); }

	/*
	 *	inline Vector3f Matrix4f::v_Offset() const
	 *		- returns offset vector
	 */
	inline Vector3f v_Offset() const { return Vector3f(f[3][0], f[3][1], f[3][2]); }

	/*
	 *	inline void Matrix4f::Right(const Vector3f &r_v_vec)
	 *		- sets x-axis vector r_v_vec
	 */
	inline void Right(const Vector3f &r_v_vec)
	{
		f[0][0] = r_v_vec.x;
		f[0][1] = r_v_vec.y;
		f[0][2] = r_v_vec.z;
	}

	/*
	 *	inline void Matrix4f::Up(const Vector3f &r_v_vec)
	 *		- sets y-axis vector r_v_vec
	 */
	inline void Up(const Vector3f &r_v_vec)
	{
		f[1][0] = r_v_vec.x;
		f[1][1] = r_v_vec.y;
		f[1][2] = r_v_vec.z;
	}

	/*
	 *	inline void Matrix4f::Dir(const Vector3f &r_v_vec)
	 *		- sets z-axis vector r_v_vec
	 */
	inline void Dir(const Vector3f &r_v_vec)
	{
		f[2][0] = r_v_vec.x;
		f[2][1] = r_v_vec.y;
		f[2][2] = r_v_vec.z;
	}

	/*
	 *	inline void Matrix4f::Offset(const Vector3f &r_v_vec)
	 *		- sets offset vector r_v_vec
	 *		- note again, this *sets* offset, does *not* add vector to offset
	 */
	inline void Offset(const Vector3f &r_v_vec)
	{
		f[3][0] = r_v_vec.x;
		f[3][1] = r_v_vec.y;
		f[3][2] = r_v_vec.z;
	}
};

/*
 *								=== ~Matrix4f ===
 */

#endif // __VECTOR2_INCLUDED
