// Should be included to memory.hpp as inline struct definition

#if defined MY_ENGINE_USE_PLATFORM_WIN32
// Windows does not provide standard aligned_alloc, so we use _aligned_malloc

#include <malloc.h>

struct DEBUG_Allocator : Allocator
{
    void * typeless_allocate(size_t size, size_t alignment) override
    {
        return ::_aligned_malloc(size, alignment);
    }

    void typeless_deallocate(void * pointer) override
    {
        ::_aligned_free(pointer);
    }
};

#endif