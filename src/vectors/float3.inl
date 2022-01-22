
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

VECTOR operator - (VECTOR v)
{
	OPERATE(
		v.x = -v.x,
		v.y = -v.y,
		v.z = -v.z,
		v.w = -v.w
	)
	return v;
}

VECTOR & operator += (VECTOR & a, VECTOR b)
{
	OPERATE(
		a.x += b.x,
		a.y += b.y,
		a.z += b.z,
		a.w += b.w
	)
	return a;
}

VECTOR operator + (VECTOR a, VECTOR b)
{
	return a += b;
}

VECTOR & operator -= (VECTOR & a, VECTOR b)
{
	OPERATE(
		a.x -= b.x,
		a.y -= b.y,
		a.z -= b.z,
		a.w -= b.w
	)
	return a;
}

VECTOR operator - (VECTOR a, VECTOR b)
{
	return a -= b;
}

VECTOR & operator *= (VECTOR & v, SCALAR s)
{
	OPERATE(
		v.x *= s,
		v.y *= s,
		v.z *= s,
		v.w *= s
	)
	return v;
}

VECTOR operator * (VECTOR v, SCALAR s)
{
	return v *= s;
}

VECTOR & operator /= (VECTOR & v, SCALAR s)
{
	OPERATE (
		v.x /= s,
		v.y /= s,
		v.z /= s,
		v.w /= s
	)
	return v;
}

VECTOR operator / (VECTOR v, SCALAR s)
{
	return v /= s;
}

#undef OPERATE

// float3 operator - (float3 v)
// {
// 	v.x -= v.x;
// 	v.y -= v.y;
// 	v.z -= v.z;
// 	return v;
// }

// float3 & operator += (float3 & a, float3 b)
// {
// 	a.x += b.x;
// 	a.y += b.y;
// 	a.z += b.z;
// 	return a;
// }

// float3 operator + (float3 a, float3 b)
// {
// 	return a += b;
// }

// float3 & operator -= (float3 & a, float3 b)
// {
// 	a.x -= b.x;
// 	a.y -= b.y;
// 	a.z -= b.z;
// 	return a;
// }

// float3 operator - (float3 a, float3 b)
// {
// 	return a -= b;
// }

// float3 & operator *= (float3 & v, float s)
// {
// 	v.x *= s;
// 	v.y *= s;
// 	v.z *= s;
// 	return v;
// }

// float3 operator * (float3 v, float s)
// {
// 	return v *= s;
// }

// float3 & operator /= (float3 & v, float s)
// {
// 	v.x /= s;
// 	v.y /= s;
// 	v.z /= s;

// 	return v;
// }

// float3 operator / (float3 v, float s)
// {
// 	return v /= s;
// }
