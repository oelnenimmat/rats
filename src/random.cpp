#include "random.hpp"

#include <cstdlib>

float random_float_01()
{
	return static_cast<float>(rand()) / RAND_MAX;
}