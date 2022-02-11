#pragma once

#include "configuration.hpp"

#if defined MY_ENGINE_DEBUG
	void minima_assert(bool condition, char const * expression, char const * file, int line);
	#define MINIMA_ASSERT(condition) minima_assert(condition, #condition, __FILE__, __LINE__)

	#define ASSERT_NOT_NULL(pointer) minima_assert(pointer != nullptr, #pointer " must not be nullptr!", __FILE__, __LINE__)

	#if defined MY_ENGINE_DEBUG_NO_WARNINGS
		#define MY_ENGINE_WARNING(condition, warning)
	#else
		void my_engine_warning(bool condition, char const * warning, char const * file, int line);
		#define MY_ENGINE_WARNING(condition, warning) my_engine_warning(condition, warning, __FILE__, __LINE__);
	#endif

	#define MY_ENGINE_EXECUTE_IN_DEBUG_ONLY(thing) thing

#else
	#define MINIMA_ASSERT(condition)
	#define ASSERT_NOT_NULL(pointer)
	#define MY_ENGINE_WARNING(condition, warning)
	#define MY_ENGINE_EXECUTE_IN_DEBUG_ONLY(thing)
#endif
