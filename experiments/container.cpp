/*
Note(Leo): 
	experiment with common container interface and iterator types
*/

/*
Container types

(T[])
Array<T>
List<T>
Pool<T>
Stack<T>

Container interface:

template<typename T>
struct PointerIterator	

template<typename T>
struct Array
{
	

	T * _data;
	size_t _capacity;	
};

Map/Reduce interface:

*/

#include <iostream>

template<typename T>
struct PointerIterator
{
	using value_type = T;

	T * address;

	PointerIterator(T * address) : address(address) {};

	operator T* () const { return address; }

	/*
	this is used like:
		iterator++;
	which means, we have to increment local value
	and sometimes:
		value = *iterator++;
	which means we have to return something.
	apparently by "definition":
		++iterator; // returns reference
		iterator++; // returns unmodified copy
	*/

	void increment()
	{
		address += 1;
	}

	PointerIterator& operator ++()
	{
		// std::cout << "[PRE]: ";
		increment();
		return *this;
	}

	PointerIterator operator ++(int)
	{ 
		// std::cout << "[POST]: ";
		PointerIterator copy (address);
		increment();
		return copy;
	}
};

template<typename T>
struct BackwardsPointerIterator
{
	using value_type = T;

	T * address;

	BackwardsPointerIterator(T * address) : address(address) {};

	operator T* () const { return address; }

	/*
	this is used like:
		iterator++;
	which means, we have to increment local value
	and sometimes:
		value = *iterator++;
	which means we have to return something.
	apparently by "definition":
		++iterator; // returns reference
		iterator++; // returns unmodified copy
	*/

	void increment()
	{
		address -= 1;
	}

	BackwardsPointerIterator& operator ++()
	{
		increment();
		return *this;
	}

	BackwardsPointerIterator operator ++(int)
	{ 
		BackwardsPointerIterator copy (address);
		increment();
		return copy;
	}
};

template <typename TIterator>
struct Range
{
	TIterator _begin;
	TIterator _end;

	Range(TIterator _begin, TIterator _end) : _begin(_begin), _end(_end) {}

	TIterator begin() { return _begin; }
	TIterator end() { return _end; }
};

template<typename T>
struct Collection
{
	using iterator = PointerIterator<T>;
	using backwards_iterator = BackwardsPointerIterator<T>;

	iterator begin() {return iterator{_memory}; }
	iterator end() {return iterator{_memory + _capacity}; }

	int _capacity;
	T * _memory;

	Collection(int capacity) : _capacity(capacity), _memory(reinterpret_cast<int*>(malloc(capacity))) {}
	~Collection() { free(_memory); }

	Range<backwards_iterator> backwards()
	{
		return Range<backwards_iterator>(
			BackwardsPointerIterator<T>{_memory + _capacity - 1},
			BackwardsPointerIterator<T>{_memory - 1}
		);
	}
};

template<typename T>
struct BackwardsCollection
{
	using iterator = BackwardsPointerIterator<T>;

	iterator begin() {return iterator{_memory + _capacity - 1}; }
	iterator end() {return iterator{_memory - 1}; }

	int _capacity;
	T * _memory;

	BackwardsCollection(int capacity) : _capacity(capacity), _memory(reinterpret_cast<int*>(malloc(capacity))) {}
	~BackwardsCollection() { free(_memory); }
};

template<typename TRange, typename TFunc>
void map(TRange & range, TFunc func)
{
	for(auto it = range.begin(); it != range.end(); ++it)
	{
		func(*it);
	}
}

template<typename TRange, typename TFunc>
void map(TRange && range, TFunc func)
{
	for(auto it = range.begin(); it != range.end(); ++it)
	{
		func(*it);
	}
}

template<typename TRange, typename TFunc>
void map(TRange & out, TRange & in_a, TRange & in_b, TFunc func)
{
	auto out_it = out.begin(), out_end = out.end();
	auto a_it = in_a.begin(), a_end = in_a.end();
	auto b_it = in_b.begin(), b_end = in_b.end();

	while(out_it != out_end && a_it != a_end && b_it != b_end)
	{
		func(*out_it, *a_it, *b_it);

		out_it++;
		a_it++;
		b_it++;
	}
}

template<typename TResult, typename TCollection, typename TFunc>
auto secret_reduce(TCollection & collection, TFunc func)
{
	TResult result {};
	for (auto it = collection.begin(); it != collection.end(); ++it)
	{
		func(result, *it);
	}
	return result;
}

template<typename TCollection, typename TFunc>
auto reduce(TCollection & collection, TFunc func)
{
	return secret_reduce<typename TCollection::value_type>(collection, func);
}


template<typename TResult, typename TCollection, typename TFunc>
auto reduce_2(TCollection & collection, TFunc func)
{
	return secret_reduce<TResult>(collection, func);
}

#include <vector>
#include <string>

template<typename TTo, typename TTFrom>
TTo & unsafe_cast(TTFrom & from)
{
	return * reinterpret_cast<TTo*>(&from);
}

int main()
{
	Collection<int> ints(10);
	int count = 0;

	map(ints, [&](int & i){i = count; count += 1; });
	map(ints.backwards(), [&](int i){ std::cout << i << "\n"; });

	count = 0;
	for (auto & i : ints.backwards())
	{
		i += count * 2;
		count += 1;
	}

	map(ints, [](int i){ std::cout << i << "\n"; });


	std::vector<std::string> words =
	{
		"hello", "from", "strange", "list", "of", "words"
	};


	map(words, [](auto const & word) {std::cout << word << "\n";});
	auto result = reduce(words, [](auto & result, auto const & word)
	{
		if (result.length() > 0)
		{
			result += " ";
		}
		result += word;
	});

	std::cout << result << "\n";

	int l_count = reduce_2<int>(result, [](int & count, char c)
	{
		if (c == 'l')
		{
			count += 1;
		}
	});
	std::cout << l_count << " 'l's in '" << result <<"'\n";



	std::vector<int> ints_a = {1,1,2,3,5,8,-1};
	std::vector<int> ints_b = {1,2,3,4,6,7,92};
	std::vector<int> out;
	out.resize(ints_a.size());

	// // for (int & out : out, int a : ints_a, int b : ints_b)
	// for(int & out, int a, int b : out, ints_a, ints_b)
	// {
	// 	out = a * b
	// }

	map(out, ints_a, ints_b, [](int & out, int a, int b)
	{ 
		out = a * b;
	});
	
	


	map(out, [](int i) { std::cout << "\t" << i << "\n"; });



	std::cout << "Hello end\n";
}