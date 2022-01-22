#include "file_helper.hpp"
#include "File.hpp"

Array<char> read_text_file (char const * filename)
{
	File file = open_file(filename);
	if (file_is_open(&file) == false)
	{
		return {};
	}		

	int length = file_get_length(&file);
	auto text = Array<char>(length + 1, global_debug_allocator);
	file_read(&file, 0, length, text.get_memory_ptr());
	close_file(&file);

	text[length] = 0;

	return text;
}

void write_text_file(char const * filename, int length, char const * contents)
{
	File file = open_file(filename);
	if (file_is_open(&file) == false)
	{
		return;
	}

	file_write(&file, 0, length, contents);
	close_file(&file);
}
