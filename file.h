#pragma once

#include "base.h"

namespace aux
{
	enum
	{
		FileMode_BadEnum = -1,

		FileMode_Read,
		FileMode_Write,
		FileMode_Append,
		FileMode_ReadWrite,

		FileMode_MaxEnums
	};

	enum
	{
		FileSeek_BadEnum = -1,

		FileSeek_Begin,
		FileSeek_End,
		FileSeek_Current,

		FileSeek_MaxEnums
	};

	typedef struct FileImpl* FileHandle;

	int64 File_GetSize(const FileHandle file);
	int64 File_GetPosition(const FileHandle file);

	FileHandle File_Open(const char name[], enum32 mode);
	void File_Close(FileHandle file);

	uint32 File_Read(FileHandle file, uint32 size, void* data);
	uint32 File_Write(FileHandle file, uint32 size, const void* data);
	bool File_Flush(FileHandle file);

	bool File_Seek(FileHandle file, enum32 origin, int64 offset);

	bool File_IsExist(const char name[]);
	bool File_Delete(const char name[]);
	bool File_Rename(const char nameOld[], const char nameNew[]);
}
