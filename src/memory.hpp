#pragma once

// http://www.swedishcoding.com/2008/08/31/are-we-out-of-memory/

// Note(Leo): I am not sure, if we should include this rather in the inline files
#include "assert.hpp"

// Avoid headers in headers (this is inlined to a header file)
// void* memset(void*, int, size_t);
#include <cstring>


enum struct AllocationType
{
	garbage,
	zero_memory
};

// Note(Leo): Seems like a proper nice idea :) Good job, me
// Note(Leo): Two weeks later(1/2020), not so sure anymore :D
#include "memory/Allocator.inl"
#include "memory/DEBUG_Allocator.inl"
#include "memory/ArenaAllocator.inl"
// #include "memory/Slice.inl"
#include "memory/Array.inl"
#include "memory/List.inl"
#include "memory/Pool.inl"

#include "memory/LowLevelArray.hpp"

extern DEBUG_Allocator global_debug_allocator;

template <typename _, size_t Length>
constexpr size_t array_length(_ (&)[Length])
{
	return Length;
}
/*
// These should be cool, but there's no real use case for any of them yet
template<typename T>
void construct_default(T * where)
{
	new (where) T;
}

template<typename T>
void swap(T & a, T & b)
{
	T temp = a;
	a = b;
	b = temp;
}

template <typename T>
void value_swap(T * a, T * b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
}

template <typename T>
void pointer_swap(T* & a, T* & b)
{
	T * temp = a;
	a = b;
	b = temp;
}
*/
template <typename T>
void unsafe_swap(T & a, T & b)
{
	unsigned char temp [sizeof(T)];
	memcpy(&temp, &a, sizeof(T));
	memcpy(&a, &b, sizeof(T));
	memcpy(&b, &temp, sizeof(T));
}

template<typename TTo, typename TTFrom>
TTo & unsafe_cast(TTFrom & from)
{
	return * reinterpret_cast<TTo*>(&from);
}

