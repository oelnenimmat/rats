#pragma once

using uint = unsigned int;

struct SmallXXHash
{
	static constexpr uint primeA = 0b10011110001101110111100110110001;
	static constexpr uint primeB = 0b10000101111010111100101001110111;
	static constexpr uint primeC = 0b11000010101100101010111000111101;
	static constexpr uint primeD = 0b00100111110101001110101100101111;
	static constexpr uint primeE = 0b00010110010101100110011110110001;

	SmallXXHash() : accumulator(0) {}

	SmallXXHash(uint accumulator) : accumulator(accumulator) {}

	SmallXXHash(SmallXXHash const & old) : accumulator(old.accumulator) {}

	static SmallXXHash seed (int s)
	{
		return SmallXXHash((uint)s + primeE);
	}

	static uint rotate_left(uint data, int steps)
	{
		return (data << steps) | (data >> (32 - steps));
	}

	SmallXXHash eat(uint data) const
	{
		return SmallXXHash(rotate_left(
			accumulator + data * primeC,
			17
		) * primeD);
	}


	// testing
	SmallXXHash eat(float data) const
	{
		return SmallXXHash(rotate_left(
			accumulator + *reinterpret_cast<uint*>(&data) * primeC, 
			17
		) * primeD);
	}

	float get_float_B_01() const
	{
		uint bytes = (get_avalanche() >> 16) & 65535;
		return static_cast<float>(bytes) * (1.0f / 65535.0f);
	}

	float get_float_A_01() const
	{
		uint bytes = get_avalanche() & 65535;
		return static_cast<float>(bytes) * (1.0f / 65535.0f);
	}


	float get_float_01() const
	{
		return get_float_A_01();
	}

	SmallXXHash operator + (int i) const
	{
		return SmallXXHash(accumulator + (uint)i);
	}
 
	uint get_avalanche() const
	{
		uint avalanche = accumulator;
		avalanche ^= avalanche >> 15;
		avalanche *= primeB;
		avalanche ^= avalanche >> 13;
		avalanche *= primeC;
		avalanche ^= avalanche >> 16;

		return avalanche;
	}

private:
	uint accumulator;
};
