// Should be included to memory.hpp as inline struct template definition

template <typename T>
struct Slice
{
	Slice(T * memory, size_t length) :
		_memory(memory),
		_length(length)
	{}

	const T & operator[](size_t index)
	{
		MINIMA_ASSERT(_memory !=nullptr);
		MINIMA_ASSERT(index < _length);

		return _memory[index];
	}

	const T * begin() { return _memory; }
	const T * end() { return _memory + _length; }

private:
	T * _memory;
	size_t _length;
};

/*
template <typename T>
struct Slice3D
{
	Slice3D(T * data, int3 data_sizes, int3 data_offset, int3 sizes) : 
		_data(data),		
		_data_sizes(data_sizes),		
		_data_offset(data_offset),		
		_sizes(sizes)
	{
		#ifdef MY_ENGINE_DEBUG
			int3 bound = _data_offset + _sizes;
			MINIMA_ASSERT(
				bound.x <= _data_sizes.x 
				&& bound.y <= _data_sizes.y 
				&& bound.z <= _data_sizes.z
			);
		#endif
	}		

	T & operator()(int x, int y, int z)
	{
		return _data[convert_index(x,y,z)];
	}

	T & operator()(int3 id)
	{
		return operator()(id.x, id.y, id.z);
	}

	int3 sizes()
	{
		return _sizes;
	}

private:
	int convert_index (int x, int y, int z)
	{
		x += _data_offset.x;
		y += _data_offset.y;
		z += _data_offset.z;

		return x + y * _data_sizes.x + z * _data_sizes.x + _data_sizes.y;
	}
	
	T * _data;
	int3 _data_sizes;
	int3 _data_offset;
	int3 _sizes;
};
*/
