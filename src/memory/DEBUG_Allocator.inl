// Should be included to memory.hpp as inline struct definition

#if defined MY_ENGINE_USE_PLATFORM_WIN32
// Windows does not provide standard aligned_alloc, so we use _aligned_malloc

#include <malloc.h>
#include <cstdio>

struct DEBUG_Allocator : Allocator
{
    void * typeless_allocate(size_t size, size_t alignment) override
    {
        _allocation_count += 1;
        return ::_aligned_malloc(size, alignment);
    }

    void typeless_deallocate(void * pointer) override
    {
        _deallocation_count += 1;
        ::_aligned_free(pointer);
    }

    int _allocation_count = 0;
    int _deallocation_count = 0;

    ~DEBUG_Allocator()
    {
        printf("Closing DEBUG_Allocator, _allocation_count = %i, _deallocation_count = %i \n", _allocation_count, _deallocation_count);
    };
};

#endif