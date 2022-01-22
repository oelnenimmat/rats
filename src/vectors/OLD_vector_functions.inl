#include "vector_functions_2.inl"

// #define VECTOR_CONST_REF VECTOR const &

// SCALAR dot(VECTOR_CONST_REF a, VECTOR_CONST_REF b);
// SCALAR sqr_length(VECTOR_CONST_REF v);
// SCALAR length(VECTOR_CONST_REF v);
// VECTOR normalize(VECTOR v);
// VECTOR lerp(VECTOR a, VECTOR_CONST_REF b, SCALAR t);
// VECTOR clamp(VECTOR v, SCALAR min, SCALAR max);
// VECTOR clamp(VECTOR v, VECTOR_CONST_REF min, VECTOR_CONST_REF max);
// VECTOR floor(VECTOR v);
// VECTOR ceil(VECTOR v);
// VECTOR reflect(VECTOR v, VECTOR_CONST_REF normal);

// #if DIMENSION == 3
// 	VECTOR cross (VECTOR lhs, VECTOR_CONST_REF rhs);
// #endif

// #if defined VECTOR_FUNCTIONS_IMPLEMENTATION
// 	#include <cmath>

// 	#if DIMENSION == 1
// 		#define OPERATE(x, y, z, w) x;
// 	#elif DIMENSION == 2
// 		#define OPERATE(x, y, z, w) x; y;
// 	#elif DIMENSION == 3
// 		#define OPERATE(x, y, z, w) x; y; z;
// 	#elif DIMENSION == 4
// 		#define OPERATE(x, y, z, w) x; y; z; w;
// 	#else
// 		#error Bad dimensions
// 	#endif

// 	SCALAR dot(VECTOR_CONST_REF a, VECTOR_CONST_REF b)
// 	{
// 		SCALAR d = 0;
// 		OPERATE(
// 			d += a.x * b.x,
// 			d += a.y * b.y,
// 			d += a.z * b.z,
// 			d += a.w * b.w
// 		)
// 		return d;
// 	}

// 	SCALAR sqr_length(VECTOR_CONST_REF v)
// 	{
// 		return dot(v, v);
// 	}

// 	SCALAR length(VECTOR_CONST_REF v)
// 	{
// 		return std::sqrt(sqr_length(v));
// 	}

// 	VECTOR normalize(VECTOR v)
// 	{
// 		return v / length(v);
// 	}

// 	VECTOR lerp(VECTOR a, VECTOR_CONST_REF b, SCALAR t)
// 	{
// 		a = a + (b - a) * t;
// 		return a;
// 	}

// 	#if DIMENSION == 3
// 		VECTOR cross(VECTOR lhs, VECTOR_CONST_REF rhs)
// 		{
// 			lhs = VECTOR(
// 				lhs.y * rhs.z - lhs.z * rhs.y,
// 				lhs.z * rhs.x - lhs.x * rhs.z,
// 				lhs.x * rhs.y - lhs.y * rhs.x
// 			);
// 			return lhs;
// 		}
// 	#endif

// 	VECTOR clamp(VECTOR v, SCALAR min, SCALAR max)
// 	{
// 		OPERATE(
// 			v.x = v.x < min ? min : v.x > max ? max : v.x,
// 			v.y = v.y < min ? min : v.y > max ? max : v.y,
// 			v.z = v.z < min ? min : v.z > max ? max : v.z,
// 			v.w = v.w < min ? min : v.w > max ? max : v.w
// 		)
// 		return v;
// 	}

// 	/*
// 	VECTOR clamp(VECTOR v, VECTOR_CONST_REF min, VECTOR_CONST_REF max)
// 	{
// 		OPERATE(
// 			v.x = v.x < min.x ? min.x : v.x > max.x ? max.x : v.x,
// 			v.y = v.y < min.y ? min.y : v.y > max.y ? max.y : v.y,
// 			v.z = v.z < min.z ? min.z : v.z > max.z ? max.z : v.z,
// 			v.w = v.w < min.w ? min.w : v.w > max.w ? max.w : v.w
// 		)
// 		return v;
// 	}
// 	*/
	
// 	VECTOR floor (VECTOR v)
// 	{
// 		OPERATE(
// 			v.x = floor(v.x),
// 			v.y = floor(v.y),
// 			v.z = floor(v.z),
// 			v.w = floor(v.w)
// 		)
// 		return v;
// 	}

// 	/*
// 	Not needed anywhere yet, lets see if we ever do
// 	VECTOR ceil (VECTOR v)
// 	{
// 		OPERATE(
// 			v.x = ceil(v.x),
// 			v.y = ceil(v.y),
// 			v.z = ceil(v.z),
// 			v.w = ceil(v.w)
// 		)
// 		return v;
// 	}
// 	*/
// 	VECTOR reflect(VECTOR v, VECTOR_CONST_REF normal)
// 	{
// 		v = v - normal * 2 * dot(v, normal);
// 		return v;
// 	}

// 	#undef OPERATE
// #endif

// #undef VECTOR_CONST_REF