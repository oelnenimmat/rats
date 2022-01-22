#include <iostream>

#include <stupid/accessor.hpp>

struct Test
{
	// Motivation is to enable using syntax like with this simple example (e.g. item.x() = 56;)
	// but with extended functionality instead of explicit get and set funxtions (e.g. item.get_x() and item.set_x(56))
	float & x() { return _x_and_w; }

	STUPID_IMPORT_ACCESSORS;
	
	Accessor<float> y()
	{ 
		return
		{
			.get = [this]() { return _x_and_w * 3; },
			.set = [this](float const & value) {_x_and_w = value * 2; }
		};
	}

	Getter<float> z()
	{
		return
		{
			.get = [this]() { return _y_and_z; }
		};
	}

	Setter<float> w()
	{
		return
		{
			.set = [this](float const & value) { _y_and_z = value; }
		};
	}


	// This can be done, but they will take up space in memory, which may be unwanted.
	// Also, using methods above, forces syntax like "object.value() = 67;" which is
	// explicit about accessor having extra functionality beyond just accessing a variable
	Accessor<float> X
	{ 
		.get = [this]() { return _x_and_w * 3; },
		.set = [this](float const & value) {_x_and_w = value * 2; }
	};

	Getter<float> Y
	{
		.get = [this]() { return _y_and_z; }
	};

	Setter<float> Z
	{
		.set = [this](float const & value) { _y_and_z = value; }
	};

private:
	float _x_and_w;
	float _y_and_z;
};


int main()
{
	Test t;
	t.x() = 7;
	t.y() = 3;
	t.w() = 6;

	std::cout << t.y() << "\n";
	std::cout << t.z() << "\n";
	std::cout << t.x() << "\n";
}