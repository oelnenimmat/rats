template <typename T>
struct List
{
	List() = default; // nullptrs and zeros

	List(size_t capacity, Allocator & allocator) :
		_memory(allocator.allocate<T>(capacity)),
		_capacity(capacity),
		_count(0),
		_allocator(&allocator)
	{}

	List(List const & other) = delete;
	void operator = (List const & other) = delete;

	List(List && old) :
		_memory(old._memory),
		_capacity(old._capacity),
		_count(old._count),
		_allocator(old._allocator)
	{}

	void operator = (List && old)
	{
		MINIMA_ASSERT(_memory == nullptr);
		MINIMA_ASSERT(_capacity == 0);
		MINIMA_ASSERT(_count == 0);
		MINIMA_ASSERT(_allocator == nullptr);

		_memory 	= old._memory;
		_capacity 	= old._capacity;
		_count 		= old._count;
		_allocator 	= old._allocator;

		old._memory 	= nullptr;
		old._capacity 	= 0;
		old._capacity 	= 0;
		old._allocator 	= nullptr;
	}

	~List()
	{
		if (_memory != nullptr)
		{
			_allocator->deallocate(_memory);
		}
	}

	T & operator[](size_t index)
	{ 
		MINIMA_ASSERT(index < _count);
		return _memory[index];
	}

	T const & operator[](size_t index) const
	{ 
		MINIMA_ASSERT(index < _count);
		return _memory[index];
	}

	void add(T item)
	{
		MINIMA_ASSERT(_count < _capacity);
		_memory[_count] = item;
		_count += 1;
	}

	T & add_new()
	{
		MINIMA_ASSERT(_count < _capacity);
		size_t index = _count;
		_count += 1;
		return _memory[index];
	}

	size_t length() { return _count; }

	T * begin() { return _memory; }
	T * end() { return _memory + _count; }

	T const * begin() const { return _memory; }
	T const * end() const { return _memory + _count; }

	void reset()
	{
		// todo:
		// std::is_trivially_destructible
		// Call dctors
		_count = 0;
	}

private:
	T * _memory;
	size_t _capacity;
	size_t _count;
	Allocator * _allocator;
};