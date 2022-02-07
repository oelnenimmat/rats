#pragma once

// using char;
// using short;
// using int;
using int64 = long long;

using u8 = unsigned char;
// using ushort = unsigned short;
using uint = unsigned int;
using uint64 = unsigned long long;

// This is how std::byte is defined.
// it is a distinct type from unsigned char, and is not integer?
// less confusion about anything (not that I had any in MY code)
enum struct byte : unsigned char {};