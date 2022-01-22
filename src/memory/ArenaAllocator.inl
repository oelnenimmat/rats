
using u8 = unsigned char;

struct ArenaAllocator : Allocator
{
	ArenaAllocator() = default;
	ArenaAllocator(size_t capacity, void * memory) :
		_capacity(capacity),
		_used(0),
		_memory(reinterpret_cast<u8*>(memory))
	{
		MY_ENGINE_ASSERT(_memory != nullptr);
	}


	void * typeless_allocate(size_t size, size_t alignment) override
	{
		size_t start = ((_used / alignment) + 1) * alignment;

		_used = start + size;
		MY_ENGINE_ASSERT(_used < _capacity);

		return _memory + start;
	}
	
	void typeless_deallocate(void * pointer) override
	{
		// MY_ENGINE_WARNING(false, "No deallocation happens here!");
	}

	void reset()
	{
		_used = 0;
	}

	void * return_memory_back_to_where_it_was_received()
	{
		void * memory = _memory;

		_capacity = 0;
		_used = 0;
		_memory = nullptr;

		return memory;
	}

	size_t capacity() const { return _capacity; }
	size_t used() const { return _used; }

private:
	size_t _capacity;
	size_t _used;
	u8 * _memory;
};