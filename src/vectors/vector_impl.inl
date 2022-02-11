/*
This file is intended to included multiple times with different #definitions for:
	SCALAR 		scalar type of vector, e.g. float
	DIMENSION	number of components in vector, e.g. 3
	VECTOR 		name of vector type, e.g. float3

Operations are implemented as loops in hope that:
	- compiler will auto vectorize
	- unroll

wherever possible

This may require compiler switches.
https://clang.llvm.org/docs/AttributeReference.html#pragma-unroll-pragma-nounroll

Currently (1/22) this works fast enough, we are bound by gpu performance anyway right now

Functions that return VECTOR by value, usually take first argument by value, modifies it
and returns it. This apparently enables somthing related to copy elision. [Source???]

Todo:
	- std min, max, floor etc. are used here. think and teste through whether or not
		that is a good idea
	- test: not all of these functions are used, and therefore not under everyday testing
		all of these are probably eventually needed, so they should be somehow automatically
		tested.

Some functions are implemented here, but not used anywhere so they are commented out

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

#include <cmath> // floor, sqrt, ceil(?)
#include <algorithm> // min, max

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

inline VECTOR floor (VECTOR v)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = std::floor(v.values[i]);
	}
	return v;
}

/*
inline VECTOR ceil (VECTOR v)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = std::ceil(v.values[i]);
	}
	return v;
}
*/

inline VECTOR reflect(VECTOR v, VECTOR const & normal)
{
	v = v - normal * 2 * dot(v, normal);
	return v;
}


// inline VECTOR min(VECTOR v, SCALAR s)
// {
// 	for (int i = 0; i < DIMENSION; i++)
// 	{
// 		v.values[i] = std::min(v.values[i], s);
// 	}
// 	return v;
// }

inline VECTOR min(VECTOR a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] = std::min(a.values[i], b.values[i]);
	}
	return a;
}

inline VECTOR max(VECTOR v, SCALAR s)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] = std::max(v.values[i], s);
	}
	return v;
}

inline VECTOR max(VECTOR a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] = std::max(a.values[i], b.values[i]);
	}
	return a;
}
