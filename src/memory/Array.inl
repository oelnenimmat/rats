// Should be included to memory.hpp as inline struct template definition

/*
Is:
	- constant sized typed memory container

Must:
	- manage memory
	- be default initializable as empty thing
	- detect complex types at compile time and reject them OR handle them properly runtime
		(call constructors and destructors)
	- expose:
		empty(): is array empty
		length(): length of array
		dispose(): dispose memory prematurely e.g. for re-allocation or other reuse
		memory_size(): size of allocated memory for interoperability
		pointer to data: (add syntax) pointer to data for interoperability and other
			accelerated usage
		begin()/end(): for algorithms and loops
		random access operator [] or (): random access, duh

		- const-correct versions where applicable/required

*/


template<typename T>
struct Array
{
	Array() : _length(0), _memory(nullptr), _allocator(nullptr) {}

	// These would need to implicitly allocate more memory, which is against
	// design decisions. If needed, we can expose an explicit copy function
	Array(Array const &) = delete;
	void operator = (Array const &) = delete;

	// These are needed so we can assign from constructor or other functions
	// No need to assert emptiness, its a constructor
	Array (Array && old) :
		_length(old._length),
		_memory(old._memory),
		_allocator(old._allocator)
	{
		old._length = 0;
		old._memory = nullptr;
		old._allocator = nullptr;
	}

	// Assign to pre-existing object, assert that it is properly
	// disposed previously
	void operator = (Array && old)
	{
		MINIMA_ASSERT(_length == 0);
		MINIMA_ASSERT(_memory == nullptr);
		MINIMA_ASSERT(_allocator == nullptr);

		_length = old._length;
		_memory = old._memory;
		_allocator = old._allocator; 

		// Note: do not dispose old, it would deallocate memory
		old._length = 0;
		old._memory = nullptr;
		old._allocator = nullptr;
	}

	~Array()
	{
		if (_memory != nullptr)
		{
			dispose();
		}
	}

	Array(size_t length, Allocator & allocator, AllocationType allocation_type = AllocationType::garbage) :
		_length(length),
		_memory(allocator.allocate<T>(length)),
		_allocator(&allocator)
	{
		MINIMA_ASSERT(_memory != nullptr);

		switch(allocation_type)
		{
			case AllocationType::garbage:
				break;

			case AllocationType::zero_memory:
				clear_memory();
				break;
			
			// We may add more types (like default construct)
			// switch without default lets us know, if we did not handle it
			// default: break;
		}
	}

	size_t length() const
	{
		return _length;
	}

	size_t memory_size() const
	{
		return _length * sizeof(T);
	}

	const T & operator[](size_t index) const
	{
		MINIMA_ASSERT(_memory !=nullptr);
		MINIMA_ASSERT(index < _length);

		return _memory[index];
	}

	T & operator[](size_t index)
	{
		MINIMA_ASSERT(_memory !=nullptr);
		MINIMA_ASSERT(index < _length);

		return _memory[index];
	}
	
	void clear_memory()
	{
		MINIMA_ASSERT(_length > 0);
		MINIMA_ASSERT(_memory != nullptr);

		memset(_memory, 0, sizeof(T) * _length);
	}

	void dispose()
	{
		if (_memory != nullptr)
		{
			MINIMA_ASSERT(_allocator != nullptr);

			_allocator->deallocate(_memory);

			_length = 0;
			_memory = nullptr;
			_allocator = nullptr;
		}

		MINIMA_ASSERT(_length == 0);
		MINIMA_ASSERT(_memory == nullptr);
		MINIMA_ASSERT(_allocator == nullptr);
		
	}

	T * get_memory_ptr()
	{
		return _memory;
	}

	T const * get_memory_ptr() const
	{
		return _memory;
	}

	T * begin() { return _memory; }
	T * end() { return _memory + _length; }

	const T * begin() const { return _memory; }
	const T * end() const { return _memory + _length; }

private:
	size_t 		_length;
	T * 		_memory;
	Allocator * _allocator;
};