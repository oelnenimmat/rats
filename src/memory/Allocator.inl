// Should be included to memory.hpp as inline struct definition
/*
	Todo(Leo):
		allocation counters did not work with arena allocator
*/

struct Allocator
{
	template<typename T>
	T * allocate(int count)
	{
		// MY_ENGINE_EXECUTE_IN_DEBUG_ONLY(_allocation_count += 1)
		MINIMA_ASSERT(count > 0);

		size_t size = sizeof(T) * count;
		T * result = reinterpret_cast<T*>(typeless_allocate(size, alignof(T)));
		return result;
	}

	template<typename T>
	T * allocate_and_clear_memory(int count)
	{
		T * result = allocate<T>(count);
		::memset(result, 0, sizeof(T) * count);
		return result;
	}

	template <typename T>
	void deallocate(T * pointer)
	{
		// MY_ENGINE_EXECUTE_IN_DEBUG_ONLY(_allocation_count -= 1)

		typeless_deallocate(pointer);
	}

	virtual void* typeless_allocate(size_t size, size_t alignment) = 0;
	virtual void typeless_deallocate(void * pointer) = 0;

// #if defined MY_ENGINE_DEBUG
//     Allocator() : _allocation_count(0) {}
	
//     ~Allocator()
//     {
//         MINIMA_ASSERT(_allocation_count == 0);
//     }
// private:
//     int _allocation_count;

//     #endif
};
