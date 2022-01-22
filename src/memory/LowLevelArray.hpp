#pragma once

/*
Low level as in not self managing. Meant to be used easily with jobs,
as reference member variables from normal arrays are harder to work with.

// Todo(Leo): probably just make a slice :D and use full array sized slices
or maybe "views" in jobs
*/
template <typename T>
struct LowLevelArray
{
	LowLevelArray() = default;

	LowLevelArray(size_t length, Allocator & allocator) :
		_length(length),
		_data(allocator.allocate<T>(length)),
		_allocator(&allocator)
	{
		MY_ENGINE_ASSERT(_length > 0);
		MY_ENGINE_ASSERT(_data != nullptr);
	}

	LowLevelArray(LowLevelArray const & other) = default;
	LowLevelArray & operator = (LowLevelArray const & other) = default;

	LowLevelArray(LowLevelArray && old) : 
		_length(old._length),
		_data(old._data),
		_allocator(old._allocator)
	{
		old._length = 0;
		old._data = nullptr;
		old._allocator = nullptr;
	}

	LowLevelArray & operator = (LowLevelArray && old)
	{
		MY_ENGINE_ASSERT(empty());

		_length = old._length;
		_data = old._data;
		_allocator = old._allocator;

		old._length = 0;
		old._data = nullptr;
		old._allocator = nullptr;

		return *this;
	}

	bool empty() const
	{ 
		MY_ENGINE_ASSERT(
			(_length == 0 && _data == nullptr && _allocator == nullptr)
			|| (_length > 0 && _data != nullptr && _allocator != nullptr)
		);
		return _data == nullptr;
	}
	size_t length() const { return _length; }

	void dispose()
	{
		MY_ENGINE_ASSERT(empty() == false);

		_allocator->deallocate(_data);

		_length = 0;
		_data = nullptr;
		_allocator = nullptr;
	}

	T & operator[](int index)
	{ 
		MY_ENGINE_ASSERT(index < _length);
		return _data[index];
	}

	T const & operator[](int index) const
	{ 
		MY_ENGINE_ASSERT(index < _length);
		return _data[index];
	}

	T * begin() { return _data; }
	T * end() { return _data + _length; }

	T const * begin() const { return _data; }
	T const * end() const { return _data + _length; }
	
	// Some nvidia paper used preproccos stuff like this, why shouldn't I? :D
#	if defined MY_ENGINE_DEBUG
	~LowLevelArray()
	{

	}
#	endif

private:
	size_t 		_length;
	T * 		_data;
	Allocator * _allocator;
};