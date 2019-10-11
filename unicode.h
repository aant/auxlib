#pragma once

#include "base.h"

namespace aux
{
	u16_t* to_utf16(const u8_t utf8[], bool replace_bad_sequences);
	u8_t* to_utf8(const u16_t utf16[]);
	void free_utf(void* utf);
}
