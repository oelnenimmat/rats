// #include <iostream>

// void * operator new (size_t size) noexcept(false)
// {
// 	std::cout << "using custom new\n";
// 	return ::malloc(size);
// }

// void operator delete(void * memory) noexcept
// {
// 	std::cout << "using custom delete\n";
// 	::free(memory);
// }

// void * operator new[] (size_t size) noexcept(false)
// {
// 	std::cout << "using custom new[]\n";
// 	return ::malloc(size);
// }

// void operator delete[] (void * memory) noexcept
// {
// 	std::cout << "using custom delete[]\n";
// 	::free(memory);
// }