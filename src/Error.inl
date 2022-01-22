inline Error no_error()
{
	return {};
}

inline Error error(int error_code)
{
	Error r;
	r.has_error_code = true;
	r.error_code = error_code;
	r.reason = nullptr;
	return r;
}

inline Error error(char const * reason)
{
	Error r;
	r.has_error_code = false;
	r.error_code = 0;
	r.reason = reason;
	return r;
}

inline Error error(char const * reason, int error_code)
{
	Error r;
	r.has_error_code = true;
	r.error_code = error_code;
	r.reason = reason;
	return r;
}


inline std::ostream& operator << (std::ostream& os, const Error& r)
{
	if (r.reason != nullptr)
	{
		os << r.reason << " ";
	}
	
	if (r.has_error_code)
	{
		os << "(" << r.error_code << ")";
	}

	return os;
}