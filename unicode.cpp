#include "unicode.h"

namespace aux
{
	static const i8_t utf8_seq_lens[] =
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

	static const i32_t max_str_len = INT16_MAX;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static u32_t seq2_to_char(const u8_t seq[])
	{
		return (((u32_t)seq[0] & 0x1f) << 6) | (seq[1] & 0x3f);
	}

	static u32_t seq3_to_char(const u8_t seq[])
	{
		return (((u32_t)seq[0] & 0xf) << 12) | (((u32_t)seq[1] & 0x3f) << 6) | (seq[2] & 0x3f);
	}

	static u32_t seq4_to_char(const u8_t seq[])
	{
		return (((u32_t)seq[0] & 0x7) << 18) | (((u32_t)seq[1] & 0x3f) << 12) | (((u32_t)seq[2] & 0x3f) << 6) | (seq[3] & 0x3f);
	}

	static bool is_octet_ok(u8_t octet)
	{
		return (octet & 0xc0) == 0x80;
	}

	static void get_char(const u8_t seq[], i32_t& seq_len, u32_t& out_char, u32_t bad_char)
	{
		switch (seq_len)
		{
			case 1:
				out_char = seq[0];
				return;
			case 2:
				if (is_octet_ok(seq[1]))
				{
					out_char = seq2_to_char(seq);
					return;
				}
				break;
			case 3:
				if (is_octet_ok(seq[1]) && is_octet_ok(seq[2]))
				{
					out_char = seq3_to_char(seq);
					return;
				}
				break;
			case 4:
				if (is_octet_ok(seq[1]) && is_octet_ok(seq[2]) && is_octet_ok(seq[3]))
				{
					out_char = seq4_to_char(seq);
					return;
				}
				break;
		}

		seq_len = 1;
		out_char = bad_char;
	}

	static i32_t get_seq_len(const u8_t seq[])
	{
		return utf8_seq_lens[*seq];
	}

	static i32_t get_seq_len16(u32_t c32)
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

	static i32_t calc_len16(const u8_t utf8[], u32_t bad_char)
	{
		i32_t seq_len8 = get_seq_len(utf8);
		i32_t seq_len16 = 0;
		i32_t len = 0;
		u32_t c32 = 0;

		while (seq_len8 >= 0)
		{
			get_char(utf8, seq_len8, c32, bad_char);
			seq_len16 = get_seq_len16(c32);

			if (seq_len16 < 1)
			{
				break;
			}

			utf8 += seq_len8;
			len += seq_len16;

			if (len == max_str_len)
			{
				return len;
			}

			seq_len8 = get_seq_len(utf8);
		}

		if (seq_len8 < 0)
		{
			return len;
		}

		return 0;
	}

	static void convert(const u8_t utf8[], u16_t utf16[], u32_t bad_char)
	{
		i32_t seq_len8 = get_seq_len(utf8);
		i32_t seq_len16 = 0;
		i32_t len = 0;
		u32_t c32 = 0;

		while (seq_len8 >= 0)
		{
			get_char(utf8, seq_len8, c32, bad_char);
			seq_len16 = get_seq_len16(c32);

			if (seq_len16 == 1)
			{
				*utf16 = (u16_t)c32;
				++utf16;
			}
			else
			{
				u32_t t = c32 - 0x10000;
				*utf16 = (u16_t)((t >> 10) + 0xd800);
				++utf16;
				*utf16 = (u16_t)((t & 0x3ff) + 0xdc00);
				++utf16;
			}

			utf8 += seq_len8;
			len += seq_len16;

			if (len == max_str_len)
			{
				break;
			}

			seq_len8 = get_seq_len(utf8);
		}
	}

	///////////////////////////////////////////////////////////
	//
	//	Unicode functions
	//
	///////////////////////////////////////////////////////////

	u16_t* to_utf16(const u8_t utf8[], bool replace_bad_sequences)
	{
		// skip bom
		if ((utf8[0] == 0xef) && (utf8[1] == 0xbb) && (utf8[2] == 0xbf))
		{
			utf8 += 3;
		}

		u32_t bad_char;

		if (replace_bad_sequences)
		{
			bad_char = 0xfffd;
		}
		else
		{
			bad_char = UINT32_MAX;
		}

		i32_t len16 = calc_len16(utf8, bad_char);

		if (len16 > 0)
		{
			u16_t* utf16 = (u16_t*)alloc_mem(((size_t)len16 + 1) * sizeof(u16_t));
			convert(utf8, utf16, bad_char);
			utf16[len16] = 0;
			return utf16;
		}

		return nullptr;
	}

	u8_t* to_utf8(const u16_t utf16[])
	{
		(void)utf16;
		throw "Is not implemented";
	}

	void free_utf(void* utf)
	{
		free_mem(utf);
	}
}
