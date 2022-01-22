#pragma once

// using File = void *;
struct File
{
	void * descriptor;

	#if defined MY_ENGINE_DEBUG
	~File();
	#endif
};

File open_file(char const * filename);
void close_file(File *);
bool file_is_open(File const *);
int file_get_length(File const *);
void file_read(File const *, size_t start, size_t length, void * destination);
void file_write(File *, size_t start, size_t length, void const * source);
