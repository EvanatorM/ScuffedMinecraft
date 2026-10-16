#pragma once

#include <wv/pch.h>

namespace ScuffedMinecraft
{
	struct Block
	{
		std::string name;

		Block(const std::string& name)
			: name(name) {}
	};
}