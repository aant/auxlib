#include "file.h"
#include "unicode.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#pragma warning(pop)

namespace aux
{
	#pragma pack(1)

	struct file_t
	{
		HANDLE handle;
		e32_t mode;
	};

	#pragma pack()

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static file_t* open_ll(const char name[], e32_t mode, DWORD access_mode, DWORD share_mode, DWORD disposition)
	{
		u16_t* name16 = to_utf16((const u8_t*)name, false);

		if (name16 != nullptr)
		{
			HANDLE handle = CreateFileW((const wchar_t*)name16, access_mode, share_mode, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
			free_utf(name16);

			if (handle != INVALID_HANDLE_VALUE)
			{
				file_t* file = (file_t*)alloc_mem(sizeof(file_t));
				file->handle = handle;
				file->mode = mode;
				return file;
			}
		}

		return nullptr;
	}

	static bool seek_ll(file_t* file, DWORD move_method, i64_t offset)
	{
		LARGE_INTEGER pos;
		pos.QuadPart = offset;
		return (bool)SetFilePointerEx(file->handle, pos, nullptr, move_method);
	}

	///////////////////////////////////////////////////////////
	//
	//	File functions
	//
	///////////////////////////////////////////////////////////

	i64_t get_file_size(const file_t* file)
	{
		LARGE_INTEGER size;

		if (GetFileSizeEx(file->handle, &size))
		{
			return size.QuadPart;
		}

		return -1;
	}

	i64_t get_file_pos(const file_t* file)
	{
		LARGE_INTEGER pos;
		pos.QuadPart = 0;

		if (SetFilePointerEx(file->handle, pos, &pos, FILE_BEGIN))
		{
			return pos.QuadPart;
		}

		return -1;
	}

	file_t* open_file(const char name[], e32_t mode)
	{
		switch (mode)
		{
			case FILE_MODE_READ:
				return open_ll(name, mode, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING);
			case FILE_MODE_WRITE:
				return open_ll(name, mode, FILE_WRITE_DATA, FILE_SHARE_READ, CREATE_ALWAYS);
			case FILE_MODE_APPEND:
				return open_ll(name, mode, FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
			case FILE_MODE_READ_WRITE:
				return open_ll(name, mode, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
			default:
				return nullptr;
		}
	}

	void close_file(file_t* file)
	{
		CloseHandle(file->handle);
		free_mem(file);
	}

	u32_t read_file(file_t* file, u32_t size, void* data)
	{
		AUX_DEBUG_ASSERT(size > 0);
		AUX_DEBUG_ASSERT(data != nullptr);
		AUX_DEBUG_ASSERT((file->mode == FILE_MODE_READ) || (file->mode == FILE_MODE_READ_WRITE));

		DWORD bytes_read;

		if (ReadFile(file->handle, data, (DWORD)size, &bytes_read, nullptr))
		{
			return (u32_t)bytes_read;
		}

		return 0;
	}

	u32_t write_file(file_t* file, u32_t size, const void* data)
	{
		AUX_DEBUG_ASSERT(size > 0);
		AUX_DEBUG_ASSERT(data != nullptr);
		AUX_DEBUG_ASSERT((file->mode == FILE_MODE_WRITE) || (file->mode == FILE_MODE_APPEND) || (file->mode == FILE_MODE_READ_WRITE));

		DWORD bytes_written;

		if (WriteFile(file->handle, data, (DWORD)size, &bytes_written, nullptr))
		{
			return (u32_t)bytes_written;
		}

		return 0;
	}

	bool flush_file(file_t* file)
	{
		return (bool)FlushFileBuffers(file->handle);
	}

	bool seek_file(file_t* file, e32_t origin, i64_t offset)
	{
		switch (origin)
		{
			case FILE_SEEK_BEGIN:
				return seek_ll(file, FILE_BEGIN, offset);
			case FILE_SEEK_END:
				return seek_ll(file, FILE_END, offset);
			case FILE_SEEK_CURRENT:
				return seek_ll(file, FILE_CURRENT, offset);
			default:
				return false;
		}
	}

	bool is_file_exist(const char name[])
	{
		u16_t* name16 = to_utf16((const u8_t*)name, false);

		if (name16 != nullptr)
		{
			DWORD attrib = GetFileAttributesW((const wchar_t*)name16);
			free_utf(name16);

			if (attrib != INVALID_FILE_ATTRIBUTES)
			{
				return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
			}
		}

		return false;
	}

	bool delete_file(const char name[])
	{
		u16_t* name16 = to_utf16((const u8_t*)name, false);

		if (name16 != nullptr)
		{
			if (DeleteFileW((const wchar_t*)name16))
			{
				free_utf(name16);
				return true;
			}

			free_utf(name16);
		}

		return false;
	}

	bool rename_file(const char name_old[], const char name_new[])
	{
		u16_t* name_old16 = to_utf16((const u8_t*)name_old, false);

		if (name_old16 != nullptr)
		{
			u16_t* name_new16 = to_utf16((const u8_t*)name_new, false);

			if (name_new16 != nullptr)
			{
				if (MoveFileW((const wchar_t*)name_old16, (const wchar_t*)name_new16))
				{
					free_utf(name_new16);
					free_utf(name_old16);
					return true;
				}

				free_utf(name_new16);
			}

			free_utf(name_old16);
		}

		return false;
	}
}
