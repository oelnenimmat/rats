
inline bool operator == (VECTOR const & a, VECTOR const & b)
{
	bool same = true;
	for (int i = 0; i < DIMENSION; i++)
	{
		same = same && (a.values[i] == b.values[i]);
	}
	return same;
}

inline VECTOR & operator %= (VECTOR & v, int s)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		v.values[i] %= s;
	}
	return v;
}

inline VECTOR operator % (VECTOR v, int s) { return v %= s; }

/*
inline VECTOR & operator %= (VECTOR & a, VECTOR const & b)
{
	for (int i = 0; i < DIMENSION; i++)
	{
		a.values[i] %= b.values[i];
	}
	return a;
}

inline VECTOR operator % (VECTOR a, VECTOR b) { return a %= b; }
*/