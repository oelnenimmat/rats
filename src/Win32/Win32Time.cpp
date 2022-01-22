#include "win32_platform.hpp"

using uint64 = unsigned long long;

uint64 time_now()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart;
}

float time_elapsed_time(uint64 start, uint64 end)
{
	static uint64 frequency = []()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency.QuadPart;
	}();

	float seconds = static_cast<float>(end - start) / frequency;
	return seconds;
}