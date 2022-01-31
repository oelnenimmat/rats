#include <cstdlib>

template<typename T, int Size = 0>
struct Array2
{
	constexpr int length() { return Size; };
	T * data() { return _data; }
	T _data [Size];
};

template<typename T>
struct Array2<T, 0>
{
	T * _data 	= nullptr;
	int _length = 0;	

	int length() { return _length; }

	bool resize(int new_length)
	{
		T * data = reinterpret_cast<T*>(malloc(sizeof(T) * new_length));

		if (data == nullptr)
		{
			return false;
		}

		if (_data != nullptr)
		{
			dispose();
		}

		_data = data;
		_length = new_length;

		return true;
	}

	void dispose()
	{
		free(_data);
		_data = nullptr;
		_length = 0;
	}

};

template<typename T>
Array2<T> allocate_array(int length)
{
	Array2<T> array;
	array.resize(length);
	return array;
}


void array_2_test()
{
	Array2<int, 50> ints = { 89, 91, 6, 7 };
	Array2<float> floats = allocate_array<float>(50);

	// ....

	floats.dispose();
}