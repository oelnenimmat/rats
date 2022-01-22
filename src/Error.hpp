#pragma once

#include <iostream>

struct Error
{
	bool has_error_code;
	int error_code;

	const char * reason;

	operator bool() const
	{
		return has_error_code || (reason != nullptr);
	}
};

inline Error no_error();
inline Error error(int error_code);
inline Error error(char const * reason);
inline Error error(char const * reason, int error_code);

inline std::ostream& operator << (std::ostream& os, const Error& r);

#include "Error.inl"
