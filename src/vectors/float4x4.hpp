#pragma once

union alignas(16) float4x4
{
	float values[16];

	float4 column(int index) const
	{
		return *reinterpret_cast<float4 const *>(values + (index * 4));
	}

	float4 & column(int index)
	{
		return *reinterpret_cast<float4*>(values + (index * 4));
	}

	float4 row(int index) const
	{
		return float4(
			values[0 + index],
			values[4 + index],
			values[8 + index],
			values[12 + index]
		);
	}
};

inline float4x4 & operator *= (float4x4 & lhs, float4x4 const & rhs)
{
	float4 lhs_0 = lhs.row(0);
	float4 lhs_1 = lhs.row(1);
	float4 lhs_2 = lhs.row(2);
	float4 lhs_3 = lhs.row(3);

	float4 rhs_0 = rhs.column(0);
	float4 rhs_1 = rhs.column(1);
	float4 rhs_2 = rhs.column(2);
	float4 rhs_3 = rhs.column(3);

	lhs = 
	{
		dot(lhs_0, rhs_0), dot(lhs_1, rhs_0), dot(lhs_2, rhs_0), dot(lhs_3, rhs_0),
		dot(lhs_0, rhs_1), dot(lhs_1, rhs_1), dot(lhs_2, rhs_1), dot(lhs_3, rhs_1),
		dot(lhs_0, rhs_2), dot(lhs_1, rhs_2), dot(lhs_2, rhs_2), dot(lhs_3, rhs_2),
		dot(lhs_0, rhs_3), dot(lhs_1, rhs_3), dot(lhs_2, rhs_3), dot(lhs_3, rhs_3),
	};
	return lhs;
}

inline float4x4 operator * (float4x4 lhs, float4x4 const & rhs)
{
	return lhs *= rhs;
}

/* Study:
	matrix multiplication implies change of space
	dot means roughly "how much are we same"
	so this kinda describes how much some vector is same with basis axes of described space
*/
inline float3 multiply_vector(float4x4 const & lhs, float3 rhs)
{
	rhs = float3(
		dot(rhs, float3(lhs.values[0], lhs.values[4], lhs.values[8])), 
		dot(rhs, float3(lhs.values[1], lhs.values[5], lhs.values[9])), 
		dot(rhs, float3(lhs.values[2], lhs.values[6], lhs.values[10]))
	);
	return rhs;
}