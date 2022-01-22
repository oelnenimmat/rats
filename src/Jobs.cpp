#include "memory.hpp"

#include <iostream>

#include "Stopwatch.hpp"
#include "vectors.hpp"


// https://stackoverflow.com/questions/44190362/what-is-the-fastest-way-for-a-multithread-simd-operation-explicitly

#include "jobs.hpp"


struct FillJob
{
	int * ints;
	int start_value;

	void execute(int i)
	{
		ints[i] = start_value + i;
	}	
};

struct FillJob4
{
	int4 * ints;
	int start_value; 

	// Note(Leo): I did some bechmarking, using int4 for start_value, 
	// so that +1, +2 amd +3 would be embedded to start_value, but that was surprisingly
	// slower.

	void execute(int i)
	{
		int i4 = i * 4;
		ints[i] = int4(
			i4 + start_value,
			i4 + start_value + 1,
			i4 + start_value + 2,
			i4 + start_value + 3
		);
	}
};

struct AddJob
{	
	int * a;
	int * b;
	int * c;

	void execute(int i)
	{
		c[i] = a[i] + b[i];
	}
};

struct AddJob4
{
	int4 const * a;
	int4 const * b;
	int4 * c;

	void execute(int i)
	{
		c[i] = a[i] + b[i];
	}
};

struct PrintJob
{
	int * ints;
	int start_index;

	void execute(int i)
	{
		std::cout << ints[start_index + i] << "\n";
	}
};

void do_job_test()
{
	std::cout << "[JOBS]: begin test\n";


	JobQueue jobs (global_debug_allocator, 10);

	// int capacity = 10;
	// int capacity = 2 * 4;
	int capacity = 100'000'000 * 4;
	size_t size = capacity * sizeof(int);
	int * a = reinterpret_cast<int*>(global_debug_allocator.typeless_allocate(size, alignof(int4)));	
	int * b = reinterpret_cast<int*>(global_debug_allocator.typeless_allocate(size, alignof(int4)));
	int * c = reinterpret_cast<int*>(global_debug_allocator.typeless_allocate(size, alignof(int4)));

	PrintJob print =
	{
		.ints = c,
		.start_index = capacity - 8
	};

#if 0
	FillJob fill_a =
	{
		.ints = a, 
		.start_value = 0
	};
	jobs.enqueue(fill_a, capacity);

	FillJob fill_b = 
	{
		.ints = b, 
		.start_value = capacity
	};
	jobs.enqueue(fill_b, capacity);

	AddJob add_job =
	{
		.a = a,
		.b = b,
		.c = c,
	};
	jobs.enqueue(add_job, capacity);
#else
	FillJob4 fill_a =
	{
		.ints = reinterpret_cast<int4*>(a),
		.start_value = 0
	};

	FillJob4 fill_b =
	{
		.ints = reinterpret_cast<int4*>(b),
		.start_value = capacity
	};

	AddJob4 add_job =
	{
		.a = reinterpret_cast<int4*>(a),
		.b = reinterpret_cast<int4*>(b),
		.c = reinterpret_cast<int4*>(c),
	};

	jobs.enqueue_parallel(fill_a, capacity / 4);
	jobs.enqueue_parallel(fill_b, capacity / 4);
	jobs.enqueue(print, 8);
	jobs.enqueue_parallel(add_job, capacity / 4);
#endif

	jobs.enqueue(print, 8);

	STOPWATCH_START(execute);
	
	jobs.execute();

	STOPWATCH_PRINT_MS(execute);

	// something
	jobs.wait();

	STOPWATCH_PRINT_MS(execute);

	global_debug_allocator.typeless_deallocate(a);
	global_debug_allocator.typeless_deallocate(b);
	global_debug_allocator.typeless_deallocate(c);


	std::cout << "[JOBS]: end test\n";
}