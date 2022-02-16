#pragma once

template<typename T>
struct Pool
{
	Pool() = default;
	Pool(size_t capacity, Allocator & allocator) :
		elements(Array<Element>(capacity, allocator)),
		first_free(0)
	{
		for (int i = 0; i < elements.length(); i++)
		{
			elements[i].next_free = i + 1;
		}
	}

	int next_free()
	{
		MINIMA_ASSERT(first_free < elements.length());
		int index = first_free;

		first_free = elements[first_free].next_free;
		elements[index].item = {};
		elements[index].next_free = in_use;
		return index;
	}

	void free(int index)
	{	
		// This is easy part
		if (index < first_free)
		{
			elements[index].next_free = first_free;
			first_free = index;
		}
		else
		{
			// starting from index, look backwards until a free element is found
			// if the pool was previously empty, first_free would have been bigger
			// than index, so that is handled in above if statement
			for (int previous = index - 1; previous > 0; previous--)
			{
				if (elements[previous].next_free > index)
				{
					elements[index].next_free = elements[previous].next_free;
					elements[previous].next_free = index;

					break;
				}
			}
		}
	}

	bool is_in_use(int index)
	{
		MINIMA_ASSERT(index >= 0 && index < elements.length());
		return elements[index].next_free == in_use;
	}

	T & operator[] (int index) { return elements[index].item; }

	template<typename TFunc>
	void for_each(TFunc func)
	{	
		for (Element & element : elements)
		{
			if (element.next_free == in_use)
			{
				func(element.item);
			} 
		}
	}

	void dispose()
	{
		first_free = 0;
		elements.dispose();
	}

private:
	enum
	{
		// out_of_bounds = -1,
		in_use = -2
	};

	struct Element
	{
		int next_free;
		T item;
	};

	Array<Element> elements;
	int first_free;
};