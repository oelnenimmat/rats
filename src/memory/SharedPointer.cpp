/*
Note(Leo): this is an excercise, I don't think we need to use this
*/

template<typename T>
struct SharedPointer
{
	int * reference_counter;
	T * element;

	SharedPointer() = delete;

	SharedPointer(SharedPointer&&) = default;
	void operator = (SharedPointer&&) = default;

	SharedPointer(SharedPointer const & other) :
		reference_counter(other.reference_counter),
		element(other.element)
	 {
	 	reference_counter += 1;
	 }

	 void operator = (SharedPointer const & other)
	 {
	 	*reference_counter -= 1;
	 	if(*reference_counter == 0)
	 	{
	 		delete reference_counter;
	 		delete element;
	 	} 

	 	reference_counter = other.reference_counter;
	 	element = other.element;

	 	*reference_counter += 1;
	 }

	 ~SharedPointer()
	 {
	 	*reference_counter -= 1;
	 	if (*reference_counter == 0)
	 	{
	 		delete reference_counter;
	 		delete element;
	 	}
	 }

	 T & operator *() { return *element; }
	 T const & operator *() const { return *element; }
};

template<typename T>
SharedPointer<T> allocate_shared_pointer(T element)
{
	SharedPointer<T> result;
	result.reference_counter = new int;
	result.element = new T;

	*result.reference_counter = 1;	
	*result.element = element;

	return result;	
}

template<typename T>
struct WeakPointer
{
	int * reference_counter;
	T * element;

	WeakPointer() = delete;

	WeakPointer(WeakPointer && old) = default;
	void operator = (WeakPointer && old) = default;

	WeakPointer(WeakPointer const & other) = default;
	void operator = (WeakPointer const & other) = default;

	T & operator *() { assert(reference_counter > 0); return *element; }
	T const & operator *() const { assert(reference_counter > 0); return *element; }
}

void test_shared_and_weak_pointer()
{
	auto ptr_0 = allocate_shared_pointer<int>(234);
	{
		auto ptr_1 = ptr_0;
		*ptr_1 = 67;
	}

	// *ptr_0 == 67

	WeakPointer<int> weak_0 = ptr_0;
	weak_0 = 98;

	// *ptr_0 == 98
}