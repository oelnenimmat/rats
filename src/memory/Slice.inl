// Should be included to data.hpp as inline struct template definition

template <typename T>
struct Slice
{
	T & operator[](size_t index)
	{
		MINIMA_ASSERT(_data !=nullptr);
		MINIMA_ASSERT(index < _length);

		return _data[index];
	}

	T const & operator[](size_t index) const
	{
		MINIMA_ASSERT(_data !=nullptr);
		MINIMA_ASSERT(index < _length);

		return _data[index];
	}

	const T * begin() { return _data; }
	const T * end() { return _data + _length; }

	size_t length() const { return _length; }
	size_t memory_size() const { return _length * sizeof(T); }
	T * get_memory_ptr() const { return _data; }

private:
	size_t _length;
	T * _data;

	template<typename U>
	friend Slice<U> make_slice(size_t length, U * data);
};

template<typename T>
Slice<T> make_slice(size_t length, T * data)
{
	Slice<T> slice;
	slice._length = length;
	slice._data = data;
	return slice;
}

template <typename T>
Slice<T> make_slice(Array<T> & array, size_t first, size_t length)
{
	return make_slice<T>(length, array.get_memory_ptr() + first);
}

template<typename T>
void copy_slice_data(Slice<T> & src, Slice<T> const & dst)
{
	MINIMA_ASSERT(src.length() == dst.length());
	memcpy(src.get_memory_ptr(), dst.get_memory_ptr(), src.memory_size());
}