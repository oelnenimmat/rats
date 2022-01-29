#pragma once

#include "vectors.hpp"

template<typename TFunc>
void for_xyz(int3 range, TFunc func)
{
	for (int z = 0; z < range.z; z++)
	{
		for (int y = 0; y < range.y; y++)
		{
			for (int x = 0; x < range.x; x++)
			{
				func(x,y,z);
			}
		}
	}	
}