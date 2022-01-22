#include "vector_operators_2.inl"

/*#define VECTOR_CONST_REF VECTOR const &


#if DIMENSION == 1
	#define OPERATE(x, y, z, w) x;
#elif DIMENSION == 2
	#define OPERATE(x, y, z, w) x; y;
#elif DIMENSION == 3
	#define OPERATE(x, y, z, w) x; y; z;
#elif DIMENSION == 4
	#define OPERATE(x, y, z, w) x; y; z; w;
#else
	#error Bad dimensions
#endif

inline VECTOR operator - (VECTOR v)
{
	OPERATE(
		v.x = -v.x,
		v.y = -v.y,
		v.z = -v.z,
		v.w = -v.w
	)
	return v;
}

inline VECTOR & operator += (VECTOR & a, VECTOR_CONST_REF b)
{
	OPERATE(
		a.x += b.x,
		a.y += b.y,
		a.z += b.z,
		a.w += b.w
	)
	return a;
}

inline VECTOR operator + (VECTOR a, VECTOR_CONST_REF b)
{
	return a += b;
}

inline VECTOR & operator -= (VECTOR & a, VECTOR_CONST_REF b)
{
	OPERATE(
		a.x -= b.x,
		a.y -= b.y,
		a.z -= b.z,
		a.w -= b.w
	)
	return a;
}

inline VECTOR operator - (VECTOR a, VECTOR_CONST_REF b)
{
	return a -= b;
}

inline VECTOR & operator *= (VECTOR & a, VECTOR_CONST_REF b)
{
	OPERATE(
		a.x *= b.x,
		a.y *= b.y,
		a.z *= b.z,
		a.w *= b.w
	)
	return a;
}

inline VECTOR operator * (VECTOR a, VECTOR_CONST_REF b)
{
	return a *= b;
}

inline VECTOR & operator *= (VECTOR & v, SCALAR s)
{
	OPERATE(
		v.x *= s,
		v.y *= s,
		v.z *= s,
		v.w *= s
	)
	return v;
}

inline VECTOR operator * (VECTOR v, SCALAR s)
{
	return v *= s;
}

inline VECTOR operator * (SCALAR s, VECTOR v)
{
	return v *= s;
}

inline VECTOR & operator /= (VECTOR & a, VECTOR_CONST_REF b)
{
	OPERATE(
		a.x /= b.x,
		a.y /= b.y,
		a.z /= b.z,
		a.w /= b.w
	)
	return a;
}

inline VECTOR operator / (VECTOR a, VECTOR_CONST_REF b)
{
	return a /= b;
}

inline VECTOR & operator /= (VECTOR & v, SCALAR s)
{
	OPERATE (
		v.x /= s,
		v.y /= s,
		v.z /= s,
		v.w /= s
	)
	return v;
}

inline VECTOR operator / (VECTOR v, SCALAR s)
{
	return v /= s;
}

#undef OPERATE

#undef VECTOR_CONST_REF*/