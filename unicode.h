#pragma once

#include "base.h"

namespace aux
{
	uint16* Unicode_Utf8to16(const uint8 utf8[], bool replaceBadSequences);
	void Unicode_Free(void* utf);
}
