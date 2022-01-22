/*
Operations are implemented as loops in hope that:
	- compiler will auto vectorize
	- unroll

wherever possible

This may require compiler switches.
https://clang.llvm.org/docs/AttributeReference.html#pragma-unroll-pragma-nounroll


Currently (1/22) this works fast enough, we are bound by gpu performance anyway right now

Functions that return VECTOR by value, usually take first argument by value, modifies it
and returns it. This apparently enables somthing related to copy elision. [Source???]
*/

// Unary Operators ------------------------------------------------------------
inline VECTOR operator - (VECTOR v)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = -v.values[i];
	}
	return v;
}

// Component wise assignment operators ----------------------------------------

inline VECTOR & operator += (VECTOR & a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] += b.values[i];
	}
	return a;
}

inline VECTOR & operator -= (VECTOR & a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] -= b.values[i];
	}
	return a;
}

inline VECTOR & operator *= (VECTOR & a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] *= b.values[i];
	}
	return a;
}

inline VECTOR & operator /= (VECTOR & a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] /= b.values[i];
	}
	return a;
}

// Component wise regular operators -------------------------------------------

inline VECTOR operator + (VECTOR a, VECTOR const & b)
{
	return a += b;
}

inline VECTOR operator - (VECTOR a, VECTOR const & b)
{
	return a -= b;
}

inline VECTOR operator * (VECTOR a, VECTOR const & b)
{
	return a *= b;
}

inline VECTOR operator / (VECTOR a, VECTOR const & b)
{
	return a /= b;
}

// Assignment operators with single scalar ------------------------------------

inline VECTOR & operator += (VECTOR & v, SCALAR s)
{
	for(int i = 0; i < DIMENSION; i++)
	{
		v.values[i] += s;
	}
	return v;
}

inline VECTOR & operator -= (VECTOR & v, SCALAR s)
{
	for(int i = 0; i < DIMENSION; i++)
	{
		v.values[i] -= s;
	}
	return v;
}

inline VECTOR & operator *= (VECTOR & v, SCALAR s)
{
	for(int i = 0; i < DIMENSION; i++)
	{
		v.values[i] *= s;
	}
	return v;
}

inline VECTOR & operator /= (VECTOR & v, SCALAR s)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] /= s;
	}
	return v;
}

// Regular operators with single scalar ---------------------------------------

inline VECTOR operator + (VECTOR v, SCALAR s)
{
	return v += s;
}

inline VECTOR operator - (VECTOR v, SCALAR s)
{
	return v -= s;
}

inline VECTOR operator * (VECTOR v, SCALAR s)
{
	return v *= s;
}

inline VECTOR operator / (VECTOR v, SCALAR s)
{
	return v /= s;
}

// ----------------------------------------------------------------------------
// FUNCTIONS 
// ----------------------------------------------------------------------------

#include <cmath>

inline SCALAR dot(VECTOR const & a, VECTOR const & b)
{
	SCALAR d = 0;
	for(int i = 0; i < DIMENSION; i++)
	{
		d += a.values[i] * b.values[i];
	}
	return d;
}

inline SCALAR sqr_length(VECTOR const & v)
{
	return dot(v, v);
}

inline SCALAR length(VECTOR const & v)
{
	return std::sqrt(sqr_length(v));
}

inline VECTOR normalize(VECTOR v)
{
	return v / length(v);
}

inline VECTOR lerp(VECTOR a, VECTOR const & b, SCALAR t)
{
	a = a + (b - a) * t;
	return a;
}

#if DIMENSION == 3
	inline VECTOR cross(VECTOR lhs, VECTOR const & rhs)
	{
		lhs = VECTOR(
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		);
		return lhs;
	}
#endif

inline VECTOR clamp(VECTOR v, SCALAR min, SCALAR max)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = v.values[i] < min ? min : v.values[i] > max ? max : v.values[i];
	}
	return v;
}

inline VECTOR clamp(VECTOR v, VECTOR const & min, VECTOR const & max)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = v.values[i] < min.values[i] ? min.values[i] : v.values[i] > max.values[i] ? max.values[i] : v.values[i];
	}
	return v;
}

/*
inline VECTOR clamp(VECTOR v, VECTOR const & min, VECTOR const & max)
{
	OPERATE(
		v.x = v.x < min.x ? min.x : v.x > max.x ? max.x : v.x,
		v.y = v.y < min.y ? min.y : v.y > max.y ? max.y : v.y,
		v.z = v.z < min.z ? min.z : v.z > max.z ? max.z : v.z,
		v.w = v.w < min.w ? min.w : v.w > max.w ? max.w : v.w
	)
	return v;
}
*/

inline VECTOR floor (VECTOR v)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = std::floor(v.values[i]);
	}
	return v;
}

/*
Not needed anywhere yet, lets see if we ever do
inline VECTOR ceil (VECTOR v)
{
	OPERATE(
		v.x = ceil(v.x),
		v.y = ceil(v.y),
		v.z = ceil(v.z),
		v.w = ceil(v.w)
	)
	return v;
}
*/
inline VECTOR reflect(VECTOR v, VECTOR const & normal)
{
	v = v - normal * 2 * dot(v, normal);
	return v;
}