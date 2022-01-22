#pragma once

#include <chrono>

struct Stopwatch
{
	static Stopwatch start_new()
	{
		return Stopwatch(clock::now());
	}

	float elapsed_seconds() const
	{
		using namespace std::chrono;

	    duration<float, std::milli> ms = clock::now() - start;
	    float seconds = ms.count() / 1000.0f;
		return seconds;
	}

	float elapsed_milliseconds() const
	{
		using namespace std::chrono;

		// Note(Leo): I have no idea, if float would have enough precision
		duration<double, std::milli> ms = clock::now() - start;
		return static_cast<float>(ms.count());
	}
private:
	using clock = std::chrono::high_resolution_clock;
	clock::time_point start;

	Stopwatch(clock::time_point start) : start(start) {}

};

#define STOPWATCH_START(name) auto stopwatch_##name = Stopwatch::start_new();
#define STOPWATCH_PRINT_MS(name) std::cout << #name " took " << stopwatch_##name.elapsed_milliseconds() << " ms\n";