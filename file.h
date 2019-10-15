#pragma once

#include "base.h"

namespace aux
{
	enum
	{
		FILE_MODE_BAD_ENUM = -1,

		FILE_MODE_READ,
		FILE_MODE_WRITE,
		FILE_MODE_APPEND,
		FILE_MODE_READ_WRITE,

		FILE_MODE_MAX_ENUMS
	};

	enum
	{
		FILE_SEEK_BAD_ENUM = -1,

		FILE_SEEK_BEGIN,
		FILE_SEEK_END,
		FILE_SEEK_CURRENT,

		FILE_SEEK_MAX_ENUMS
	};

	struct file_t;

	i64_t get_file_size(const file_t* file);
	i64_t get_file_pos(const file_t* file);

	file_t* open_file(const char name[], e32_t mode);
	void close_file(file_t* file);

	u32_t read_file(file_t* file, u32_t size, void* data);
	u32_t write_file(file_t* file, u32_t size, const void* data);
	bool flush_file(file_t* file);

	bool seek_file(file_t* file, e32_t origin, i64_t offset);

	bool is_file_exist(const char name[]);
	bool delete_file(const char name[]);
	bool rename_file(const char name_old[], const char name_new[]);
}
