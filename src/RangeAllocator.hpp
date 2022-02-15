persistent allocator

size: int;
used: int;
memory: byte*;

allocate(size):
	mem = memory + used;
	memory += size;
	return mem;

Range
{
	start: int;
	length: int;
}

struct FreeableAllocator
{
	struct Range
	{
		size_t 	start;
		size_t 	length;
		int 	next_free_range;
	};

	Range ranges [100];

	int first_free_range;

	byte * memory;


	void * allocate(size_t size)
	{
		ASSERT_LESS_THAN(first_free_range, 99);

		Range & range = ranges[first_free_range];

		int next_range_index = first_free_range + 1;

		ranges[next_range_index].start 				= range.start + size;
		ranges[next_range_index].length 			= range.length - size;
		ranges[next_range_index].next_free_range 	= range.next_free_range;

		first_free_range = next_range_index;

		return reinterpret_cast<void*>(memory + range.start);
	}

	void free(void * pointer)
	{
		byte * _poonter = reinterpret_cast<byte*>(pointer);
		size_t start = _poonter - memory;

		int range_index = [&]()
		{
			for (int i = 0; i < 100; i++)
			{
				if (ranges[i].start == start)
				{
					return i;
				}
			}
			return 100;
		}();

		
	}
};