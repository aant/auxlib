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

	struct FileImpl
	{
		HANDLE handle;
		enum32 mode;
	};

	#pragma pack()

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static FileHandle LL_OpenFile(const char name[], enum32 mode, DWORD accessMode, DWORD shareMode, DWORD disposition)
	{
		uint16* name16 = Unicode_Utf8to16((const uint8*)name, false);

		if (name16 != nullptr)
		{
			HANDLE handle = CreateFileW((const wchar_t*)name16, accessMode, shareMode, nullptr, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
			Unicode_Free(name16);

			if (handle != INVALID_HANDLE_VALUE)
			{
				FileHandle file = (FileHandle)Memory_Allocate(sizeof(FileImpl));
				file->handle = handle;
				file->mode = mode;
				return file;
			}
		}

		return nullptr;
	}

	static bool LL_SeekFile(FileHandle file, DWORD moveMethod, int64 offset)
	{
		LARGE_INTEGER pos;
		pos.QuadPart = offset;
		return (bool)SetFilePointerEx(file->handle, pos, nullptr, moveMethod);
	}

	///////////////////////////////////////////////////////////
	//
	//	File functions
	//
	///////////////////////////////////////////////////////////

	int64 File_GetSize(const FileHandle file)
	{
		LARGE_INTEGER size;

		if (GetFileSizeEx(file->handle, &size))
		{
			return size.QuadPart;
		}

		return -1;
	}

	int64 File_GetPosition(const FileHandle file)
	{
		LARGE_INTEGER pos = {};

		if (SetFilePointerEx(file->handle, pos, &pos, FILE_BEGIN))
		{
			return pos.QuadPart;
		}

		return -1;
	}

	FileHandle File_Open(const char name[], enum32 mode)
	{
		switch (mode)
		{
			case FileMode_Read:
				return LL_OpenFile(name, mode, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING);
			case FileMode_Write:
				return LL_OpenFile(name, mode, FILE_WRITE_DATA, FILE_SHARE_READ, CREATE_ALWAYS);
			case FileMode_Append:
				return LL_OpenFile(name, mode, FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
			case FileMode_ReadWrite:
				return LL_OpenFile(name, mode, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
			default:
				return nullptr;
		}
	}

	void File_Close(FileHandle file)
	{
		CloseHandle(file->handle);
		Memory_Free(file);
	}

	uint32 File_Read(FileHandle file, uint32 size, void* data)
	{
		AUX_DEBUG_ASSERT(size > 0);
		AUX_DEBUG_ASSERT(data != nullptr);
		AUX_DEBUG_ASSERT((file->mode == FileMode_Read) || (file->mode == FileMode_ReadWrite));

		DWORD bytesRead;

		if (ReadFile(file->handle, data, (DWORD)size, &bytesRead, nullptr))
		{
			return (uint32)bytesRead;
		}

		return 0;
	}

	uint32 File_Write(FileHandle file, uint32 size, const void* data)
	{
		AUX_DEBUG_ASSERT(size > 0);
		AUX_DEBUG_ASSERT(data != nullptr);
		AUX_DEBUG_ASSERT((file->mode == FileMode_Write) || (file->mode == FileMode_Append) || (file->mode == FileMode_ReadWrite));

		DWORD bytesWritten;

		if (WriteFile(file->handle, data, (DWORD)size, &bytesWritten, nullptr))
		{
			return (uint32)bytesWritten;
		}

		return 0;
	}

	bool File_Flush(FileHandle file)
	{
		return (bool)FlushFileBuffers(file->handle);
	}

	bool File_Seek(FileHandle file, enum32 origin, int64 offset)
	{
		switch (origin)
		{
			case FileSeek_Begin:
				return LL_SeekFile(file, FILE_BEGIN, offset);
			case FileSeek_End:
				return LL_SeekFile(file, FILE_END, offset);
			case FileSeek_Current:
				return LL_SeekFile(file, FILE_CURRENT, offset);
			default:
				return false;
		}
	}

	bool File_IsExist(const char name[])
	{
		uint16* name16 = Unicode_Utf8to16((const uint8*)name, false);

		if (name16 != nullptr)
		{
			DWORD attrib = GetFileAttributesW((const wchar_t*)name16);
			Unicode_Free(name16);

			if (attrib != INVALID_FILE_ATTRIBUTES)
			{
				return (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0;
			}
		}

		return false;
	}

	bool File_Delete(const char name[])
	{
		uint16* name16 = Unicode_Utf8to16((const uint8*)name, false);

		if (name16 != nullptr)
		{
			if (DeleteFileW((const wchar_t*)name16))
			{
				Unicode_Free(name16);
				return true;
			}

			Unicode_Free(name16);
		}

		return false;
	}

	bool File_Rename(const char nameOld[], const char nameNew[])
	{
		uint16* nameOld16 = Unicode_Utf8to16((const uint8*)nameOld, false);

		if (nameOld16 != nullptr)
		{
			uint16* nameNew16 = Unicode_Utf8to16((const uint8*)nameNew, false);

			if (nameNew16 != nullptr)
			{
				if (MoveFileW((const wchar_t*)nameOld16, (const wchar_t*)nameNew16))
				{
					Unicode_Free(nameNew16);
					Unicode_Free(nameOld16);
					return true;
				}

				Unicode_Free(nameNew16);
			}

			Unicode_Free(nameOld16);
		}

		return false;
	}
}
