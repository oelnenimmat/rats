#include "../File.hpp"

#include "win32_platform.hpp"
#include "../assert.hpp"

#if defined MY_ENGINE_DEBUG
File::~File()
{
	MY_ENGINE_ASSERT(file_is_open(this) == false);
}
#endif

File open_file(char const * filename)
{
	void * handle = ::CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);

	File file = {};
	file.descriptor = handle; 
	return file;
}

void close_file(File * file)
{
	::CloseHandle(file->descriptor);
	file->descriptor = nullptr;
}

bool file_is_open(File const * file)
{
	return file->descriptor != nullptr;
}

int file_get_length(File const * file)
{
	LARGE_INTEGER file_size;
	::GetFileSizeEx(file->descriptor, &file_size);
	return file_size.QuadPart;
}

void file_read(File const * file, size_t start, size_t length, void * destination)
{
	LARGE_INTEGER distance;
	distance.QuadPart = start;
	::SetFilePointerEx(file->descriptor, distance, nullptr, FILE_BEGIN);
	::ReadFile(file->descriptor, destination, length, nullptr, nullptr);
}

void file_write(File * file, size_t start, size_t length, void const * source)
{
	LARGE_INTEGER distance;
	distance.QuadPart = start;
	::SetFilePointerEx(file->descriptor, distance, nullptr, FILE_BEGIN);
	::WriteFile(file->descriptor, source, length, nullptr, nullptr);

	::SetEndOfFile(file->descriptor);
}