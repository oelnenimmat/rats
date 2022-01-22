#include "assert.hpp"
#include "configuration.hpp"

#if defined MY_ENGINE_DEBUG
	#include <iostream>
	void win32_emergency_shutdown();

	void my_engine_assert(bool condition, char const * expression, char const * file, int line)
	{
		if (condition == false)
		{
			std::cout << "[ASSERT FAIL|file " << file << "|line " << line << "]: " << expression << "\n";
			abort();
		}
	}

	void my_engine_warning(bool condition, char const * warning, char const * file, int line)
	{
		if (condition == false)
		{
			std::cout << "[warning|file " << file << "|line " << line << "]: " << warning << "\n";
		}
	}
#endif
