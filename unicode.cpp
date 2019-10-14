#include "unicode.h"

namespace aux
{
	enum
	{
		MaxStrlen = INT16_MAX
	};

	static const int8 utf8SeqLens[] =
	{
		-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static uint32 LL_Seq2ToChar(const uint8 seq[])
	{
		return (((uint32)seq[0] & 0x1f) << 6) | (seq[1] & 0x3f);
	}

	static uint32 LL_Seq3ToChar(const uint8 seq[])
	{
		return (((uint32)seq[0] & 0xf) << 12) | (((uint32)seq[1] & 0x3f) << 6) | (seq[2] & 0x3f);
	}

	static uint32 LL_Seq4ToChar(const uint8 seq[])
	{
		return (((uint32)seq[0] & 0x7) << 18) | (((uint32)seq[1] & 0x3f) << 12) | (((uint32)seq[2] & 0x3f) << 6) | (seq[3] & 0x3f);
	}

	static bool LL_IsOctetOk(uint8 octet)
	{
		return (octet & 0xc0) == 0x80;
	}

	static void LL_GetChar(const uint8 seq[], int32& seqLen, uint32& outChar, uint32 badChar)
	{
		switch (seqLen)
		{
			case 1:
				outChar = seq[0];
				return;
			case 2:
				if (LL_IsOctetOk(seq[1]))
				{
					outChar = LL_Seq2ToChar(seq);
					return;
				}
				break;
			case 3:
				if (LL_IsOctetOk(seq[1]) && LL_IsOctetOk(seq[2]))
				{
					outChar = LL_Seq3ToChar(seq);
					return;
				}
				break;
			case 4:
				if (LL_IsOctetOk(seq[1]) && LL_IsOctetOk(seq[2]) && LL_IsOctetOk(seq[3]))
				{
					outChar = LL_Seq4ToChar(seq);
					return;
				}
				break;
		}

		seqLen = 1;
		outChar = badChar;
	}

	static int32 LL_GetSeqLen(const uint8 seq[])
	{
		return utf8SeqLens[*seq];
	}

	static int32 LL_GetSeqLen16(uint32 c32)
	{
		if (c32 < 0x10000)
		{
			if ((c32 < 0xd800) || (c32 > 0xdfff))
			{
				return 1;
			}
		}
		else
		{
			if (c32 <= 0x10ffff)
			{
				return 2;
			}
		}

		return 0;
	}

	static int32 LL_CalcLen16(const uint8 utf8[], uint32 badChar)
	{
		int32 seqLen8 = LL_GetSeqLen(utf8);
		int32 seqLen16 = 0;
		int32 len = 0;
		uint32 c32 = 0;

		while (seqLen8 >= 0)
		{
			LL_GetChar(utf8, seqLen8, c32, badChar);
			seqLen16 = LL_GetSeqLen16(c32);

			if (seqLen16 < 1)
			{
				break;
			}

			utf8 += seqLen8;
			len += seqLen16;

			if (len == MaxStrlen)
			{
				return len;
			}

			seqLen8 = LL_GetSeqLen(utf8);
		}

		if (seqLen8 < 0)
		{
			return len;
		}

		return 0;
	}

	static void LL_Convert(const uint8 utf8[], uint16 utf16[], uint32 badChar)
	{
		int32 seqLen8 = LL_GetSeqLen(utf8);
		int32 seqLen16 = 0;
		int32 len = 0;
		uint32 c32 = 0;

		while (seqLen8 >= 0)
		{
			LL_GetChar(utf8, seqLen8, c32, badChar);
			seqLen16 = LL_GetSeqLen16(c32);

			if (seqLen16 == 1)
			{
				*utf16 = (uint16)c32;
				++utf16;
			}
			else
			{
				uint32 t = c32 - 0x10000;
				*utf16 = (uint16)((t >> 10) + 0xd800);
				++utf16;
				*utf16 = (uint16)((t & 0x3ff) + 0xdc00);
				++utf16;
			}

			utf8 += seqLen8;
			len += seqLen16;

			if (len == MaxStrlen)
			{
				break;
			}

			seqLen8 = LL_GetSeqLen(utf8);
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Unicode functions
	//
	///////////////////////////////////////////////////////////

	uint16* Unicode_Utf8to16(const uint8 utf8[], bool replaceBadSequences)
	{
		// skip bom
		if ((utf8[0] == 0xef) && (utf8[1] == 0xbb) && (utf8[2] == 0xbf))
		{
			utf8 += 3;
		}

		uint32 badChar;

		if (replaceBadSequences)
		{
			badChar = 0xfffd;
		}
		else
		{
			badChar = UINT32_MAX;
		}

		int32 len16 = LL_CalcLen16(utf8, badChar);

		if (len16 > 0)
		{
			uint16* utf16 = (uint16*)Memory_Allocate(((size_t)len16 + 1) * sizeof(uint16));
			LL_Convert(utf8, utf16, badChar);
			utf16[len16] = 0;
			return utf16;
		}

		return nullptr;
	}

	void Unicode_Free(void* utf)
	{
		Memory_Free(utf);
	}
}
